#pragma once

#include <qabstractitemmodel.h>
#include <qqmlintegration.h>
#include <qtypes.h>

#include "../util/sampler.h"
#include "../collection/cpu_data.h"

struct SimpleCpuDataSnapshot
{
    qreal freq = 0.0;
    qreal temp = 0.0;
    qreal util = 0.0;
    qreal draw = 0.0;
};

class SimpleCpuDataSnapshotModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class Roles
    {
        Temperature = Qt::UserRole + 1,
        Frequency,
        Utilization,
        PowerDraw,
    };

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    [[nodiscard]] qsizetype size() const;
    [[nodiscard]] qsizetype maxSize() const;

    void maxSize(qsizetype size);

    const SimpleCpuDataSnapshot& snapshotAt(qsizetype row) const;

    const SimpleCpuDataSnapshot& appendSnapshot(const SimpleCpuDataSnapshot& s);

private:
    qsizetype _maxSize = 50;
    QVector<SimpleCpuDataSnapshot> _snapshots;
};

class SimpleCpuDataEntryBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal frequencyMin READ frequencyMin NOTIFY staticChanged);
    Q_PROPERTY(qreal frequencyMax READ frequencyMax NOTIFY staticChanged);
    Q_PROPERTY(qreal frequency    READ frequency    NOTIFY dynamicChanged);
    Q_PROPERTY(qreal temperature  READ temperature  NOTIFY dynamicChanged);
    Q_PROPERTY(qreal powerDraw    READ powerDraw    NOTIFY dynamicChanged);

    friend class SimpleCpuDataSampler;

public:
    explicit SimpleCpuDataEntryBase(QObject* parent = nullptr);

    [[nodiscard]] qreal frequencyMin() const;
    [[nodiscard]] qreal frequencyMax() const;
    [[nodiscard]] qreal frequency()    const;
    [[nodiscard]] qreal temperature()  const;
    [[nodiscard]] qreal utilization()  const;
    [[nodiscard]] qreal powerDraw()    const;

signals:
    void dynamicChanged();
    void staticChanged();

protected:
    using Model_t = SimpleCpuDataSnapshotModel;

    const SimpleCpuDataSnapshot* _latestSnapshot = nullptr;

    qreal _freqMin = 0.0;
    qreal _freqMax = 0.0;

    quint64 _total = 0;
    quint64 _idle  = 0;
    Model_t _snapshots;

    void importData(const Data_Cpu::Entry& entry, qreal draw = 0.0);
};

using SimpleCpuDataCoreEntry = SimpleCpuDataEntryBase;

class SimpleCpuDataSampler : public SimpleCpuDataEntryBase
{
    Q_OBJECT
    Q_PROPERTY(qreal load1  READ load1  NOTIFY dynamicChanged)
    Q_PROPERTY(qreal load5  READ load5  NOTIFY dynamicChanged)
    Q_PROPERTY(qreal load15 READ load15 NOTIFY dynamicChanged)
    QML_NAMED_ELEMENT(CpuDataSampler)

public:
    [[nodiscard]] QString name()  const;
    [[nodiscard]] qreal load1()  const;
    [[nodiscard]] qreal load5()  const;
    [[nodiscard]] qreal load15() const;

    void sample(Data_Cpu& data);

private:
    QString _name = "N/A";

    qreal _load1  = 0.0;
    qreal _load5  = 0.0;
    qreal _load15 = 0.0;

    QVector<SimpleCpuDataCoreEntry*> _cores;
};