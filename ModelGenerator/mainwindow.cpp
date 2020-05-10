#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include "layer_property_widget.h"
#include "tissuesolver.h"
#include "iterativetissuesolver.h"
#include "generationprogressdialog.h"
#include "model_export.h"
#include "image_utils.h"

#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>
#include <QThread>

#include <algorithm>

namespace {

template <class F>
void for_each_layer_widget(QToolBox* toolbox, F&& f) {
    for (int i = 0; i < toolbox->count(); ++i) {
        auto* layer_descr = qobject_cast<LayerPropertyWidget*>(toolbox->widget(i));
        if (!layer_descr)
        {
            continue;
        }
        f(layer_descr, i);
    }
}

}

const constexpr char kLayerNumberTag[] = "layer_num";
const constexpr char kLayerATag[] = "a";
const constexpr char kLayerBTag[] = "b";
const constexpr char kLayerCTag[] = "c";
const constexpr char kLayerDTag[] = "d";
const constexpr char kLayerAlphaTag[] = "alpha";
const constexpr char kBorderSigmaTag[] = "sigma";
const constexpr char kBorderAlphaTag[] = "border_alpha";
const constexpr char kBorderMaxZTag[] = "max_z";
const constexpr char kImgWidthTag[] = "img_width";
const constexpr char kImgHeightTag[] = "img_height";
const constexpr char kLowerColorTag[] = "lower_color";
const constexpr char kUpperColorTag[] = "upper_color";
const constexpr char kBorderMovingAverWindowTag[] = "border_moving_aver";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->toolBoxModelLayer->removeItem(0);

    loadSettings();
    updateButtonEnabled();
    createMenus();
    updateListOfLayerWidget();

    connect(ui->pbInsert, &QPushButton::clicked, [this]() {onAddModelLayer();});
    connect(ui->pbRemove, &QPushButton::clicked, [this]() {onRemoveModelLayer();});
    connect(ui->pbGenerate, &QPushButton::clicked, [this]() {onGenerateClicked();});
    connect(ui->colorLower, &ColorSelectionWidget::sigColorChanged,
            [this](){updateImage();});
    connect(ui->colorUpper, &ColorSelectionWidget::sigColorChanged,
            [this](){updateImage();});
    connect(ui->sbHeight, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int val) {
                layerBoundsChanged(val, ui->toolBoxModelLayer->count() - 1);
            });
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::layerBoundsChanged(int max_z, int layer_idx)
{
    if (layer_idx > 0)
    {
        auto* prev = qobject_cast<LayerPropertyWidget*>(
                        ui->toolBoxModelLayer->widget(layer_idx - 1));
        prev->SetMaxZMaximum(max_z - 1);
    }
    if (layer_idx + 1 < ui->toolBoxModelLayer->count())
    {
        auto next = qobject_cast<LayerPropertyWidget*>(
                        ui->toolBoxModelLayer->widget(layer_idx + 1));
        next->SetMaxZMinimum(max_z + 1);
    }
}

void MainWindow::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());
    img_width_ = settings.value(kImgWidthTag, 640).toInt();
    img_height_ = settings.value(kImgHeightTag, 480).toInt();
    int count = settings.value(kLayerNumberTag, 0).toInt();
    for (int i = 0; i < count; ++i)
    {
        LayerProperty lp;
        auto idx_str = std::to_string(i);
        lp.a = settings.value((kLayerATag + idx_str).c_str(), 0.0).toDouble();
        lp.b = settings.value((kLayerBTag + idx_str).c_str(), 0.0).toDouble();
        lp.c = settings.value((kLayerCTag + idx_str).c_str(), 0.0).toDouble();
        lp.d = settings.value((kLayerDTag + idx_str).c_str(), 0.0).toDouble();
        lp.alpha = settings.value((kLayerAlphaTag + idx_str).c_str(), 0.5).toDouble();
        lp.sigma = settings.value((kBorderSigmaTag + idx_str).c_str(), 1.0).toDouble();
        lp.border_alpha = settings.value((kBorderAlphaTag + idx_str).c_str(), 0.5).toDouble();
        lp.max_z = settings.value((kBorderMaxZTag + idx_str).c_str(), 1).toInt();
        lp.border_aver_window = settings.value((kBorderMovingAverWindowTag + idx_str).c_str(), 1).toInt();
        if (i == count - 1)
        {
            lp.max_z = static_cast<int>(img_height_);
        }
        ui->toolBoxModelLayer->addItem(new LayerPropertyWidget(lp),
                                       QString("Layer %1").arg(i));
    }

    reinitLayerBounds();
    for_each_layer_widget(ui->toolBoxModelLayer,
        [this](auto* lp_widget, int /*idx*/)
        {
            connect(lp_widget, &LayerPropertyWidget::sigMaxZChanged,
                    this, &MainWindow::layerBoundsChanged);
        }
    );

    ui->sbWidth->setValue(static_cast<int>(img_width_));
    ui->sbHeight->setValue(static_cast<int>(img_height_));
    ui->colorLower->setColor(
        QColor::fromRgb(settings.value(kLowerColorTag, qRgb(255, 255, 255)).toUInt()));
    ui->colorUpper->setColor(
        QColor::fromRgb(settings.value(kUpperColorTag, qRgb(0, 0, 0)).toUInt()));
}

