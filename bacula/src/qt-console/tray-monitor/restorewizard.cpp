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
#include "restorewizard.h"
#include "ui_restorewizard.h"
#include "filesmodel.h"
#include "task.h"
#include <QStandardItemModel>

RestoreWizard::RestoreWizard(RESMON *r, QWidget *parent) :
    QWizard(parent),
    res(r),
    ui(new Ui::RestoreWizard),
    jobs_model(new QStandardItemModel),
    src_files_model(new FileSourceModel),
    dest_files_model(new FileDestModel)
{
    ui->setupUi(this);

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));

    ui->RestWizBackupSelectPage->setModel(jobs_model);
    ui->RestWiFileSelectionPage->setModels(src_files_model, dest_files_model);
    connect(ui->RestWiFileSelectionPage, SIGNAL(currentFileChanged()), this, SLOT(fillFilePage()));
    ui->RestWizAdvancedOptionsPage->setModel(dest_files_model);
    ui->RestWizAdvancedOptionsPage->setResmon(res);

}

void RestoreWizard::pageChanged(int index)
{
    switch(index) {
    case RW_CLIENT_PAGE:
        ui->RestWizClientPage->setClients(res->clients);
        break;
    case RW_JOB_PAGE:
        fillJobPage();
        break;
    case RW_FILE_PAGE:
        fillFilePage();
        break;
    case RW_ADVANCEDOPTIONS_PAGE:
        fillOptionsPage();
        break;
    default:
        qDebug( "pageChanged default: %d", index );
        break;
    }
}
RestoreWizard::~RestoreWizard()
{
    delete ui;
}

void RestoreWizard::fillJobPage()
{
    task *t = new task();
    connect(t, SIGNAL(done(task*)), ui->RestWizBackupSelectPage, SLOT(optimizeSize()), Qt::QueuedConnection);
    connect(t, SIGNAL(done(task*)), t, SLOT(deleteLater()));

    t->init(res, TASK_LIST_CLIENT_JOBS);

    int idx = field("currentClient").toInt();
    char *p = (char*) res->clients->get(idx);
    POOL_MEM info;
    pm_strcpy(info, p);
    t->arg = info.c_str();
    t->model = jobs_model;
    res->wrk->queue(t);
}

void RestoreWizard::fillFilePage()
{
    task *t = new task();
    connect(t, SIGNAL(done(task*)), t, SLOT(deleteLater()));

    t->init(res, TASK_LIST_JOB_FILES);

    const char *p = field("currentJob").toString().toUtf8();
    POOL_MEM info;
    pm_strcpy(info, p);
    t->arg = info.c_str();

    t->pathId = field("currentFile").toULongLong();
    t->model = src_files_model;

    res->wrk->queue(t);
}

void RestoreWizard::fillOptionsPage()
{
    ui->RestWizAdvancedOptionsPage->setClients(res->clients);

    task *t = new task();
    connect(t, SIGNAL(done(task *)), ui->RestWizAdvancedOptionsPage, SLOT(fill_defaults(task *)), Qt::QueuedConnection);
    connect(t, SIGNAL(done(task*)), t, SLOT(deleteLater()));

    const char *p = "RestoreFile";/*field("currentJob").toString().toStdString().c_str();*/
    POOL_MEM info;
    pm_strcpy(info, p);
    res->mutex->lock();
    bfree_and_null(res->defaults.job);
    res->defaults.job = bstrdup(info.c_str());
    res->mutex->unlock();

    t->init(res, TASK_DEFAULTS);
    res->wrk->queue(t);
}
