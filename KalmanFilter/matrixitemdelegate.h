#pragma once

#include <QStyledItemDelegate>

class MatrixItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MatrixItemDelegate(QObject* parent = nullptr);

    QString displayText(const QVariant &value, const QLocale &locale) const final;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const final;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const final;
};
