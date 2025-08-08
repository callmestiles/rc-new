#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <PathfindingEngine.h>
#include <CarController.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<PathfindingEngine>("PathfindingEngine", 1, 0, "PathfindingEngine");
    qmlRegisterType<CarController>("CarController", 1, 0, "CarController");

    QQmlApplicationEngine engine;

    PathfindingEngine pathfindingEngine;
    CarController carController;

    engine.rootContext()->setContextProperty("pathfindingEngine", &pathfindingEngine);
    engine.rootContext()->setContextProperty("carController", &carController);

    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
