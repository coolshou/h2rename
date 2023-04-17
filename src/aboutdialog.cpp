#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QString>

#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->te_new->append(COMPANY);
    ui->te_new->append(QString(COPYRIGHT)+QString(" ")+QString(COMPANY));
    //ui->te_org->setHtml();

}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
