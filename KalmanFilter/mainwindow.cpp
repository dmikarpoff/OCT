#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kalman_filter.h"
#include "matrixmodel.h"
#include "matrixitemdelegate.h"

#include <QCheckBox>
#include <QFileIconProvider>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>
#include <QThread>

constexpr const char kInputCsvFileTag[] = "InputCsvFile";
constexpr const char kOutputCsvFileTag[] = "OutputCsvFile";
constexpr const char kOutputPngFileTag[] = "OutputPngFile";
constexpr const char kAutoRprTag[] = "AutoR_pr";
constexpr const char kValueTag[] = "v";
constexpr const char kRprTag[] = "Rpr";
constexpr const char kRwTag[] = "Rw";
constexpr const char kRnTag[] = "Rn";

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

void expandView(QTableView* view)
{
    view->resizeColumnsToContents();
    view->resizeRowsToContents();
    auto w = view->verticalHeader()->width() + 2 * view->frameWidth();
    for (int i = 0; i < view->model()->columnCount(); ++i)
    {
        w += view->columnWidth(i);
    }

    auto h = view->horizontalHeader()->height() + 2 * view->frameWidth();
    for (auto i = 0; i < view->model()->rowCount(); ++i)
    {
        h += view->rowHeight(i);
    }

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setMinimumSize(w, h);
}

void saveMatrix(const QAbstractItemModel* model, const char* name, QSettings& settings)
{
    settings.beginWriteArray(name);
    for (int i = 0; i < model->rowCount(); ++i)
    {
        for (int j = 0; j < model->columnCount(); ++j)
        {
            settings.setArrayIndex(i * model->columnCount() + j);
            settings.setValue(kValueTag, model->data(model->index(i, j), Qt::DisplayRole));
        }
    }
    settings.endArray();
}

void loadMatrix(const char* name, QSettings& settings, QAbstractItemModel* model)
{
    auto sz = settings.beginReadArray(name);
    if (sz != model->rowCount() * model->columnCount())
    {
        settings.endArray();
        return;
    }
    for (int i = 0; i < model->rowCount(); ++i)
    {
        for (int j = 0; j < model->columnCount(); ++j)
        {
            settings.setArrayIndex(i * model->columnCount() + j);
            auto val = settings.value(kValueTag).toDouble();
            model->setData(model->index(i, j), val);
        }
    }
    settings.endArray();
}

boost::numeric::ublas::matrix<double> getMatrixFromUi(QAbstractItemModel* model)
{
    boost::numeric::ublas::matrix<double> res(model->rowCount(), model->columnCount());
    for (int i = 0; i < model->rowCount(); ++i)
    {
        for (int j = 0; j < model->columnCount(); ++j)
        {
            res(i, j) = model->data(model->index(i, j)).toDouble();
        }
    }
    return res;
}

}  // namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList teta_header = {"s_0", "s_m", QString::fromUtf8("\xce\xa6"), "u_h", "u_v"};
    QStringList data_header = {"s(i,j)", "s(i,j+1)", "s(i+1,j)", "s(i+1,j+1)"};
    ui->matrixRpr->setModel(new MatrixModel(this, 5, 5, teta_header));
    ui->matrixRpr->setItemDelegate(new MatrixItemDelegate(this));
    expandView(ui->matrixRpr);
    ui->matrixRw->setModel(new MatrixModel(this, 5, 5, teta_header));
    ui->matrixRw->setItemDelegate(new MatrixItemDelegate(this));
    expandView(ui->matrixRw);
    ui->matrixRn->setModel(new MatrixModel(this, 4, 4, data_header));
    ui->matrixRn->setItemDelegate(new MatrixItemDelegate(this));
    expandView(ui->matrixRn);

    ui->pbInputFile->setIcon(QFileIconProvider{}.icon(QFileIconProvider::Folder));
    ui->pbOutputCsv->setIcon(QFileIconProvider{}.icon(QFileIconProvider::Folder));
    ui->pbOutputPng->setIcon(QFileIconProvider{}.icon(QFileIconProvider::Folder));
    ui->progressBar->setEnabled(false);

    loadSettings();

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
    connect(ui->cbAutoRpr, &QCheckBox::clicked, [this] {onAutoRprTriggered();});

    onAutoRprTriggered();
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
    ui->cbAutoRpr->setChecked(settings.value(kAutoRprTag, false).toBool());

    loadMatrix(kRprTag, settings, ui->matrixRpr->model());
    loadMatrix(kRwTag, settings, ui->matrixRw->model());
    loadMatrix(kRnTag, settings, ui->matrixRn->model());
}

