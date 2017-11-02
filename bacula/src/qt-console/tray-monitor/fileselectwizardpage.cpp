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
#include "fileselectwizardpage.h"
#include "ui_fileselectwizardpage.h"
#include "task.h"
#include "filesmodel.h"
#include <QStandardItemModel>

FileSelectWizardPage::FileSelectWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::FileSelectWizardPage),
    currentPathId(0)
{
    ui->setupUi(this);
    registerField("currentFile", this, "currentFile", "currentFileChanged");

    connect(ui->sourceListView, SIGNAL(activated(const QModelIndex&)), this, SLOT(changeCurrentFolder(const QModelIndex&)));
    ui->sourceListView->setLayoutMode(QListView::Batched);
    ui->sourceListView->setBatchSize(BATCH_SIZE);

    /* FIXME : need context menu + shortcuts for del */
    FileDestModel *destModel = new FileDestModel();
    connect(destModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(completeChanged()));
    connect(destModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(completeChanged()));
    ui->destListView->setModel(destModel);


}

FileSelectWizardPage::~FileSelectWizardPage()
{
    delete ui;
}

void FileSelectWizardPage::setModels(QStandardItemModel *src_model, QStandardItemModel *dest_model)
{
    ui->sourceListView->setModel(src_model);
    src_model->clear();
    currentPathId = 0;

    ui->destListView->setModel(dest_model);
    connect(dest_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(completeChanged()));
    connect(dest_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(completeChanged()));

}

qulonglong FileSelectWizardPage::currentFile() const
{
    return currentPathId;
}

void FileSelectWizardPage::changeCurrentFolder(const QModelIndex& current)
{
    if (current.isValid() && ui->sourceListView->model()) {
        QStandardItem *item = ((QStandardItemModel *)ui->sourceListView->model())->itemFromIndex(current);
        if (item && item->data(TypeRole) == TYPEROLE_DIRECTORY) {
            currentPathId = item->data(PathIdRole).toULongLong();
            emit currentFileChanged();
        }
    }
}

bool FileSelectWizardPage::isComplete() const
{
    return ui->destListView->model() && ui->destListView->model()->rowCount() != 0;
}
