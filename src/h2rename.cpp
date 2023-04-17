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

#include "h2rename.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QTime>
#include <QThreadPool>
#include <QtAlgorithms>
#include <QProgressDialog>

#include "version.h"
#include "renamerulesmodel.h"
#include "namesmodel.h"
#include "readdirprogressdialog.h"
#include "namesfilterproxymodel.h"

bool isLongerThan(const QString &s, const QString &t)
{
    return s.length() > t.length();
}

H2rename::H2rename(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    ui.setupUi(this);
    setWindowTitle(QString(H2RENAME_NAME) + QString(" ") + QString(H2RENAME_VER) );
    ui.tableView_replacements->setModel(&RenameRulesModel::globalInstance());
    ui.tableView_replacements->verticalHeader()->hide();
    ui.tableView_replacements->horizontalHeader()->setStretchLastSection(true);

    NamesFilterProxyModel::globalInstance().setSourceModel(&NamesModel::globalInstance());

    ui.tableView_names->setModel(&NamesFilterProxyModel::globalInstance());
    ui.tableView_names->verticalHeader()->hide();
    ui.tableView_names->horizontalHeader()->setStretchLastSection(true);

    insertRuleAction = new QAction(tr("Regel &einfügen"), this);
    insertRuleAction->setShortcut(tr("Ctrl+E"));
    insertRuleAction->setIconVisibleInMenu(false);
    connect(insertRuleAction, SIGNAL(triggered()),
            this, SLOT(on_pushButton_insertRow_clicked()));
    ui.tableView_replacements->addAction(insertRuleAction);

    removeRulesAction = new QAction(tr("Regel(n) &löschen"), this);
    removeRulesAction->setShortcut(tr("Ctrl+L"));
    removeRulesAction->setIconVisibleInMenu(false);
    connect(removeRulesAction, SIGNAL(triggered()),
            this, SLOT(on_pushButton_removeRow_clicked()));
    ui.tableView_replacements->addAction(removeRulesAction);

    createRuleAction = new QAction(tr("&Regel erzeugen"), this);
    createRuleAction->setShortcut(tr("Ctrl+R"));
    createRuleAction->setIconVisibleInMenu(false);
    connect(createRuleAction, SIGNAL(triggered()),
            this, SLOT(createRule()));
    ui.tableView_names->addAction(createRuleAction);

    connect(&RenameRulesModel::globalInstance(), SIGNAL(rulesChanged()),
            &NamesModel::globalInstance(), SLOT(computeNewNames()));
    connect(&RenameRulesModel::globalInstance(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            &NamesModel::globalInstance(), SLOT(computeNewNames()));
    connect(&NamesModel::globalInstance(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(updateNumbers()));
    connect(ui.tableView_names->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(namesSelectionChanged()));
    connect(ui.tableView_replacements->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(replacementsSelectionChanged()));
    connect(&NamesModel::globalInstance(), SIGNAL(collisionDetected(const QModelIndex &)),
            this, SLOT(collisionDetected(const QModelIndex &)));
    connect(&NamesModel::globalInstance(), SIGNAL(highlightsChanged()),
            &NamesFilterProxyModel::globalInstance(), SLOT(highlightsChanged()));

}

H2rename::~H2rename()
{
}

QChar H2rename::parseUTF8Char(QString::const_iterator &p, QString::const_iterator end)
{
    if (p == end)
        return QChar();

    QString::const_iterator q = p;
    ushort decoded;

    if (q->unicode() < 0x80 || q->unicode() >= 0x100)
        return QChar();
    else if ((q->unicode() & 0xffe0) == 0xc0)
    { // two byte character
        decoded = (q->unicode() & 0x1f) << 6;
        if (++q == end || (q->unicode() & 0xffc0) != 0x80)
            return QChar();
        decoded |= q->unicode() & 0x3f;
        p = ++q;
        return QChar(decoded);
    }
    else if ((q->unicode() & 0xfff0) == 0xe0)
    {
        // three byte character
        decoded = (q->unicode() & 0x0f) << 12;
        if (++q == end || (q->unicode() & 0xffc0) != 0x80)
            return QChar();
        decoded |= (q->unicode() & 0x3f) << 6;
        if (++q == end || (q->unicode() & 0xffc0) != 0x80)
            return QChar();
        decoded |= q->unicode() & 0x3f;
        p = ++q;
        return QChar(decoded);
    }
    else if ((p->unicode() & 0xf0) == 0xf0)
    { // four byte charater not implemented FIXME
        return QChar();
    }
    return QChar(); // otherwise invalid
}

void H2rename::updateNumbers()
{
    int nNames = NamesModel::globalInstance().rowCount();
    int nChanges = NamesModel::globalInstance().changedNamesCount();
    int nCollisions = NamesModel::globalInstance().collisionCount();
    ui.label_numbers->setText(tr("%1 %2, %3 %4").arg(nNames).arg(nNames == 1 ? tr("Name") : tr("Namen")).arg(nChanges).arg(nChanges == 1 ? tr("Änderung") : tr("Änderungen")));
    ui.pushButton_Rename->setEnabled(nChanges - nCollisions > 0);
    if (NamesModel::globalInstance().collisionCount() > 0)
        ui.label_collisions->setText(tr("%1 %2!").arg(NamesModel::globalInstance().collisionCount()).arg(NamesModel::globalInstance().collisionCount() == 1 ? tr("Namenskollision") : tr("Namenskollisionen")));
    else
        ui.label_collisions->setText("");
}

void H2rename::on_pushButton_SelectDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis wählen"),
                                                     ui.lineEdit_DirName->text(), QFileDialog::ShowDirsOnly);
    ui.lineEdit_DirName->setText(QDir::toNativeSeparators(path));
    if (!path.isEmpty())
        readDirs();
}

