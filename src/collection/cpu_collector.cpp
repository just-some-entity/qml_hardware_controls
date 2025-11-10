#include "cpu_collector.h"

#include <qdir.h>
#include <qfile.h>
#include <qtypes.h>
#include <qregularexpression.h>

#include "cpu_data.h"

// Utilities for /proc/cpu_info
struct CpuInfoEntry
{
    int processor;    // cpuX from /proc/stat
    int physicalId;   // physical CPU/package
    int coreId;       // core number in the CPU

    QMap<QString, QVariant> rawPairs;
};

QVector<CpuInfoEntry> readCpuInfo(const FilterMode filterMode, const std::unordered_set<QString>& filter)
{
    QVector<CpuInfoEntry> entries;

    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open /proc/cpuinfo. CpuMonitor data will be incomplete";
        return entries;
    }

    CpuInfoEntry* currentEntry = nullptr;

    const QRegularExpression re("^\\s*([^:]+)\\s*:\\s*(.+)$"); // key : value

    QString contents = file.readAll();
    QStringList lines = contents.split('\n', Qt::SkipEmptyParts);

    for (const auto& lineRaw : lines)
    {
        QString line = lineRaw;
        if (line.isEmpty())
            continue;

        QRegularExpressionMatch match = re.match(line);
        if (!match.hasMatch())
            continue;

        QString key   = match.captured(1).trimmed();
        QString value = match.captured(2).trimmed();

        if (key == "processor")
        {
            const auto val = value.toInt();

            if (currentEntry == nullptr || currentEntry->processor != val)
                currentEntry = &entries.emplace_back();

            currentEntry->processor = val;
            currentEntry->rawPairs.insert(key, value);
        }
        else if (currentEntry == nullptr)
            throw "Error while reading /proc/cpuinfo, processor should be the first key, but it's not?";
        else if (key == "physical id")
        {
            currentEntry->physicalId = value.toInt();
            currentEntry->rawPairs.insert(key, value);
        }
        else if (key == "core id")
        {
            currentEntry->coreId = value.toInt();
            currentEntry->rawPairs.insert(key, value);
        }
        else if (key == "model name")
            currentEntry->rawPairs.insert(key, value);
        else if (filterMode == FilterMode::Inclusive && filter.contains(key))
            currentEntry->rawPairs.insert(key, value);
        else if (filterMode == FilterMode::Exclusive && !filter.contains(key))
            currentEntry->rawPairs.insert(key, value);
    }

    return entries;
}

//# Utils for /proc/stat
struct StatData
{
    Data_Cpu::Stats totalCpuStats;
    QVector<Data_Cpu::Stats> processorStats;

    QVector<quint64> interrupts;

    quint64 contextSwitches = 0;
    quint64 bootTime        = 0;
    quint64 processes       = 0;
    quint64 procsRunning    = 0;
    quint64 procsBlocked    = 0;

    QVector<quint64> softIrqs;
};

void parseStatCpu(Data_Cpu::Stats& stats, const QStringList& parts)
{
    stats.user       = parts[ 1].toDouble();
    stats.nice       = parts[ 2].toDouble();
    stats.system     = parts[ 3].toDouble();
    stats.idle       = parts[ 4].toDouble();
    stats.iowait     = parts[ 5].toDouble();
    stats.irq        = parts[ 6].toDouble();
    stats.softirq    = parts[ 7].toDouble();
    stats.steal      = parts[ 8].toDouble();
    stats.guest      = parts[ 9].toDouble();
    stats.guest_nice = parts[10].toDouble();
}

StatData readStat()
{
    StatData data{};

    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open /proc/stat. CpuMonitor data will be incomplete";
        return data;
    }

    QString contents  = file.readAll();
    QStringList lines = contents.split('\n', Qt::SkipEmptyParts);

    for (const auto& line : lines)
    {
        if (line.isEmpty())
            continue;

        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (parts[0] == "cpu") // global cpu stats
            parseStatCpu(data.totalCpuStats, parts);
        else if (line.startsWith("cpu"))
            parseStatCpu(data.processorStats.emplace_back(), parts);
        else if (parts[0] == "intr") // interrupts
        {
            data.interrupts.clear();
            for (int i = 1; i < parts.size(); ++i)
                data.interrupts.push_back(parts[i].toULongLong());
        }
        else if (parts[0] == "ctxt") // context switches
            data.contextSwitches = parts[1].toULongLong();
        else if (parts[0] == "btime") // boot time
            data.bootTime = parts[1].toULongLong();
        else if (parts[0] == "processes") // total forks
            data.processes = parts[1].toULongLong();
        else if (parts[0] == "procs_running")
            data.procsRunning = parts[1].toULongLong();
        else if (parts[0] == "procs_blocked")
            data.procsBlocked = parts[1].toULongLong();
        else if (parts[0] == "softirq")
        {
            data.softIrqs.clear();
            for (int i = 1; i < parts.size(); ++i)
                data.softIrqs.push_back(parts[i].toULongLong());
        }
    }

    return data;
}


//# Utils for /sys/devices/system/cpu/cpufreq
struct FreqEntry
{
    qreal min;
    qreal max;
    qreal curr;
};

std::unordered_map<quint64, FreqEntry> readFreqMinMax()
{
    std::unordered_map<quint64, FreqEntry> data;

    const QDir cpuDir("/sys/devices/system/cpu/");
    QStringList cpuDirs = cpuDir.entryList(QStringList() << "cpu[0-9]*", QDir::Dirs);

    for (const QString& cpu : cpuDirs)
    {
        bool ok = false;
        quint64 index = cpu.mid(3).toULongLong(&ok);
        if (!ok)
            continue;

        QString basePath = cpuDir.absoluteFilePath(cpu + "/cpufreq/");

        QFile fMin(basePath + "cpuinfo_min_freq");
        QFile fMax(basePath + "cpuinfo_max_freq");
        QFile fNow(basePath + "scaling_cur_freq");

        qreal minFreq = 0;
        qreal maxFreq = 0;
        qreal nowFreq = 0;

        if (fMin.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            minFreq = fMin.readAll().trimmed().toDouble();
            fMin.close();
        }

        if (fMax.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            maxFreq = fMax.readAll().trimmed().toDouble();
            fMax.close();
        }

        if (fNow.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            nowFreq = fNow.readAll().trimmed().toDouble();
            fNow.close();
        }

        data[index] = FreqEntry{minFreq, maxFreq, nowFreq}; // keyed by cpu index
    }

    return data;
}

//# Utils for /proc/loadavg
struct LoadAvgData
{
    qreal load1  = 0;
    qreal load5  = 0;
    qreal load15 = 0;
};

LoadAvgData readLoadAvg()
{
    LoadAvgData data{};

    QFile f("/proc/loadavg");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&f);
        in >> data.load1 >> data.load5 >> data.load15;
    }

    return data;
}

Data_Cpu CpuCollector::collect(const Options& options)
{
    Data_Cpu data;

    return data;
}