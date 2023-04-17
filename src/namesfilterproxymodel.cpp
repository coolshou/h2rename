#include "namesfilterproxymodel.h"
#include "namesmodel.h"

void NamesFilterProxyModel::highlightsChanged()
{
    if (curFilter == ShowHighlights)
        // filterChanged();
        invalidateFilter();
}

bool NamesFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    switch (curFilter)
    {
    case ShowChanged:
        return static_cast<NamesModel *>(sourceModel())->isChanged(index);
    case ShowHighlights:
        return static_cast<NamesModel *>(sourceModel())->isHighlight(index);
    case ShowCollisions:
        return static_cast<NamesModel *>(sourceModel())->isCollision(index);
    default:;
    }
    return true;
}
