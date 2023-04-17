/*
    This file is part of H2rename.

    Copyright (C) 2009 by Harald Bögeholz / c't Magazin für Computertechnik
    www.ctmagazin.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef H2RENAME_H
#define H2RENAME_H

#include <QDialog>
#include <QAction>

#include "ui_h2rename.h"

#include "directory.h"
#include "readdirthread.h"

class H2rename : public QWidget
{
    Q_OBJECT

public:
    H2rename(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~H2rename();
    enum Colors
    {
        CollisionColor = 0xff0000,
        HighlightColor = 0xffe489,
        ErrorColor = 0xff0000
    };

private:
    Ui::H2renameClass ui;
    QAction *insertRuleAction;
    QAction *removeRulesAction;
    QAction *createRuleAction;
    ReadDirThread readDirThread;
    QList<Name *> names;
    QMap<QString, QString> replacements;

private:
    void readDirectory(const QString &path, bool recursive, Name *parent);

public:
    static QChar parseUTF8Char(QString::const_iterator &p, QString::const_iterator end);

public slots:
    void updateNumbers();
    void createRule();

private slots:
    void on_pushButton_Load_clicked();
    void on_comboBox_currentIndexChanged(int);
    void replacementsSelectionChanged();
    void namesSelectionChanged();
    void collisionDetected(const QModelIndex &);
    void on_pushButton_Rename_clicked();
    void on_lineEdit_DirName_returnPressed();
    void on_pushButton_removeRow_clicked();
    void on_pushButton_insertRow_clicked();
    void readDirs();
    void on_pushButton_SelectDir_clicked();
};


bool isLongerThan(const QString &s, const QString &t);

#endif // H2RENAME_H
