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
#include "restoreoptionswizardpage.h"
#include "ui_restoreoptionswizardpage.h"
#include "common.h"
#include "filesmodel.h"
#include "task.h"

RestoreOptionsWizardPage::RestoreOptionsWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::RestoreOptionsWizardPage),
    dest_model(0),
    m_res(0)

{
    ui->setupUi(this);

    registerField("restoreClient*", ui->restoreClientComboxBox);
    registerField("restoreWhere", ui->whereLineEdit);
    registerField("restoreReplace*", ui->replaceComboBox);
    registerField("restoreComment", ui->commentLineEdit);
}

RestoreOptionsWizardPage::~RestoreOptionsWizardPage()
{
    delete ui;
}


void RestoreOptionsWizardPage::fill_defaults(task *t)
{
    if (ui->restoreClientComboxBox->isEnabled()) {
        ui->restoreClientComboxBox->setCurrentIndex(ui->restoreClientComboxBox->findData(t->res->defaults.client));
    }
    if (ui->whereLineEdit->isEnabled()) {
        ui->whereLineEdit->setText(t->res->defaults.where);
    }
}

void RestoreOptionsWizardPage::setClients(alist *lst)
{
    if (lst && lst->size() > 0) {
       ui->restoreClientComboxBox->clear();
       QStringList list;
       char *str;
       foreach_alist(str, lst) {
          list << QString(str);
       }
       ui->restoreClientComboxBox->addItems(list);
       ui->restoreClientComboxBox->setEnabled(true);
    } else {
       ui->restoreClientComboxBox->setEnabled(false);
    }

    /* by default, select the same client as in clientselectwizardPage */
    /* this is assuming lists are identical and in the same order. It should be the case */
    ui->restoreClientComboxBox->setCurrentIndex(field("currentClient").toInt());
}

void RestoreOptionsWizardPage::setModel(QStandardItemModel *model)
{
    dest_model = model;
}

void RestoreOptionsWizardPage::setResmon(RESMON *res)
{
    m_res = res;
}
extern int decode_stat(char *buf, struct stat *statp, int stat_size, int32_t *LinkFI);

bool RestoreOptionsWizardPage::validatePage()
{
    task *t = new task();
    connect(t, SIGNAL(done(task*)), t, SLOT(deleteLater()));

    t->init(m_res, TASK_RESTORE);

    t->arg = (char*) m_res->clients->get(field("restoreClient").toInt());

    if (!field("restoreWhere").isNull()) {
        t->arg2 = field("restoreWhere").toString().toUtf8();
    } else {
        t->arg2 = NULL;
    }

    if (!field("restoreComment").isNull()) {
        t->arg3 = field("restoreComment").toString().toUtf8();
    } else {
        t->arg3 = NULL;
    }

    t->model = dest_model;

    m_res->wrk->queue(t);
    return true;
}
