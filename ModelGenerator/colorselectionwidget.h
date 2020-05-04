#pragma once

#include <QScopedPointer>
#include <QWidget>

namespace Ui {
class ColorSelectionWidget;
}

class ColorSelectionWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit ColorSelectionWidget(QWidget *parent = nullptr);
    ~ColorSelectionWidget() = default;

    void setColor(QColor color);
    QColor color() const;
    QString text() const;
    void setText(QString str);

signals:
    void sigColorChanged();

private:
   void onButtonClicked();

    QScopedPointer<Ui::ColorSelectionWidget> ui;
    QColor color_;
};