void MainWindow::saveSettings() const
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    settings.setValue(kInputCsvFileTag, ui->leInputFile->text());
    settings.setValue(kOutputCsvFileTag, ui->leOutputCsv->text());
    settings.setValue(kOutputPngFileTag, ui->leOutputPng->text());
    settings.setValue(kAutoRprTag, ui->cbAutoRpr->isChecked());

    saveMatrix(ui->matrixRpr->model(), kRprTag, settings);
    saveMatrix(ui->matrixRw->model(), kRwTag, settings);
    saveMatrix(ui->matrixRn->model(), kRnTag, settings);
}

void MainWindow::onStartStopClicked()
{
    running_ ? stopKalmanFiltering() : startKalmanFiltering();
}

void MainWindow::startKalmanFiltering()
{
    QString input_csv = ui->leInputFile->text();
    QString output_csv = ui->leOutputCsv->text();
    QString output_png = ui->leOutputPng->text();

    if (!checkInputFile(input_csv))
    {
        QMessageBox::critical(this, "File error",
            QString::fromLatin1("File '%1' is not readable").arg(input_csv));
        return;
    }
    auto output_files = {output_csv, output_png};
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

    auto* working_thread = new QThread();
    kalman_filter_.reset(new oct::KalmanFilter());
    kalman_filter_->moveToThread(working_thread);
    auto r_pr = getMatrixFromUi(ui->matrixRpr->model());
    auto r_n = getMatrixFromUi(ui->matrixRn->model());
    auto r_w = getMatrixFromUi(ui->matrixRw->model());
    bool auto_r_pr = ui->cbAutoRpr->isChecked();

    connect(working_thread, &QThread::started, kalman_filter_.get(),
            [this, input_csv, output_csv, output_png, r_pr = std::move(r_pr),
                r_n = std::move(r_n), r_w = std::move(r_w), auto_r_pr] {
                if (auto_r_pr) {
                    kalman_filter_->filterAuto(input_csv, output_csv, output_png,
                                           std::move(r_pr), std::move(r_n));
                } else {
                    kalman_filter_->filter(input_csv, output_csv, output_png,
                                               std::move(r_w), std::move(r_n));
                }
            },
        Qt::QueuedConnection
    );
    connect(working_thread, &QThread::finished, working_thread, &QThread::deleteLater,
            Qt::QueuedConnection);
    connect(kalman_filter_.get(), &oct::KalmanFilter::finished, working_thread, &QThread::quit,
            Qt::QueuedConnection);
    connect(kalman_filter_.get(), &oct::KalmanFilter::finished, this, &MainWindow::onKalmanFilterStopped,
            Qt::QueuedConnection);
    connect(kalman_filter_.get(), &oct::KalmanFilter::reportProgress, this, &MainWindow::onProgressReported,
            Qt::QueuedConnection);

    working_thread->start();

    running_ = true;
}

void MainWindow::stopKalmanFiltering()
{
    if (kalman_filter_) {
        kalman_filter_->interrupt();
    }
}

void MainWindow::onKalmanFilterStopped(oct::KalmanFilter::Status status)
{
    kalman_filter_.reset();
    QMessageBox msg_box(status.icon, status.caption, status.message, QMessageBox::Ok, this);
    msg_box.exec();

    ui->pbStart->setText("Start");
    ui->leInputFile->setEnabled(true);
    ui->pbInputFile->setEnabled(true);
    ui->leOutputCsv->setEnabled(true);
    ui->pbOutputCsv->setEnabled(true);
    ui->leOutputPng->setEnabled(true);
    ui->pbOutputPng->setEnabled(true);
    ui->progressBar->setEnabled(false);
    running_ = false;

    onProgressReported("", 0.0);
}

void MainWindow::onAutoRprTriggered()
{
    bool checked = ui->cbAutoRpr->isChecked();

    ui->matrixRpr->setEnabled(!checked);
    ui->matrixRw->setEnabled(checked);
}

void MainWindow::onProgressReported(QString status, double progress)
{
    ui->progressBar->setValue(qRound(progress * 100.0));
    ui->lblProgressStatus->setText(status);
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
