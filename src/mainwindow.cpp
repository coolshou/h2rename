#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "version.h"


#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    h2rename = new H2rename(this);
    setCentralWidget(h2rename);
    setWindowTitle(QString(H2RENAME_NAME) + QString(" ") + QString(H2RENAME_VER));

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(actAbout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::actAbout()
{
    qDebug() << "actAbout" << Qt::endl;
    dlg = new AboutDialog(this);
    dlg->exec();
}
