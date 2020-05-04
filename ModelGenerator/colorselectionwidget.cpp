#include "colorselectionwidget.h"
#include "ui_colorselectionwidget.h"

#include <QColorDialog>

ColorSelectionWidget::ColorSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorSelectionWidget)
{
    ui->setupUi(this);
    setColor(color_);
    connect(ui->pbDialogButton, &QPushButton::clicked, [this]() {onButtonClicked();});
}

void ColorSelectionWidget::setColor(QColor color)
{
    static const QString stylesheet_pattern =
            QString::fromStdString(R"style(
                background:rgb(%1,%2,%3);
                border-style: outset;
                border-width: 3px;
            )style");
    QString style = stylesheet_pattern.arg(color.red())
            .arg(color.green()).arg(color.blue());
    ui->pbDialogButton->setStyleSheet(style);
    color_ = color;
    emit sigColorChanged();
}

QColor ColorSelectionWidget::color() const
{
    return color_;
}

QString ColorSelectionWidget::text() const
{
    return ui->pbDialogButton->text();
}

void ColorSelectionWidget::setText(QString str)
{
    ui->pbDialogButton->setText(str);
}

void ColorSelectionWidget::onButtonClicked()
{
    QColorDialog dialog(color_);
    dialog.setWindowTitle(text());
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }
    auto color = dialog.currentColor();
    setColor(color);
}
