#include "renamerulesmodel.h"
#include "h2rename.h"

RenameRulesModel::RenameRulesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

bool RenameRulesModel::containsSearchString(const QString &s) const
{
    for (QList<RenameRule>::const_iterator p = rules.constBegin(); p != rules.constEnd(); ++p)
        if (p->column[RenameRule::Search] == s)
            return true;
    return false;
}

void RenameRulesModel::appendRules(const QList<RenameRule> &rules)
{
    beginResetModel();
    for (QList<RenameRule>::const_iterator p = rules.constBegin(); p != rules.constEnd(); ++p)
    {
        if (!containsSearchString(p->column[RenameRule::Search]))
        {
            this->rules.append(*p);
            // nColumns = 3;
        }
    }
    endResetModel();
    emit rulesChanged();
}

void RenameRulesModel::setHighlights(const QVector<bool> &highlights)
{
    for (int i = 0; i < rules.count(); ++i)
        if (i < highlights.count() && highlights.at(i))
            rules[i].highlight = true;
        else
            rules[i].highlight = false;
    emit dataChanged(createIndex(0, 0), createIndex(rules.count(), 1));
}

void RenameRulesModel::clearHighlights()
{
    for (int i = 0; i < rules.count(); ++i)
        rules[i].highlight = false;
    emit dataChanged(createIndex(0, 0), createIndex(rules.count(), 1));
}

void RenameRulesModel::clear()
{
    beginResetModel();
    rules.clear();
    endResetModel();
    emit rulesChanged();
}

int RenameRulesModel::rowCount(const QModelIndex & /* parent */) const
{
    return rules.count() + 1;
}

int RenameRulesModel::columnCount(const QModelIndex & /* parent */) const
{
    return RenameRule::NCOLUMNS;
}

QVariant RenameRulesModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        switch (role)
        {
        case Qt::TextAlignmentRole:
            return int(Qt::AlignLeft | Qt::AlignVCenter);

        case Qt::DisplayRole:
        case Qt::EditRole:
            if (index.row() >= rules.count())
                return QString();
            else
                return rules.at(index.row()).column[index.column()];

        case Qt::ForegroundRole:
            if (index.row() < rules.count() &&
                (rules.at(index.row()).column[RenameRule::Search].isEmpty() ||
                 rules.at(index.row()).column[RenameRule::Replace].contains(QChar('\\')) ||
                 rules.at(index.row()).column[RenameRule::Replace].contains(QChar('/')) ||
                 rules.at(index.row()).column[RenameRule::Replace].contains(QChar(':'))))
                return QColor(H2rename::ErrorColor);
            break;

        case Qt::ToolTipRole:
            if (index.row() < rules.count() && rules.at(index.row()).column[RenameRule::Search].isEmpty())
                return tr("ACHTUNG: Suchstring leer!");
            if (index.row() < rules.count() && rules.at(index.row()).column[RenameRule::Replace].contains(QChar('\\')))
                return tr("ACHTUNG: Ersetzungsstring enthält einen Backslash (\\)");
            if (index.row() < rules.count() && rules.at(index.row()).column[RenameRule::Replace].contains(QChar('/')))
                return tr("ACHTUNG: Ersetzungsstring enthält einen Schrägstrich (/)");
            if (index.row() < rules.count() && rules.at(index.row()).column[RenameRule::Replace].contains(QChar(':')))
                return tr("ACHTUNG: Ersetzungsstring enthält einen Doppelpunkt!");
            break;

        case Qt::BackgroundRole:
            if (index.row() < rules.count() && rules.at(index.row()).highlight)
                return QColor(H2rename::HighlightColor);
            break;
        }
    }
    return QVariant();
}

bool RenameRulesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        if (index.row() >= rules.count())
        {
            beginInsertRows(QModelIndex(), index.row() + 1, index.row() + 1);
            rules.append(RenameRule());
            endInsertRows();
        }
        rules[index.row()].column[index.column()] = value.toString();

        emit dataChanged(index, index);
        emit rulesChanged();

        return true;
    }
    return false;
}

QVariant RenameRulesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return (QStringList() << tr("Ersetze") << tr("durch") << tr("Regel automatisch erzeugt aus")).at(section);
    return QVariant();
}

Qt::ItemFlags RenameRulesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == RenameRule::Search || index.column() == RenameRule::Replace)
        flags |= Qt::ItemIsEditable;
    return flags;
}

bool RenameRulesModel::insertRow(int row, const QModelIndex &parent)
{
    if (row >= 0 && row <= rules.size())
    {
        beginInsertRows(parent, row, row);
        rules.insert(row, RenameRule());
        endInsertRows();
        return true;
    }
    return false;
}

QModelIndex RenameRulesModel::appendCreatedRule(const QString &text)
{
    RenameRule newRule(text, text);
    beginInsertRows(QModelIndex(), rules.count(), rules.count());
    rules.append(newRule);
    endInsertRows();
    emit rulesChanged();
    return index(rules.count() - 1, 0);
}

QModelIndex RenameRulesModel::prependCreatedRule(const QString &text)
{
    RenameRule newRule(text, text);
    beginInsertRows(QModelIndex(), 0, 0);
    rules.insert(0, newRule);
    endInsertRows();
    emit rulesChanged();
    return index(0, 0);
}

void RenameRulesModel::removeRowList(const QVector<bool> &rowlist)
{
    beginResetModel();
    for (int i = rowlist.count() - 1; i >= 0; --i)
        if (rowlist.at(i) && i < rules.count())
            rules.removeAt(i);
    endResetModel();
    emit rulesChanged();
}

const QString &RenameRulesModel::searchAt(int row) const
{
    return rules.at(row).column[RenameRule::Search];
}

const QString &RenameRulesModel::replaceAt(int row) const
{
    return rules.at(row).column[RenameRule::Replace];
}
