#pragma once

#include <QDialog>
#include <QThread>
#include <QScopedPointer>

namespace Ui {
class GenerationProgressDialog;
}

class GenerationProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenerationProgressDialog(QThread* assoc_thread, QWidget *parent = nullptr);
    ~GenerationProgressDialog();

    void runEstimation()
    {
        if (assoc_thread_) {
            assoc_thread_->start();
        }
        QDialog::exec();
        if (assoc_thread_) {
            assoc_thread_->wait();
        }
    }

public slots:
    void onEstimationStep(QString stage, int num, double err);

signals:
    void sigStopClicked();

private:
    void closeEvent(QCloseEvent *e) override;

    QScopedPointer<Ui::GenerationProgressDialog> ui;
    QThread* assoc_thread_ = nullptr;
};
