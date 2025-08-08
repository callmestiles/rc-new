#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <PathfindingEngine.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<PathfindingEngine>("PathfindingEngine", 1, 0, "PathfindingEngine");

    QQmlApplicationEngine engine;

    PathfindingEngine pathfindingEngine;

    engine.rootContext()->setContextProperty("pathfindingEngine", &pathfindingEngine);

    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
