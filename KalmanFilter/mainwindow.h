#pragma once

#include <kalman_filter.h>

#include <QScopedPointer>
#include <QLineEdit>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace oct {

class KalmanFilter;

}

class MainWindow : public QMainWindow
{
    Q_OBJECT

 public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

 private:
    void loadSettings();
    void saveSettings() const;
    void onStartStopClicked();
    void startKalmanFiltering();
    void stopKalmanFiltering();
    void onKalmanFilterStopped(oct::KalmanFilter::Status status);
    void onAutoRprTriggered();
    void onProgressReported(QString status, double progress);

    void selectInputFile(QLineEdit* le, QString caption, bool input);

    bool running_ = false;
    QScopedPointer<Ui::MainWindow> ui;
    QScopedPointer<oct::KalmanFilter> kalman_filter_;
};
