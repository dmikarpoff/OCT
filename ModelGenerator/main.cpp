#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("dmikarpoff");
    QApplication::setApplicationName("ModelGenerator");
    MainWindow w;
    w.show();
    return a.exec();
}
