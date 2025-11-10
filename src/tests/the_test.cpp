#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <qqml.h>

#include "../brightness.h"
#include "../hardware_manager.h"

#include "../samplers/cpu_sampler_simple.h"

int main(int argc, char *argv[])
{
    using namespace hw_monitor;

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<SimpleCpuDataSampler>("HardwareManager", 1, 0, "CpuDataSampler");

    qmlRegisterSingletonType<Brightness>("HardwareManager", 1, 0, "BrightnessController",
    [](QQmlEngine*, QJSEngine*) -> QObject*
    {
        return new Brightness;
    });

    qmlRegisterSingletonType<HardwareManager>("HardwareManager", 1, 0, "HardwareManager",
    [](QQmlEngine*, QJSEngine*) -> QObject*
    {
        return new HardwareManager;
    });

    engine.load(QUrl("qrc:/tests/primary/test.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return QGuiApplication::exec();
}