#include "generationprogressdialog.h"
#include "ui_generationprogressdialog.h"

GenerationProgressDialog::GenerationProgressDialog(QThread* assoc_thread, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GenerationProgressDialog)
    , assoc_thread_(assoc_thread)
{
    auto flags = windowFlags();
    auto no_context_hint = (0xffffffff ^ Qt::WindowContextHelpButtonHint);
    flags &= no_context_hint;
    setWindowFlags(flags);
    ui->setupUi(this);

    connect(ui->pbFinish, &QPushButton::clicked, this, &GenerationProgressDialog::sigStopClicked);
}

GenerationProgressDialog::~GenerationProgressDialog() {}

void GenerationProgressDialog::onEstimationStep(QString stage, int step, double err)
{
    ui->lblStage->setText(stage);
    ui->lblCurrentStep->setText(QString::number(step));
    if (std::isnan(err))
    {
        ui->lblStepLength->setVisible(false);
        ui->lblTitleLen->setVisible(false);
    }
    else
    {
        ui->lblStepLength->setVisible(true);
        ui->lblTitleLen->setVisible(true);
        ui->lblStepLength->setText(QString::number(err));
    }
}

void GenerationProgressDialog::closeEvent(QCloseEvent *e)
{
    emit sigStopClicked();
    QDialog::closeEvent(e);
}
