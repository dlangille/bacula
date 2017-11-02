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
#include "clientselectwizardpage.h"
#include "ui_clientselectwizardpage.h"
#include "common.h"
ClientSelectWizardPage::ClientSelectWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ClientSelectWizardPage)
{
    ui->setupUi(this);

    registerField("currentClient*", ui->backupClientComboBox);
}

ClientSelectWizardPage::~ClientSelectWizardPage()
{
    delete ui;
}

void ClientSelectWizardPage::setClients(alist *lst)
{
    if (lst && lst->size() > 0) {
       ui->backupClientComboBox->clear();
       QStringList list;
       char *str;
       foreach_alist(str, lst) {
          list << QString(str);
       }
       ui->backupClientComboBox->addItems(list);
       ui->backupClientComboBox->setEnabled(true);
    } else {
       ui->backupClientComboBox->setEnabled(false);
    }
}
