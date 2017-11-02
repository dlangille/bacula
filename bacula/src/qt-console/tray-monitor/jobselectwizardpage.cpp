/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2017 Kern Sibbald

   The original author of Bacula is Kern Sibbald, with contributions
   from many others, a complete list can be found in the file AUTHORS.

   You may use this file and others of this release according to the
   license defined in the LICENSE file, which includes the Affero General
   Public License, v3.0 ("AGPLv3") and some additional permissions and
   terms pursuant to its AGPLv3 Section 7.

   This notice must be preserved when any source code is
   conveyed and/or propagated.

   Bacula(R) is a registered trademark of Kern Sibbald.
*/
#include "jobselectwizardpage.h"
#include "ui_jobselectwizardpage.h"
#include <QStandardItemModel>

JobSelectWizardPage::JobSelectWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::JobSelectWizardPage)
{
    ui->setupUi(this);
    registerField("currentJob*", this, "currentJob", SIGNAL(currentJobChanged()));
}

JobSelectWizardPage::~JobSelectWizardPage()
{
    delete ui;
}

void JobSelectWizardPage::setModel(QStandardItemModel *model)
{
    ui->BackupTableView->setModel(model);

    connect(ui->BackupTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SIGNAL(completeChanged()));
    connect(ui->BackupTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SIGNAL(currentJobChanged()));
}

bool JobSelectWizardPage::isComplete() const
{
    int s = ui->BackupTableView->selectionModel() ?
                ui->BackupTableView->selectionModel()->selectedRows().size() : 0;

    return (ui->BackupTableView->selectionModel() && s>0);
}

void JobSelectWizardPage::optimizeSize()
{
    ui->BackupTableView->resizeColumnsToContents();
    ui->BackupTableView->horizontalHeader()->setStretchLastSection(true);
    ui->BackupTableView->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
}

u_int64_t JobSelectWizardPage::currentJob() const
{
    if (ui->BackupTableView->selectionModel())
    {
        QModelIndex idx = ui->BackupTableView->selectionModel()->currentIndex();
        /* return the JobId (column 0) */
        QModelIndex idIdx = idx.sibling(idx.row(), 0);
        return idIdx.data().toULongLong();
    }
    return -1;
}
