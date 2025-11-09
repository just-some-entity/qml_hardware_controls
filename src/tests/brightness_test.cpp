#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <qqml.h>

#include "../brightness.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType<Brightness>("BrightnessControllerP", 1, 0, "BrightnessController",
    [](QQmlEngine*, QJSEngine*) -> QObject*
    {
        return new Brightness();
    });

    engine.load(QUrl("qrc:/tests/brightness/test.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return QGuiApplication::exec();
}