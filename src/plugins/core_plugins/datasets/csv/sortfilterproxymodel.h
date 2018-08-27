#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SortFilterProxyModel(QObject *parent = 0);
    virtual ~SortFilterProxyModel(){}

    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    QList<QPair<int, Qt::SortOrder> > sortedColumns() {return m_sortedColumns;}
    void setSortedColumns(QList<QPair<int, Qt::SortOrder> > sortedColumns);

    void addSortedColumn(int column, Qt::SortOrder order);
    void removeSortedColumn(int column);
    void changeSortedColumn(int column, Qt::SortOrder order);

    Qt::SortOrder sortOrder() const {return m_sortOrder;}
    Qt::SortOrder sortOrder(int column) const;

    bool isColumnInSort(int column) const;

    void triggerColumn(int column, bool withModifier = false);

    void update();

protected:
    void sort();
    bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const;

public slots:
    void clearSort();

private:
    QList<QPair<int, Qt::SortOrder> > m_sortedColumns;
    Qt::SortOrder m_sortOrder;
};

#endif // SORTFILTERPROXYMODEL_H
