#include "matrixitemdelegate.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QFontMetrics>
#include <QLocale>
#include <QStyleOptionSpinBox>

MatrixItemDelegate::MatrixItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

QString MatrixItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    bool good = false;
    double val = value.toDouble(&good);
    if (!good) {
        return {};
    }

    return locale.toString(val, 'f', 3);
}

QWidget* MatrixItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex& /*index*/) const
{
    auto* spinbox = new QDoubleSpinBox(parent);
    spinbox->setDecimals(3);
    spinbox->setSingleStep(0.01);
    spinbox->resize(option.rect.size());
    return spinbox;
}

QSize MatrixItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex&/*index*/) const
{
    QFontMetrics fm(option.font);
    auto rect = fm.boundingRect("999.999");
    QStyleOptionSpinBox spinbox_option;
    static_cast<QStyleOption&>(spinbox_option) = option;
    spinbox_option.frame = true;
    spinbox_option.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    spinbox_option.stepEnabled = (QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled);
    auto sz = QApplication::style()->sizeFromContents(QStyle::CT_SpinBox, &spinbox_option, rect.size());
    return sz;
}
