#pragma once

#include <QWidget>

#include "layer_property.h"

namespace Ui {
class LayerPropertyWidget;
}

class LayerPropertyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LayerPropertyWidget(QWidget *parent = nullptr);
    explicit LayerPropertyWidget(const LayerProperty& lp, QWidget* parent = nullptr);
    ~LayerPropertyWidget();

    void SetLayerProperty(const LayerProperty& lp);
    LayerProperty GetLayerProperty() const;
    void NormalizeLayerProperty();

    void SetMultilayerGroupVisible(bool visible);
    void SetZBounds(int from, int to);
    void SetZBounds(int from, int to, int val);
    void SetMaxZMaximum(int to);
    void SetMaxZMinimum(int from);
    void SetIndex(int idx)
    {
        idx_ = idx;
    }
    int GetIndex() const
    {
        return idx_;
    }

signals:
    void sigMaxZChanged(int val, int idx);

private:
    Ui::LayerPropertyWidget *ui;
    int idx_;
};
