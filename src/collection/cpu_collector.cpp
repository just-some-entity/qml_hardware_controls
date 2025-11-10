#include "cpu_collector.h"

#include <qdir.h>
#include <qfile.h>
#include <qtypes.h>
#include <qregularexpression.h>

#include "cpu_data.h"

using Mappings_t = std::unordered_map<qsizetype, QPair<qsizetype, qsizetype>>;

// Returns a list of all cores, (not grouped by cpu, will do that later)
Mappings_t readCpuInfo(const CpuCollector::Options& options, Data_Cpu& data)
{
    Mappings_t mappings{};

    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open /proc/cpuinfo. CpuMonitor data will be incomplete";
        return mappings;
    }

    // I assume that at least one cpu should be found
    Data_Cpu::CpuData* currentCpu = &data.cpus.emplace_back();
    std::unique_ptr<Data_Cpu::CoreData> currentCore = nullptr;

    const QRegularExpression re("^\\s*([^:]+)\\s*:\\s*(.+)$"); // key : value

    QString contents = file.readAll();
    QStringList lines = contents.split('\n', Qt::SkipEmptyParts);

    qsizetype coreCount = 0;

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
            if (currentCore != nullptr)
            {
                if (currentCpu == nullptr) throw std::runtime_error("Failed to assign core to cpu. No cpu found?");
                currentCpu->cores.push_back(*currentCore);

                mappings.insert({coreCount++, { data.cpus.size() - 1, currentCpu->cores.size() - 1 }});
            }

            currentCore.reset(new Data_Cpu::CoreData);
        }
        else if (key == "physical id" && value.toInt() > data.cpus.size() - 1)
            currentCpu = &data.cpus.emplace_back();
        else if (key == "model name")
            currentCpu->name = value;
        if (options.filterMode == FilterMode::Inclusive && options.filter.contains(key))
            currentCore->cpuInfoEntries.insert(key, value);
        else if (options.filterMode == FilterMode::Exclusive && !options.filter.contains(key))
            currentCore->cpuInfoEntries.insert(key, value);
    }

    currentCpu->cores.push_back(*currentCore);
    mappings.insert({coreCount++, { data.cpus.size() - 1, currentCpu->cores.size() - 1 }});

    return mappings;
}

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

void readStat(Data_Cpu& data, const Mappings_t& mappings)
{
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open /proc/stat. CpuMonitor data will be incomplete";
        return;
    }

    QString contents  = file.readAll();
    QStringList lines = contents.split('\n', Qt::SkipEmptyParts);

    for (const auto& line : lines)
    {
        if (line.isEmpty())
            continue;

        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (parts[0] == "cpu") // global cpu stats
            parseStatCpu(data.globalStats.totalCpuStats, parts);
        else if (line.startsWith("cpu"))
        {
            bool ok = false;
            const quint64 index = parts[0].mid(3).toULongLong(&ok);

            if (!ok) throw std::runtime_error("Failed to parse cpu index");

            auto [cpuIndex, coreIndex] = mappings.at(index);
            auto& core = data.cpus[cpuIndex].cores[coreIndex];

            parseStatCpu(core.stats, parts);
        }
        else if (parts[0] == "intr") // interrupts
        {
            data.globalStats.interrupts.clear();
            for (int i = 1; i < parts.size(); ++i)
                data.globalStats.interrupts.push_back(parts[i].toULongLong());
        }
        else if (parts[0] == "ctxt") // context switches
            data.globalStats.contextSwitches = parts[1].toULongLong();
        else if (parts[0] == "btime") // boot time
            data.globalStats.bootTime = parts[1].toULongLong();
        else if (parts[0] == "processes") // total forks
            data.globalStats.processes = parts[1].toULongLong();
        else if (parts[0] == "procs_running")
            data.globalStats.procsRunning = parts[1].toULongLong();
        else if (parts[0] == "procs_blocked")
            data.globalStats.procsBlocked = parts[1].toULongLong();
        else if (parts[0] == "softirq")
        {
            data.globalStats.softIrqs.clear();
            for (int i = 1; i < parts.size(); ++i)
                data.globalStats.softIrqs.push_back(parts[i].toULongLong());
        }
    }
}


//# Utils for /sys/devices/system/cpu/cpufreq
struct FreqEntry
{
    qreal min;
    qreal max;
    qreal now;
};

void readFreqMinMax(Data_Cpu& data, const Mappings_t& mappings)
{
    const QDir cpuDir("/sys/devices/system/cpu/");
    QStringList cpuDirs = cpuDir.entryList(QStringList() << "cpu[0-9]*", QDir::Dirs);

    for (const QString& cpu : cpuDirs)
    {
        bool ok = false;
        quint64 index = cpu.mid(3).toULongLong(&ok);
        if (!ok) continue;

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

        auto [cpuIndex, coreIndex] = mappings.at(index);
        auto& core = data.cpus[cpuIndex].cores[coreIndex];

        core.freqMin = minFreq;
        core.freqMax = maxFreq;
        core.freqNow = nowFreq;
    }
}

//# Utils for /proc/loadavg
void readLoadAvg(Data_Cpu& data)
{
    QFile f("/proc/loadavg");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&f);
        in >> data.load1 >> data.load5 >> data.load15;
    }
}

Data_Cpu CpuCollector::collect(const Options& options)
{
    Data_Cpu data;

    const auto mappings = readCpuInfo(options, data);
    readStat(data, mappings);
    readFreqMinMax(data, mappings);
    readLoadAvg(data);

    return data;
}