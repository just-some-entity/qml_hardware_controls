#pragma once

#include <qqmlintegration.h>
#include <QTimer>
#include <qtmetamacros.h>

#include "collection/cpu_collector.h"
#include "collection/cpu_data.h"

namespace hw_monitor {

class HardwareManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sampleRate READ sampleRate WRITE sampleRate NOTIFY sampleRateChanged);
    QML_SINGLETON;
    QML_NAMED_ELEMENT(HardwareManager);

public:
    explicit HardwareManager(QObject *parent = nullptr);

    [[nodiscard]] int sampleRate() const;

    void sampleRate(int sampleRate);

signals:
    void sampleRateChanged();
    void collect();

    void cpuDataChanged(const Data_Cpu& data);

private slots:
    void triggerCollect();

private:
    // In milliseconds
    int _sampleRate = 2000;

    QTimer *_timer = nullptr;
};
}