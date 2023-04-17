#ifndef RENAMERULE_H
#define RENAMERULE_H

#include <QString>

class RenameRule
{
public:
    RenameRule(const QString &s1 = QString(), const QString &s2 = QString())
    {
        column[0] = s1;
        column[1] = s2;
        highlight = false;
    }
    enum Columns
    {
        Search = 0,
        Replace,
        NCOLUMNS
    };
    QString column[NCOLUMNS];
    bool highlight;
};
#endif // RENAMERULE_H
