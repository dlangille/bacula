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
#ifndef RESTOREWIZARD_H
#define RESTOREWIZARD_H

#include <QWizard>
#include <QModelIndex>
namespace Ui {
class RestoreWizard;
}

class task;
class RESMON;
class QStandardItemModel;

class RestoreWizard : public QWizard
{
    Q_OBJECT

public:
    enum {
        RW_CLIENT_PAGE = 0,
        RW_JOB_PAGE = 1,
        RW_FILE_PAGE = 2,
        RW_ADVANCEDOPTIONS_PAGE = 3
    };

    explicit RestoreWizard(RESMON *r, QWidget *parent = 0);
    ~RestoreWizard();

public slots:
    void pageChanged(int index);

protected slots:
    void fillJobPage();
    void fillFilePage();
    void fillOptionsPage();

private:
    RESMON            *res;
    Ui::RestoreWizard *ui;

    QStandardItemModel *jobs_model;
    QStandardItemModel *src_files_model;
    QStandardItemModel *dest_files_model;
};

#endif // RESTOREWIZARD_H
