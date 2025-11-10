#include "sampler.h"

QHash<int, QByteArray> SimpleCpuDataSnapshotModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[static_cast<int>(Roles::Temperature)] = "temperature";
    roles[static_cast<int>(Roles::Frequency)]   = "frequency";
    roles[static_cast<int>(Roles::Utilization)] = "utilization";
    roles[static_cast<int>(Roles::PowerDraw)]   = "powerDraw";
    return roles;
}

int SimpleCpuDataSnapshotModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _snapshots.size();
}

QVariant SimpleCpuDataSnapshotModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _snapshots.size())
        return {};

    const SimpleCpuDataSnapshot& s = _snapshots.at(index.row());

    switch (static_cast<Roles>(role))
    {
    case Roles::Temperature:
        return s.temp;
    case Roles::Frequency:
        return s.freq;
    case Roles::PowerDraw:
        return s.draw;
    case Roles::Utilization:
        return s.util;
    default:
        return {};
    }
}

qsizetype SimpleCpuDataSnapshotModel::size() const
{
    return _snapshots.size();
}

qsizetype SimpleCpuDataSnapshotModel::maxSize() const
{
    return _maxSize;
}

void SimpleCpuDataSnapshotModel::maxSize(const qsizetype size)
{
    if (_maxSize == size) return;
    _maxSize = size;

    // Trim if current size exceeds new max
    if (_snapshots.size() > _maxSize)
    {
        beginRemoveRows(QModelIndex(), 0, _snapshots.size() - _maxSize - 1);
        _snapshots.erase(_snapshots.begin(), _snapshots.begin() + (_snapshots.size() - _maxSize));
        endRemoveRows();
    }
}

const SimpleCpuDataSnapshot& SimpleCpuDataSnapshotModel::snapshotAt(const qsizetype row) const
{
    if (row < 0 || row >= _snapshots.size())
        throw "Index out of bounds.";

    return _snapshots.at(row);
}

const SimpleCpuDataSnapshot& SimpleCpuDataSnapshotModel::appendSnapshot(
    const SimpleCpuDataSnapshot& s)
{
    while (_snapshots.size() >= _maxSize && _maxSize > 0)
    {
        beginRemoveRows(QModelIndex(), 0, 0);
        _snapshots.pop_front();
        endRemoveRows();
    }

    const int row = _snapshots.size();
    beginInsertRows(QModelIndex(), row, row);
    _snapshots.append(s);
    endInsertRows();

    return _snapshots.last();
}

SimpleCpuDataEntryBase::SimpleCpuDataEntryBase(QObject* parent)
: QObject(parent) {}

qreal SimpleCpuDataEntryBase::frequencyMin() const { return  _freqMin; }
qreal SimpleCpuDataEntryBase::frequencyMax() const { return  _freqMax; }
qreal SimpleCpuDataEntryBase::frequency()    const
{
    return _latestSnapshot ? _latestSnapshot->freq : 0.0;
}
qreal SimpleCpuDataEntryBase::temperature() const
{
    return _latestSnapshot ? _latestSnapshot->temp : 0.0;
}
qreal SimpleCpuDataEntryBase::utilization() const
{
    return _latestSnapshot ? _latestSnapshot->util : 0.0;
}

qreal SimpleCpuDataEntryBase::powerDraw() const
{
    return _latestSnapshot ? _latestSnapshot->draw : 0.0;
}

void SimpleCpuDataEntryBase::importData(
    const Data_Cpu::Entry& entry,
    const qreal draw)
{
    _freqMin = entry.freqMin;
    _freqMax = entry.freqMax;

    const quint64 newTotal = entry.stats.total();
    const quint64 newIdle  = entry.stats.idle;

    SimpleCpuDataSnapshot snap;
    snap.freq = entry.freqNow;
    snap.temp = entry.temp;
    snap.util = 0.0;
    snap.draw = draw;

    if (_total != 0)
    {
        const auto totalDiff = newTotal - _total;
        const auto idleDiff  = newIdle  - _idle;
        snap.util = static_cast<qreal>(totalDiff - idleDiff) / static_cast<float>(totalDiff);
    }

    _total = newTotal;
    _idle  = newIdle;

    //SimpleCpuDataSnapshot is protected so cast
    _latestSnapshot = &_snapshots.appendSnapshot(snap);
}

QString SimpleCpuDataSampler::name() const
{
    return _name;
}

qreal SimpleCpuDataSampler::load1() const
{
    return _load1;
}

qreal SimpleCpuDataSampler::load5() const
{
    return _load5;
}

qreal SimpleCpuDataSampler::load15() const
{
    return _load15;
}

void SimpleCpuDataSampler::sample(Data_Cpu& data)
{
    _load1  = data.load1;
    _load5  = data.load5;
    _load15 = data.load15;

    if (data.cpus.empty()) return;

    auto& cpu = data.cpus.at(0);

    importData(cpu, cpu.draw);
    _name = cpu.name;

    qsizetype i = 0;
    for (const auto& coreData : cpu.cores)
    {
        if (_cores.size() <= i)
        {
            // Create new entry
            auto thiz = static_cast<SimpleCpuDataEntryBase*>(this);
            const auto entry = _cores.emplace_back(new SimpleCpuDataEntryBase(thiz));
            entry->importData(coreData);

            emit staticChanged();
        }

        else
            _cores.at(i)->importData(coreData);

        ++i;
    }

    emit dynamicChanged();
}