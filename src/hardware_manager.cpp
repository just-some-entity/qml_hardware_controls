#include "hardware_manager.h"

#include <QLocalSocket>
#include <QTimer>
#include <qtmetamacros.h>
#include <qlogging.h>
#include <qdebug.h>
#include <QProcess>

#include "daemon/daemon.h"

namespace hw_monitor {

HardwareManager::HardwareManager(QObject* parent) : QObject(parent)
{
    triggerCollect();

    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, this, &HardwareManager::triggerCollect);
    _timer->start(_sampleRate);

    // Launch daemon
    QProcess::startDetached("/home/entity/projects/qml-brightnessctrl/build-debug/src/daemon/daemon");

    QLocalSocket socket;
    socket.connectToServer(SERVER_NAME);
    socket.waitForConnected();
    socket.waitForReadyRead();

    connect(&socket, &QLocalSocket::readyRead, [&]
    {
        QByteArray data = socket.readAll();
        qDebug() << "Received: " << data.toHex();
    });

    //qDebug() << socket.readAll();
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
    cpuDataChanged(data);

    emit collect();
}
}
