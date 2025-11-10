#include <QCoreApplication>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QTimer>
#include <qobject.h>

#include "daemon.h"

// daemon.cpp
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QLocalServer::removeServer(SERVER_NAME);

    QLocalServer server;
    server.listen(SERVER_NAME);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]
    {
        auto clients = server.findChildren<QLocalSocket*>();
        for (auto client : clients)
        {
            SampledData data
            {
                10
            };

            QByteArray buf(reinterpret_cast<const char*>(&data), sizeof(SampledData));

            client->write(buf);
            client->flush();
        }
    });
    timer.start(5000);

    QObject::connect(&server, &QLocalServer::newConnection, [&]
    {
        auto socket = server.nextPendingConnection();
        socket->write("Hello");
        socket->flush();
    });

    return app.exec();
}
