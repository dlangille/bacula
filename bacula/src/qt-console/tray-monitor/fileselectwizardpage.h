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
#ifndef FILESELECTWIZARDPAGE_H
#define FILESELECTWIZARDPAGE_H

#include <QWizardPage>

class QStandardItemModel;
class QModelIndex;

namespace Ui {
class FileSelectWizardPage;
}

class FileSelectWizardPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(qulonglong currentFile READ currentFile NOTIFY currentFileChanged)

public:
    explicit FileSelectWizardPage(QWidget *parent = 0);
    ~FileSelectWizardPage();

    void setModels(QStandardItemModel *src_model, QStandardItemModel *dest_model);

    qulonglong currentFile() const;

    bool isComplete() const;

signals:
    void currentFileChanged();

protected slots:
    void changeCurrentFolder(const QModelIndex& current);

private:
    Ui::FileSelectWizardPage *ui;
    qulonglong                currentPathId;
};

#endif // FILESELECTWIZARDPAGE_H
