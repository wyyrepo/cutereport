#include "sortfilterproxymodel.h"

#include <QVariant>
#include <QDate>

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
    , m_sortOrder(Qt::AscendingOrder)
{

}

QVariant SortFilterProxyModel::headerData ( int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if(orientation == Qt::Horizontal) {
            for (int i = 0; i < m_sortedColumns.size(); i++) {
                if (m_sortedColumns.at(i).first == section) {
                    QString sortIndicator = m_sortedColumns.at(i).second == Qt::AscendingOrder ? QString(0x25BC) : QString(0x25B2) ;
                    if (i > 0)
                        sortIndicator += QString::number(i + 1);
                    return sortIndicator + " " + sourceModel()->headerData(section, orientation, role).toString();
                }
            }
        }
        break;
    case Qt::ToolTipRole: return tr("Hold CTRL and click another header's section to choose next sorting level.");
    default: return QSortFilterProxyModel::headerData(section, orientation, role);
    }

    return QSortFilterProxyModel::headerData(section, orientation, role);
}


void SortFilterProxyModel::setSortedColumns(QList<QPair<int, Qt::SortOrder> > sortedColumns)
{
    m_sortedColumns = sortedColumns;
}


void SortFilterProxyModel::addSortedColumn(int column, Qt::SortOrder order)
{
    m_sortOrder = order;
    QPair<int, Qt::SortOrder> pair;
    pair.first = column;
    pair.second = order;
    m_sortedColumns << pair;
}


void SortFilterProxyModel::removeSortedColumn(int column)
{
    for (int i = 0; i < m_sortedColumns.size(); i++) {
        if (m_sortedColumns.at(i).first == column) {
            m_sortedColumns.removeAt(i);
            break;
        }
    }
}


void SortFilterProxyModel::changeSortedColumn(int column, Qt::SortOrder order)
{
    for (int i = 0; i < m_sortedColumns.size(); i++) {
        if (m_sortedColumns.at(i).first == column) {
            m_sortOrder = order;
            m_sortedColumns[i].second = order;
            break;
        }
    }
}


void SortFilterProxyModel::clearSort()
{
    beginResetModel();
    m_sortedColumns.clear();
    endResetModel();
}


void SortFilterProxyModel::sort()
{
    int column = m_sortedColumns.isEmpty() ? -1 : 0;
    QSortFilterProxyModel::sort(column, Qt::AscendingOrder);
}


bool SortFilterProxyModel::lessThan ( const QModelIndex & left, const QModelIndex & right ) const
{
    for (int i = 0; i < m_sortedColumns.size(); i++) {
        QVariant leftData = sourceModel()->index(left.row(), m_sortedColumns.at(i).first).data(Qt::DisplayRole);
        QVariant rightData = sourceModel()->index(right.row(), m_sortedColumns.at(i).first).data(Qt::DisplayRole);
        Qt::SortOrder order = m_sortedColumns.at(i).second;

        if (leftData == rightData)
            continue;

        switch (leftData.type()) {
        case QVariant::Int:
            return (leftData.toInt() < rightData.toInt()) ^ order;
            break;
        case QVariant::UInt:
            return (leftData.toUInt() < rightData.toUInt()) ^ order;
            break;
        case QVariant::LongLong:
            return (leftData.toLongLong() < rightData.toLongLong()) ^ order;
            break;
        case QVariant::ULongLong:
            return (leftData.toULongLong() < rightData.toULongLong()) ^ order;
            break;
        case QVariant::Double:
            return (leftData.toDouble() < rightData.toDouble()) ^ order;
            break;
        case QVariant::Char:
            return (leftData.toChar() < rightData.toChar()) ^ order;
            break;
        case QVariant::Date:
            return (leftData.toDate() < rightData.toDate()) ^ order;
            break;
        case QVariant::Time:
            return (leftData.toTime() < rightData.toTime()) ^ order;
            break;
        case QVariant::DateTime:
            return (leftData.toDateTime() < rightData.toDateTime()) ^ order;
            break;
        case QVariant::String: default:
            return (QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0) ^ order;
            break;
        }
    }
    return false;
}


Qt::SortOrder SortFilterProxyModel::sortOrder(int column) const
{
    for (int i = 0; i < m_sortedColumns.size(); i++) {
        if (m_sortedColumns.at(i).first == column) {
            return m_sortedColumns.at(i).second;
        }
    }
    return Qt::AscendingOrder;
}


bool SortFilterProxyModel::isColumnInSort(int column) const
{
    for (int i = 0; i < m_sortedColumns.size(); i++) {
        if (m_sortedColumns.at(i).first == column) {
            return true;
        }
    }
    return false;
}


void SortFilterProxyModel::triggerColumn(int column, bool withModifier)
{
    if (withModifier) {
        if (isColumnInSort(column)) {
            Qt::SortOrder order = (sortOrder(column) == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
            changeSortedColumn(column, order);
        } else {
            addSortedColumn(column, sortOrder());
        }
    } else {
        Qt::SortOrder order = sortOrder();
        if (isColumnInSort(column)) {
            order = (sortOrder(column) == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
        }
        clearSort();
        addSortedColumn(column, order);
    }
}


void SortFilterProxyModel::update()
{
    sort();
}
