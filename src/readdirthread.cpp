#include <QFileInfo>
#include <QDir>

#include "readdirthread.h"
#include "name.h"
#include "h2rename.h"

void ReadDirThread::run()
{
    {
        QMutexLocker locker(&m_aborted);
        aborted = false;
    }
    error.clear();
    directories.clear();
    replacements.clear();

    readDirectory(rootPath);
    analyseNames();
}

void ReadDirThread::abort()
{
    QMutexLocker locker(&m_aborted);
    aborted = true;
}

bool ReadDirThread::readDirectory(const QString &path)
{
    {
        QMutexLocker locker(&m_aborted);
        if (aborted)
            return false;
    }
    QDir dir(path);
    if (!dir.exists())
    {
        error = tr("Das Verzeichnis '%1' existiert nicht").arg(path);
        return false;
    }
    emit processing(path);

    Directory currentDir(path);
    foreach (QFileInfo subdir, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
    {
        if (recursive && !subdir.isBundle())
        {
            if (!readDirectory(path + QDir::separator() + subdir.fileName()))
                return false;
        }
        if (subdir.isBundle())
            currentDir.names.append(Name(subdir.fileName(), Name::Application, path));
        else
            currentDir.names.append(Name(subdir.fileName(), Name::Directory, path));
    }
    foreach (QString file, dir.entryList(QDir::Files))
    {
        currentDir.names.append(Name(file, Name::File, path));
    }
    directories.append(currentDir);

    return true;
}

void ReadDirThread::analyseNames()
{
    QVector<Directory>::const_iterator d;
    for (d = directories.constBegin(); d != directories.constEnd(); ++d)
    {
        QVector<Name>::const_iterator p;
        for (p = d->names.constBegin(); p != d->names.constEnd(); ++p)
            analyseName(p->name);
    }
}

void ReadDirThread::analyseName(const QString &s)
{
    QString::const_iterator p;
    for (p = s.constBegin(); p != s.constEnd();)
    {
        QString decoded;
        QString::const_iterator start = p;

        QChar c = H2rename::parseUTF8Char(p, s.constEnd());
        if (!c.isNull())
        {
            do
            {
                decoded += c;
                c = H2rename::parseUTF8Char(p, s.constEnd());
            } while (!c.isNull());
            for (;;)
            {
                QString::const_iterator q;
                QString dedecoded;
                QChar cc;
                for (q = decoded.constBegin(); !(cc = H2rename::parseUTF8Char(q, decoded.constEnd())).isNull();)
                    dedecoded += cc;
                if (q == decoded.constEnd())
                    decoded = dedecoded;
                else
                    break;
            }
            QString replace;
            for (QString::const_iterator q = start; q != p; ++q)
                replace += *q;
            if (!replacements.contains(replace))
            {
                replacements[replace] = decoded;
            }
        }
        else
            ++p;
    }
}
