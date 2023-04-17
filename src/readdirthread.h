#ifndef READDIRTHREAD_H
#define READDIRTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMap>

#include "directory.h"

class ReadDirThread : public QThread
{
    Q_OBJECT

public:
    //	ReadDirThread();
    //	~ReadDirThread();

    void setRootPath(const QString &path, bool recursive)
    {
        rootPath = path;
        this->recursive = recursive;
    }
    QString errorMessage() const { return error; }
    void abort();

    QVector<Directory> directories;
    QMap<QString, QString> replacements;

signals:
    void processing(QString path);

protected:
    void run();

private:
    QString rootPath;
    bool recursive;
    QString error;
    volatile bool aborted;
    QMutex m_aborted;

    bool readDirectory(const QString &path);
    void analyseNames();
    void analyseName(const QString &s);
};
#endif // READDIRTHREAD_H
