#include <QObject>
#include <QAbstractTableModel>
#ifndef RENAMERULESMODEL_H
#define RENAMERULESMODEL_H

#include "renamerule.h"

class RenameRulesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static RenameRulesModel &globalInstance() // Singleton
    {
        static RenameRulesModel renameRulesModel;
        return renameRulesModel;
    }
    bool containsSearchString(const QString &s) const;
    void appendRules(const QList<RenameRule> &rules);
    QModelIndex appendCreatedRule(const QString &text);
    QModelIndex prependCreatedRule(const QString &text);
    void setHighlights(const QVector<bool> &highlights);
    void clearHighlights();
    void clear();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRow(int row, const QModelIndex &parent = QModelIndex());

    void removeRowList(const QVector<bool> &rowlist);
    const QString &searchAt(int row) const;
    const QString &replaceAt(int row) const;

signals:
    void rulesChanged();

private:
    RenameRulesModel(QObject *parent = 0); // Singleton
    RenameRulesModel(RenameRulesModel const &);
    RenameRulesModel &operator=(RenameRulesModel const &);

    QList<RenameRule> rules;
};
#endif //RENAMERULESMODEL_H
