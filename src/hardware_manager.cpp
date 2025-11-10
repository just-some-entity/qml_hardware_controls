#include "hardware_manager.h"

#include <QTimer>
#include <qtmetamacros.h>
#include <qdebug.h>
#include <qlogging.h>

namespace hw_monitor {

HardwareManager::HardwareManager(QObject* parent) : QObject(parent)
{
    triggerCollect();

    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, this, &HardwareManager::triggerCollect);
    _timer->start(_sampleRate);
}

int HardwareManager::sampleRate() const
{
    return _sampleRate;
}

void HardwareManager::sampleRate(const int sampleRate)
{
    _sampleRate = qMax(sampleRate, 0);
    emit sampleRateChanged();

    if (_timer)
    {
        _timer->stop();
        if (_sampleRate > 0)
            _timer->start(_sampleRate);
    }
}

void HardwareManager::triggerCollect()
{
    const auto data = CpuCollector::collect(CpuCollector::Options {});
    emit cpuDataChanged(data);
    emit collect();
}
}
