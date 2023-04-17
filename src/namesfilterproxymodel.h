#ifndef NAMESFILTERPROXYMODEL_H
#define NAMESFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>

class NamesFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    static NamesFilterProxyModel &globalInstance() // Singleton
    {
        static NamesFilterProxyModel namesFilterProxyModel;
        return namesFilterProxyModel;
    }
    enum Type
    {
        ShowAll = 0,
        ShowChanged,
        ShowHighlights,
        ShowCollisions
    };

    Type filterType() { return curFilter; }

public slots:
    void setFilterType(Type filterType)
    {
        curFilter = filterType;
        invalidateFilter();
    }
    void highlightsChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    NamesFilterProxyModel() { curFilter = ShowAll; }
    NamesFilterProxyModel(NamesFilterProxyModel const &); // Singleton
    NamesFilterProxyModel &operator=(NamesFilterProxyModel const &);

    Type curFilter;
};
#endif // NAMESFILTERPROXYMODEL_H
