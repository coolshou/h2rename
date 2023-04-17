#include <QtConcurrent>

#include "namesmodel.h"
#include "name.h"
#include "h2rename.h"
#include "namesfilterproxymodel.h"
#include "renamerulesmodel.h"

void applyRename(Directory &dir)
{
    QSet<QString> uniqueNames;
    dir.nChanged = dir.nCollisions = 0;
    for (QVector<Name>::iterator p = dir.names.begin(); p != dir.names.end(); ++p)
    {
        // p->newname = RenameRulesModel::globalInstance().applyRules(p->name);
        p->newname = p->name;
        p->usedRule.resize(RenameRulesModel::globalInstance().rowCount(QModelIndex()) - 1);
        for (int i = 0; i < RenameRulesModel::globalInstance().rowCount(QModelIndex()) - 1; ++i)
        {
            const QString &search = RenameRulesModel::globalInstance().searchAt(i);
            const QString &replace = RenameRulesModel::globalInstance().replaceAt(i);
            if (!search.isEmpty() && p->newname.contains(search))
            {
                p->newname = p->newname.replace(search, replace);
                p->usedRule[i] = true;
            }
            else
                p->usedRule[i] = false;
        }

        p->isChanged = (p->name != p->newname);
        if (p->isChanged)
            ++dir.nChanged;
        if (uniqueNames.contains(p->newname))
        {
            p->isUnique = false;
            ++dir.nCollisions;
        }
        else
        {
            p->isUnique = true;
            uniqueNames.insert(p->newname);
        }
    }
}

NamesModel::NamesModel(QObject *parent)
    : QAbstractTableModel(parent), namesCount(0), nChanged(0), nCollisions(0)
{
}

void NamesModel::setDirectories(const QVector<Directory> &directories)
{
    beginResetModel();
    this->directories = directories;
    rowOffset.clear();
    int row = 0;
    for (QVector<Directory>::const_iterator p = directories.constBegin(); p != directories.constEnd(); ++p)
    {
        rowOffset.append(row);
        row += p->names.count();
    }
    namesCount = row;

    computeNewNames();
    endResetModel();
}

void NamesModel::clear()
{
    beginResetModel();
    directories.clear();
    rowOffset.clear();
    namesCount = 0;
    nChanged = 0;
    nCollisions = 0;
    endResetModel();
}

void NamesModel::setHighlights(const QVector<bool> &highlights)
{
    this->highlights = highlights;
    if (namesCount > 0)
    {
        emit dataChanged(index(0, 0), index(namesCount - 1, 1));
        emit highlightsChanged();
    }
}

void NamesModel::clearHighlights()
{
    highlights.clear();
    if (namesCount > 0)
        emit dataChanged(index(0, 0), index(namesCount - 1, 1));
}

int NamesModel::rowCount(const QModelIndex & /* parent */) const
{
    return namesCount;
}

int NamesModel::columnCount(const QModelIndex & /* parent */) const
{
    return 2;
}

QVariant NamesModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        switch (role)
        {
        case Qt::TextAlignmentRole:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        case Qt::DisplayRole:
            switch (index.column())
            {
            case 0:
                return constNameAt(index.row()).name;
            case 1:
                return constNameAt(index.row()).newname;
            default:
                return tr("FIXME");
            }
        case Qt::ToolTipRole:
            return (constNameAt(index.row()).typeString()) + " in " + constNameAt(index.row()).path;
        case Qt::BackgroundRole:
            if (isCollision(index))
                return QColor(H2rename::CollisionColor);
            if (isHighlight(index))
                return QColor(H2rename::HighlightColor);
            break;
        case Qt::UserRole:
            return isHighlight(index);
        }
    }
    return QVariant();
}

QVariant NamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return (QStringList() << tr("Originalname") << tr("Neuer Name")).at(section);
    return QVariant();
}

const Name &NamesModel::constNameAt(int row) const
{
    // QVector<int>::const_iterator p = qUpperBound(rowOffset.constBegin(), rowOffset.constEnd(), row);
    QVector<int>::const_iterator p = std::upper_bound(rowOffset.constBegin(), rowOffset.constEnd(), row);
    if (p == rowOffset.constEnd() || *p > row)
        --p; // müsste eigentlich jedes Mal passieren
    int i = p - rowOffset.constBegin();
    int j = row - *p;
    return directories.at(i).names.at(j);
}

Name &NamesModel::nameAt(int row)
{
    // QVector<int>::const_iterator p = qUpperBound(rowOffset.constBegin(), rowOffset.constEnd(), row);
    QVector<int>::const_iterator p = std::upper_bound(rowOffset.constBegin(), rowOffset.constEnd(), row);
    if (p == rowOffset.constEnd() || *p > row)
        --p; // müsste eigentlich jedes Mal passieren
    int i = p - rowOffset.constBegin();
    int j = row - *p;
    return directories[i].names[j];
}

bool NamesModel::isChanged(const QModelIndex &index) const
{
    if (index.isValid())
        return constNameAt(index.row()).isChanged;
    return false;
}

bool NamesModel::isCollision(const QModelIndex &index) const
{
    if (index.isValid())
        return !constNameAt(index.row()).isUnique;
    return false;
}

bool NamesModel::isHighlight(const QModelIndex &index) const
{
    if (index.isValid())
    {
        const Name &name = constNameAt(index.row());
        for (int i = 0; i < highlights.count() && i < name.usedRule.count(); ++i)
            if (highlights.at(i) && name.usedRule.at(i))
                return true;
    }
    return false;
}

int NamesModel::changedNamesCount() const
{
    return nChanged;
}

int NamesModel::collisionCount() const
{
    return nCollisions;
}

void NamesModel::computeNewNames()
{
    if (namesCount > 0)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        QtConcurrent::blockingMap(directories, applyRename);

        int firstCollision = -1;
        nChanged = nCollisions = 0;
        for (QVector<Directory>::const_iterator p = directories.constBegin(); p != directories.constEnd(); ++p)
        {
            nChanged += p->nChanged;
            nCollisions += p->nCollisions;
            if (firstCollision == -1 && p->nCollisions > 0)
            { // find first collision
                for (int i = 0; i < p->names.count(); ++i)
                    if (!p->names.at(i).isUnique)
                    {
                        firstCollision = i + rowOffset.at(p - directories.constBegin());
                        break;
                    }
            }
        }

        emit dataChanged(index(0, 1, QModelIndex()), index(namesCount - 1, 1, QModelIndex()));
        NamesFilterProxyModel::globalInstance().invalidate();
        if (nCollisions > 0)
            emit collisionDetected(index(firstCollision, 1));
        QApplication::restoreOverrideCursor();
    }
}
