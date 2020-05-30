#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileIconProvider>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

namespace
{

bool checkInputFile(QString file_name)
{
    QFileInfo info(file_name);
    return info.exists() && info.isReadable();
}

bool checkOutputFile(QString file_name)
{
    QFileInfo info(file_name);
    if (info.exists())
    {
        return info.isWritable();
    }
    QFileInfo dir_info(info.absoluteDir().absolutePath());
    return dir_info.isDir() && dir_info.exists() && dir_info.isWritable();
}

}  // namespace

constexpr const char kInputCsvFileTag[] = "InputCsvFile";
constexpr const char kOutputCsvFileTag[] = "OutputCsvFile";
constexpr const char kOutputPngFileTag[] = "OutputPngFile";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pbInputFile->setIcon(QFileIconProvider{}.icon(QFileIconProvider::Folder));
    ui->pbOutputCsv->setIcon(QFileIconProvider{}.icon(QFileIconProvider::Folder));
    ui->pbOutputPng->setIcon(QFileIconProvider{}.icon(QFileIconProvider::Folder));
    ui->progressBar->setEnabled(false);

    connect(ui->pbInputFile, &QPushButton::clicked,
            [this]() {
                selectInputFile(ui->leInputFile, "Input CSV-file", true);
            }
    );
    connect(ui->pbOutputCsv, &QPushButton::clicked,
            [this]() {
                selectInputFile(ui->leOutputCsv, "Output CSV-file", false);
            }
    );
    connect(ui->pbOutputPng, &QPushButton::clicked,
            [this]() {
                selectInputFile(ui->leOutputPng, "Output PNG-file", false);
            }
    );
    connect(ui->pbStart, &QPushButton::clicked, [this] {onStartStopClicked();});

    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    ui->leInputFile->setText(settings.value(kInputCsvFileTag, "").toString());
    ui->leOutputCsv->setText(settings.value(kOutputCsvFileTag, "").toString());
    ui->leOutputPng->setText(settings.value(kOutputPngFileTag, "").toString());
}

void MainWindow::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    settings.setValue(kInputCsvFileTag, ui->leInputFile->text());
    settings.setValue(kOutputCsvFileTag, ui->leOutputCsv->text());
    settings.setValue(kOutputPngFileTag, ui->leOutputPng->text());
}

void MainWindow::onStartStopClicked()
{
    running_ ? stopKalmanFiltering() : startKalmanFiltering();
}

void MainWindow::startKalmanFiltering()
{
    if (!checkInputFile(ui->leInputFile->text()))
    {
        QMessageBox::critical(this, "File error",
            QString::fromLatin1("File '%1' is not readable").arg(ui->leInputFile->text()));
        return;
    }
    auto output_files = {ui->leOutputCsv->text(), ui->leOutputPng->text()};
    for (auto file_name : output_files)
    {
        if (!checkOutputFile(file_name))
        {
            QMessageBox::critical(this, "File error",
                QString::fromLatin1("File %1 is not writable").arg(file_name));
            return;
        }
    }

    ui->pbStart->setText("Stop");
    ui->leInputFile->setEnabled(false);
    ui->pbInputFile->setEnabled(false);
    ui->leOutputCsv->setEnabled(false);
    ui->pbOutputCsv->setEnabled(false);
    ui->leOutputPng->setEnabled(false);
    ui->pbOutputPng->setEnabled(false);
    ui->progressBar->setEnabled(true);

    running_ = true;
}

void MainWindow::stopKalmanFiltering()
{

}

void MainWindow::selectInputFile(QLineEdit *le, QString caption, bool input)
{
    QString file_name;
    QString old_path = le->text();
    QString prefix;
    if (!old_path.isEmpty())
    {
        QFileInfo file_info(old_path);
        prefix = file_info.absoluteDir().absolutePath();
    }
    if (input)
    {
        file_name = QFileDialog::getOpenFileName(this, caption, prefix, "Table file (*.csv)");
    }
    else
    {
        file_name = QFileDialog::getSaveFileName(this, caption, prefix,
                        caption.contains("PNG") ? "Image (*.png)" : "Table (*.csv)" );
    }
    if (!file_name.isEmpty())
    {
        le->setText(file_name);
    }
}
