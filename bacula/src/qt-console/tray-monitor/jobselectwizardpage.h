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
#ifndef JOBSELECTWIZARDPAGE_H
#define JOBSELECTWIZARDPAGE_H

#include <QWizardPage>

class QStandardItemModel;
class QItemSelection;

namespace Ui {
class JobSelectWizardPage;
}

class JobSelectWizardPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(qlonglong currentJob READ currentJob NOTIFY currentJobChanged)

public:
    explicit JobSelectWizardPage(QWidget *parent = 0);
    ~JobSelectWizardPage();

    void setModel(QStandardItemModel *model);

    u_int64_t currentJob() const;

    bool isComplete() const;

signals:
    void currentJobChanged();

public slots:
    void optimizeSize();

private:
    Ui::JobSelectWizardPage *ui;
    qlonglong               m_jobId;
};

#endif // JOBSELECTWIZARDPAGE_H
