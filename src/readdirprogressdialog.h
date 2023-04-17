#ifndef READDIRPROGRESSDIALOG_H
#define READDIRPROGRESSDIALOG_H

#include <QDialog>
#include <QObject>
#include <QWidget>

#include "ui_ReadDirProgress.h"

class ReadDirProgressDialog : public QDialog
{
    Q_OBJECT

public:
    ReadDirProgressDialog(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    Ui::ReadDirProgressDialog ui;
};
#endif // READDIRPROGRESSDIALOG_H
