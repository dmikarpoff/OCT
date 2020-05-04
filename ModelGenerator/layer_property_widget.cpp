#include "layer_property_widget.h"

#include "ui_layer_property_widget.h"

#include <general_utils.h>

#include <QSpinBox>

#include <limits>

LayerPropertyWidget::LayerPropertyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LayerPropertyWidget)
{
    ui->setupUi(this);
    ui->spinboxMaxZ->setMaximum(std::numeric_limits<int>::max());
    connect(ui->spinboxMaxZ, QOverload<int>::of(&QSpinBox::valueChanged),
            [this] () {
                emit sigMaxZChanged(ui->spinboxMaxZ->value(), idx_);
            }
    );
}

LayerPropertyWidget::LayerPropertyWidget(const LayerProperty& lp, QWidget* parent)
        : LayerPropertyWidget(parent)
{
    SetLayerProperty(lp);
}

void LayerPropertyWidget::SetLayerProperty(const LayerProperty &lp)
{
    ui->spinboxA->setValue(lp.a);
    ui->spinboxB->setValue(lp.b);
    ui->spinboxC->setValue(lp.c);
    ui->spinboxD->setValue(lp.d);
    ui->spinboxAlpha->setValue(lp.alpha);
    ui->sbSigma->setValue(lp.sigma);
    ui->sbBorderAlpha->setValue(lp.border_alpha);
    ui->spinboxMaxZ->setValue(lp.max_z);
}

LayerPropertyWidget::~LayerPropertyWidget()
{
    delete ui;
}

LayerProperty LayerPropertyWidget::GetLayerProperty() const
{
    LayerProperty prop;
    prop.a = ui->spinboxA->value();
    prop.b = ui->spinboxB->value();
    prop.c = ui->spinboxC->value();
    prop.d = ui->spinboxD->value();
    prop.alpha = ui->spinboxAlpha->value();
    prop.sigma = ui->sbSigma->value();
    prop.border_alpha = ui->sbBorderAlpha->value();
    prop.max_z = ui->spinboxMaxZ->value();
    return prop;
}

void LayerPropertyWidget::NormalizeLayerProperty()
{
    auto norm_lp = Normalize(GetLayerProperty());
    SetLayerProperty(norm_lp);
}

void LayerPropertyWidget::SetMultilayerGroupVisible(bool visible)
{
    ui->lineGroupSeparation->setVisible(visible);
    ui->widgetBorderDescr->setVisible(visible);
}

void LayerPropertyWidget::SetZBounds(int from, int to)
{
    QSignalBlocker blocker(ui->spinboxMaxZ);

    ui->spinboxMaxZ->setRange(from, to);
    ui->spinboxMaxZ->setValue(utils::EnforceInRange(ui->spinboxMaxZ->value(), from, to));
}

void LayerPropertyWidget::SetZBounds(int from, int to, int val)
{
    val = std::max(from, std::min(val, to));
    SetZBounds(from, to);
    ui->spinboxMaxZ->setValue(val);
}

void LayerPropertyWidget::SetMaxZMaximum(int to)
{
    SetZBounds(ui->spinboxMaxZ->minimum(), to);
}

void LayerPropertyWidget::SetMaxZMinimum(int from)
{
    SetZBounds(from, ui->spinboxMaxZ->maximum());
}
