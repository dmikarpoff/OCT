#include "matrixmodel.h"

MatrixModel::MatrixModel(QObject* parent, int r, int c, QStringList header)
    : QAbstractTableModel(parent)
    , matrix_(boost::numeric::ublas::identity_matrix<double>(r, c))
    , header_list_(header) {}

QVariant MatrixModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return matrix_(index.row(), index.column());
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    default:
        return {};
    }
    return {};
}

bool MatrixModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }
    if (!index.isValid() || index.row() >= matrix_.size1() || index.column() >= matrix_.size2())
    {
        return false;
    }
    bool ok = false;
    double v = value.toDouble(&ok);
    if (!ok)
    {
        return false;
    }
    matrix_(index.row(), index.column()) = v;
    return true;
}

QVariant MatrixModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (section < 0 || section >= header_list_.size())
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return header_list_[section];
    }
    return {};
}