void H2rename::readDirs()
{
    if (!readDirThread.isRunning())
    {
        NamesModel::globalInstance().clear();

        ReadDirProgressDialog dialog;
        connect(&readDirThread, SIGNAL(finished()), &dialog, SLOT(accept()));
        connect(&readDirThread, SIGNAL(processing(QString)), dialog.ui.label_currentDir, SLOT(setText(QString)));

        readDirThread.setRootPath(ui.lineEdit_DirName->text(), ui.checkBox_Recursive->isChecked());
        readDirThread.start();
        if (dialog.exec() == QDialog::Accepted)
        {
            if (!readDirThread.errorMessage().isEmpty())
                QMessageBox::warning(this, tr("H2rename"), readDirThread.errorMessage(), QMessageBox::Ok);
            else
            {
                QStringList repstrings = readDirThread.replacements.keys();
                // qStableSort(repstrings.begin(), repstrings.end(), isLongerThan);
                std::stable_sort(repstrings.begin(), repstrings.end(), isLongerThan);
                QList<RenameRule> rules;
                foreach (QString replace, repstrings)
                {
                    rules.append(RenameRule(replace, readDirThread.replacements[replace]));
                }
                if (rules.count() > 0)
                {
                    RenameRulesModel::globalInstance().appendRules(rules);
                    ui.comboBox->setCurrentIndex(NamesFilterProxyModel::ShowChanged);
                    // ui.tableView_replacements->resizeColumnToContents(0);
                    // ui.tableView_replacements->resizeColumnToContents(1);
                }

                NamesModel::globalInstance().setDirectories(readDirThread.directories);
            }
        }
        else
        {
            readDirThread.abort();
            readDirThread.wait();
        }
    }
}

void H2rename::on_pushButton_insertRow_clicked()
{
    RenameRulesModel::globalInstance().insertRow(ui.tableView_replacements->currentIndex().row());
}

void H2rename::on_pushButton_removeRow_clicked()
{
    int nRules = RenameRulesModel::globalInstance().rowCount(QModelIndex()) - 1;
    if (nRules > 0)
    {
        QVector<bool> rows(nRules);
        QItemSelectionModel *sel = ui.tableView_replacements->selectionModel();
        for (int i = 0; i < nRules; ++i)
            rows[i] = sel->rowIntersectsSelection(i, QModelIndex());
        RenameRulesModel::globalInstance().removeRowList(rows);
        replacementsSelectionChanged();
    }
}

void H2rename::on_lineEdit_DirName_returnPressed()
{
    readDirs();
}

