#include "stdafx.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include "backend.h"

int main(int argc, char *argv[])
{
	QQuickStyle::setStyle(QLatin1String("Material"));
	QGuiApplication app(argc, argv);
	
	qmlRegisterType<BackEnd>("io.qt.examples.backend", 1, 0, "BackEnd");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}
