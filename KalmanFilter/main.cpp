#include "mainwindow.h"

#include <QApplication>
#include <QLocale>

#include <iostream>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("dmikarpoff");
    QApplication::setApplicationName("KalmanFilter");
    QLocale::setDefault(QLocale::c());
    MainWindow window;
    window.show();
    app.exec();
    return 0;
}