void MainWindow::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());

    auto count = ui->toolBoxModelLayer->count();
    settings.setValue(kLayerNumberTag, count);
    for (int i = 0; i < count; ++i)
    {
        auto* layer_prop = qobject_cast<LayerPropertyWidget*>(ui->toolBoxModelLayer->widget(i));
        if (!layer_prop)
        {
            continue;
        }
        const auto& lp = layer_prop->GetLayerProperty();
        auto idx_str = std::to_string(i);
        settings.setValue((kLayerATag + idx_str).c_str(), lp.a);
        settings.setValue((kLayerBTag + idx_str).c_str(), lp.b);
        settings.setValue((kLayerCTag + idx_str).c_str(), lp.c);
        settings.setValue((kLayerDTag + idx_str).c_str(), lp.d);
        settings.setValue((kLayerAlphaTag + idx_str).c_str(), lp.alpha);
        settings.setValue((kBorderSigmaTag + idx_str).c_str(), lp.sigma);
        settings.setValue((kBorderAlphaTag + idx_str).c_str(), lp.border_alpha);
        settings.setValue((kBorderMaxZTag + idx_str).c_str(), lp.max_z);
        settings.setValue((kBorderMovingAverWindowTag + idx_str).c_str(), lp.border_aver_window);
    }

    settings.setValue(kImgWidthTag, img_width_);
    settings.setValue(kImgHeightTag, img_height_);
    settings.setValue(kLowerColorTag, ui->colorLower->color().rgba());
    settings.setValue(kUpperColorTag, ui->colorUpper->color().rgba());
}

void MainWindow::createMenus()
{
    auto main_menu = menuBar()->addMenu("Main");
    auto* export_action = new QAction("Export");
    main_menu->addAction(export_action);
    connect(export_action, &QAction::triggered, [this] () {onExportFile();});
}

void MainWindow::updateButtonEnabled()
{
    bool empty = (ui->toolBoxModelLayer->count() == 0);
    ui->pbRemove->setEnabled(!empty);
    ui->pbGenerate->setEnabled(!empty);
}

void MainWindow::updateImage()
{
    if (raw_data_.empty())
    {
        return;
    }

    auto img_data = rawDataToRgb(raw_data_, ui->colorLower->color(), ui->colorUpper->color());
    QImage img(img_data.data(), static_cast<int>(img_width_),
               static_cast<int>(img_height_), QImage::Format_ARGB32);
    ui->labelModelImage->setPixmap(QPixmap::fromImage(std::move(img)));
}

void MainWindow::updateListOfLayerWidget()
{
    for_each_layer_widget(ui->toolBoxModelLayer,
        [toolbox = ui->toolBoxModelLayer](auto* layer_descr, int idx)
        {
            layer_descr->SetMultilayerGroupVisible(idx + 1 != toolbox->count());
            layer_descr->SetIndex(idx);
            toolbox->setItemText(idx, QString("Layer %1").arg(idx));
        }
    );
}

