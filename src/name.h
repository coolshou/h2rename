#ifndef NAME_H
#define NAME_H

#include <QString>
#include <QObject>
#include <QCoreApplication>

#include "directory.h"

class Name
{
    Q_DECLARE_TR_FUNCTIONS(Name)
public:
    enum Type
    {
        Undefined,
        File,
        Directory,
        Application
    };

    Name(const QString &name = QString(), Type type = Undefined,
         const QString &path = QString(), const QString &newname = QString())
        : name(name), type(type), path(path), newname(newname), isChanged(false), isUnique(true)
    {
    }
    QString typeString(void) const
    {
        switch (type)
        {
        case Directory:
            return tr("Verzeichnis");
        case Application:
            return tr("Anwendung");
        default:
            return tr("Datei");
        }
    };
    QString name;
    Type type;
    QString path;
    QString newname;
    bool isChanged;
    bool isUnique;
    QVector<bool> usedRule;
};
#endif // NAME_H
