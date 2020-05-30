#include "mainwindow.h"

#include <QApplication>

#include <iostream>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("dmikarpoff");
    QApplication::setApplicationName("KalmanFilter");
    MainWindow window;
    window.show();
    app.exec();
    return 0;
}
