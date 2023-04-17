#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QString>
#include "name.h"

class Directory
{
public:
    Directory(const QString &path = QString())
        : path(path), nChanged(0), nCollisions(0)
    {
    }

    QVector<Name> names;
    QString path;
    int nChanged;
    int nCollisions;
};
#endif // DIRECTORY_H