void H2rename::on_pushButton_Rename_clicked()
{
    int renameCount = NamesModel::globalInstance().changedNamesCount() -
                      NamesModel::globalInstance().collisionCount();
    if (renameCount == 0)
        return; // should not happen; button should be inactive

    QString text = tr("Sie sind im Begriff, %1 %2 umzubenennen.").arg(renameCount).arg(renameCount == 1 ? tr("Datei/Verzeichnis") : tr("Dateien/Verzeichnisse"));
    if (NamesModel::globalInstance().collisionCount() > 0)
        text += tr(" %1 %2 aufgrund von Namenskollisionen nicht umbenannt.")
                    .arg(NamesModel::globalInstance().collisionCount())
                    .arg(NamesModel::globalInstance().collisionCount() == 1 ? tr("wird") : tr("werden"));
    text += tr(" Fortfahren?");

    if (QMessageBox::Yes == QMessageBox::warning(this, tr("H2rename - Umbenennen starten?"), text,
                                                 QMessageBox::Yes | QMessageBox::No))
    {
        QProgressDialog progress(this);
        progress.setLabelText(tr("Umbenennen läuft ..."));
        progress.setRange(0, renameCount);
        progress.setModal(true);

        int nDone = 0;
        for (int i = 0; i < NamesModel::globalInstance().rowCount(); ++i)
        {
            Name &name = NamesModel::globalInstance().nameAt(i);
            if (name.isChanged && name.isUnique)
            {
                QString oldFullName = name.path + QDir::separator() + name.name;
                QString newFullName = name.path + QDir::separator() + name.newname;
                if (!QFile::rename(oldFullName, newFullName))
                {
                    QMessageBox::critical(this, tr("H2rename"),
                                          tr("Konnte %1 \"%2\" nicht in \"%3\" umbenennen.")
                                              .arg(name.typeString())
                                              .arg(oldFullName)
                                              .arg(newFullName),
                                          QMessageBox::Ok);
                    break;
                }
                name.name = name.newname;
                if (name.type == Name::Directory)
                { // clean up path names for contained files and dirs
                    for (int j = 0; j < i; ++j)
                    {
                        Name &jname = NamesModel::globalInstance().nameAt(j);
                        if (jname.path.startsWith(oldFullName))
                            jname.path = newFullName + jname.path.mid(oldFullName.length());
                    }
                }
                ++nDone;
                progress.setValue(nDone);
                qApp->processEvents();
                if (progress.wasCanceled())
                {
                    break;
                }
            }
        }
        NamesModel::globalInstance().computeNewNames();
    }
}

void H2rename::namesSelectionChanged()
{
    QItemSelectionModel *sel = ui.tableView_names->selectionModel();
    if (sel->hasSelection())
    {
        NamesModel::globalInstance().clearHighlights();
        QModelIndexList li = sel->selection().indexes();
        if (!li.isEmpty())
        {
            const QVector<bool> &highlights = NamesModel::globalInstance().constNameAt(NamesFilterProxyModel::globalInstance().mapToSource(li.at(0)).row()).usedRule;
            RenameRulesModel::globalInstance().setHighlights(highlights);
            ui.tableView_replacements->clearSelection();
            for (int i = 0; i < highlights.count(); ++i)
                if (highlights.at(i))
                {
                    ui.tableView_replacements->scrollTo(RenameRulesModel::globalInstance().index(i, 0));
                    break;
                }
        }
    }
}

void H2rename::replacementsSelectionChanged()
{
    QItemSelectionModel *sel = ui.tableView_replacements->selectionModel();
    if (sel->hasSelection())
    {
        ui.pushButton_removeRow->setEnabled(true);
        RenameRulesModel::globalInstance().clearHighlights();
        ui.tableView_names->clearSelection();
        QVector<bool> selectedRules(RenameRulesModel::globalInstance().rowCount(QModelIndex()));
        for (int i = 0; i < RenameRulesModel::globalInstance().rowCount(QModelIndex()); ++i)
            selectedRules[i] = sel->rowIntersectsSelection(i, QModelIndex());
        NamesModel::globalInstance().setHighlights(selectedRules);
        if (NamesFilterProxyModel::globalInstance().filterType() != NamesFilterProxyModel::ShowHighlights)
        {
            for (int i = 0; i < NamesFilterProxyModel::globalInstance().rowCount(QModelIndex()); ++i)
                if (NamesFilterProxyModel::globalInstance().data(NamesFilterProxyModel::globalInstance().index(i, 0), Qt::UserRole).toBool())
                {
                    ui.tableView_names->scrollTo(NamesFilterProxyModel::globalInstance().index(i, 0));
                    break;
                }
        }
    }
    else
        ui.pushButton_removeRow->setEnabled(false);
}

void H2rename::collisionDetected(const QModelIndex &firstCollision)
{
    ui.tableView_names->scrollTo(NamesFilterProxyModel::globalInstance().mapFromSource(firstCollision));
}

void H2rename::on_comboBox_currentIndexChanged(int index)
{
    NamesFilterProxyModel::globalInstance().setFilterType(static_cast<NamesFilterProxyModel::Type>(index));
}

void H2rename::createRule()
{
    QModelIndex index = ui.tableView_names->currentIndex();
    if (index.isValid())
    {
        QString text = NamesFilterProxyModel::globalInstance().data(index).toString();
        if (!text.isEmpty())
        {
            QModelIndex newIndex;
            if (index.column() == NamesModel::OldName)
                newIndex = RenameRulesModel::globalInstance().prependCreatedRule(text);
            else
                newIndex = RenameRulesModel::globalInstance().appendCreatedRule(text);
            ui.tableView_replacements->selectionModel()->select(newIndex,
                                                                QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
            ui.tableView_replacements->scrollTo(newIndex);
            ui.tableView_replacements->setFocus(Qt::OtherFocusReason);
        }
    }
}

void H2rename::on_pushButton_Load_clicked()
{
    readDirs();
}
