#pragma once

#include <boost/numeric/ublas/matrix.hpp>

#include <QAbstractTableModel>

class MatrixModel : public QAbstractTableModel
{
    Q_OBJECT

 public:
    MatrixModel(QObject* parent, int r, int c, QStringList header);

    int rowCount(const QModelIndex& = {}) const final {
        return static_cast<int>(matrix_.size1());
    }
    int columnCount(const QModelIndex& = {}) const final {
        return static_cast<int>(matrix_.size2());
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) final;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final;
    Qt::ItemFlags flags(const QModelIndex& index) const final {
        return (QAbstractItemModel::flags(index) | Qt::ItemIsEditable);
    }

 private:
    boost::numeric::ublas::matrix<double> matrix_;
    QStringList header_list_;
};