void MainWindow::reinitLayerBounds()
{
    if (ui->toolBoxModelLayer->count() > 0)
    {
        auto first = qobject_cast<LayerPropertyWidget*>(ui->toolBoxModelLayer->widget(0));
        first->SetMaxZMinimum(1);
        auto last = qobject_cast<LayerPropertyWidget*>(
                        ui->toolBoxModelLayer->widget(ui->toolBoxModelLayer->count() - 1));
        last->SetMaxZMaximum(static_cast<int>(img_height_));
    }
    for_each_layer_widget(ui->toolBoxModelLayer,
        [this](auto* layer_descr, int idx)
        {
            const auto& lp = layer_descr->GetLayerProperty();
            layerBoundsChanged(lp.max_z, idx);
        }
    );
}

void MainWindow::onAddModelLayer()
{
    auto* toolbox = ui->toolBoxModelLayer;
    auto* new_item = new LayerPropertyWidget(toolbox);
    int pos = toolbox->insertItem(toolbox->currentIndex() + 1, new_item,
                                  "");
    updateButtonEnabled();
    updateListOfLayerWidget();
    int lower_z = 1;
    if (pos > 0)
    {
        auto* lp = qobject_cast<LayerPropertyWidget*>(toolbox->widget(pos - 1));
        if (lp) {
            lower_z = lp->GetLayerProperty().max_z + 1;
        }
    }
    int upper_z = ui->sbHeight->value();
    if (pos < toolbox->count() - 1)
    {
        auto* lp = qobject_cast<LayerPropertyWidget*>(toolbox->widget(pos + 1));
        if (lp) {
            upper_z = lp->GetLayerProperty().max_z - 1;
        }
    }
    new_item->SetZBounds(lower_z, upper_z, (lower_z + upper_z) / 2);
    connect(new_item, &LayerPropertyWidget::sigMaxZChanged,
            this, &MainWindow::layerBoundsChanged);
}

void MainWindow::onRemoveModelLayer()
{
    auto* toolbox = ui->toolBoxModelLayer;
    if (toolbox->currentIndex() == -1)
    {
        return;
    }
    toolbox->removeItem(toolbox->currentIndex());
    updateButtonEnabled();
    updateListOfLayerWidget();
    reinitLayerBounds();
}

void MainWindow::onGenerateClicked()
{
    using Solver = IterativeTissueSolver;
    std::vector<LayerProperty> layers;
    layers.reserve(ui->toolBoxModelLayer->count());
    for (int i = 0; i < ui->toolBoxModelLayer->count(); ++i)
    {
        auto layer_prop_widget = qobject_cast<LayerPropertyWidget*>(ui->toolBoxModelLayer->widget(i));
        if (!layer_prop_widget)
        {
            continue;
        }
        layer_prop_widget->NormalizeLayerProperty();
        layers.push_back(layer_prop_widget->GetLayerProperty());
    }
    if (layers.empty())
    {
        return;
    }
    img_width_ = ui->sbWidth->value();
    img_height_ = ui->sbHeight->value();

    QThread worker_thread(this);
    Solver solver(nullptr, layers, img_width_, img_height_);
    solver.moveToThread(&worker_thread);
    GenerationProgressDialog dialog(&worker_thread);

    connect(&worker_thread, &QThread::started, &solver, &Solver::estimate, Qt::QueuedConnection);
    connect(&solver, &Solver::finished, &worker_thread, &QThread::quit, Qt::QueuedConnection);
    connect(&solver, &Solver::finished, &dialog, &GenerationProgressDialog::accept, Qt::QueuedConnection);
    connect(&dialog, &GenerationProgressDialog::sigStopClicked, &solver, &Solver::stop, Qt::DirectConnection);
    connect(&solver, &Solver::sigStep, &dialog, &GenerationProgressDialog::onEstimationStep, Qt::QueuedConnection);

    dialog.runEstimation();

    auto data = solver.extractData();
    if (!data.empty()) {
        raw_data_ = std::move(data);
        updateImage();
    }
}

void MainWindow::onExportFile()
{
    QString file_name = QFileDialog::getSaveFileName(this, "File to export image", "",
                                                     "Image (*.png);;Table (*.csv)");
    if (file_name.isNull())
    {
        return;
    }
    export_model(file_name, img_width_, img_height_, raw_data_,
                 ui->colorLower->color(), ui->colorUpper->color());
}

