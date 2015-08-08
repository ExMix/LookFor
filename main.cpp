#include "mainwindow.hpp"
#include <QApplication>

#include <QFile>

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("WG");
  QCoreApplication::setApplicationName("LookFor");
  QCoreApplication::setApplicationVersion("0.1");

  QApplication a(argc, argv);
  QFile file(QStringLiteral(":/assets/stylesheet.qss"));
  if (file.open(QIODevice::ReadOnly))
    a.setStyleSheet(QString(file.readAll()));

  MainWindow w;
  w.show();

  return a.exec();
}
