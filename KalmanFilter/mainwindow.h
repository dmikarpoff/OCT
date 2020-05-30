#pragma once

#include <QScopedPointer>
#include <QLineEdit>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void loadSettings();
    void saveSettings();
    void onStartStopClicked();
    void startKalmanFiltering();
    void stopKalmanFiltering();

    void selectInputFile(QLineEdit* le, QString caption, bool input);

    bool running_ = false;
    QScopedPointer<Ui::MainWindow> ui;
};
