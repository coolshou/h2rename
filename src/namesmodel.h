#ifndef NAMESMODEL_H
#define NAMESMODEL_H

#include <QAbstractTableModel>
#include "directory.h"

class NamesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static NamesModel &globalInstance() // Singleton
    {
        static NamesModel namesModel;
        return namesModel;
    }
    enum Columns
    {
        OldName = 0,
        NewName,
        NCOLUMNS
    };
    void setDirectories(const QVector<Directory> &directories);
    void clear();
    void setHighlights(const QVector<bool> &highlights);
    void clearHighlights();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool isChanged(const QModelIndex &index) const;
    bool isCollision(const QModelIndex &index) const;
    bool isHighlight(const QModelIndex &index) const;
    int changedNamesCount() const;
    int collisionCount() const;
    const Name &constNameAt(int row) const;
    Name &nameAt(int row);

public slots:
    void computeNewNames();

signals:
    void collisionDetected(const QModelIndex &firstCollision);
    void highlightsChanged();

private:
    NamesModel(QObject *parent = 0);
    NamesModel(NamesModel const &); // Singleton
    NamesModel &operator=(NamesModel const &);

    QVector<Directory> directories;
    QVector<int> rowOffset; // one int per directory: row# for dir.names.at(0)
    int namesCount;			// total number of names in all directories
    int nChanged;
    int nCollisions;
    QVector<bool> highlights;
};
#endif //NAMESMODEL_H
