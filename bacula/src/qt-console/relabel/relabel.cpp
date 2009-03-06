/*
   Bacula® - The Network Backup Solution

   Copyright (C) 2007-2008 Free Software Foundation Europe e.V.

   The main author of Bacula is Kern Sibbald, with contributions from
   many others, a complete list can be found in the file AUTHORS.
   This program is Free Software; you can redistribute it and/or
   modify it under the terms of version two of the GNU General Public
   License as published by the Free Software Foundation and included
   in the file LICENSE.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Bacula® is a registered trademark of Kern Sibbald.
   The licensor of Bacula is the Free Software Foundation Europe
   (FSFE), Fiduciary Program, Sumatrastrasse 25, 8006 Zürich,
   Switzerland, email:ftf@fsfeurope.org.
*/
 
/*
 *  Label Dialog class
 *
 *   Kern Sibbald, February MMVII
 *
 */ 

#include "bat.h"
#include "relabel.h"
#include <QMessageBox>

/*
 * An overload of the constructor to have a default storage show in the
 * combobox on start.  Used from context sensitive in storage class.
 */
relabelDialog::relabelDialog(Console *console, QString &fromVolume)
{
   m_console = console;
   m_fromVolume = fromVolume;
   m_console->notify(false);
   setupUi(this);
   storageCombo->addItems(console->storage_list);
   poolCombo->addItems(console->pool_list);
   volumeName->setText(fromVolume);
   QString fromText(tr("From Volume : "));
   fromText += fromVolume;
   fromLabel->setText(fromText);
   QStringList defFields;
   if (getDefs(defFields) >= 1) {
      poolCombo->setCurrentIndex(poolCombo->findText(defFields[1], Qt::MatchExactly));
      storageCombo->setCurrentIndex(storageCombo->findText(defFields[0], Qt::MatchExactly));
   }
   this->show();
}

/*
 * Use an sql statment to get some defaults
 */
int relabelDialog::getDefs(QStringList &fieldlist)
{
   QString job, client, fileset;
   QString query("");
   query = "SELECT MediaType AS MediaType, Pool.Name AS PoolName"
   " FROM Media"
   " LEFT OUTER JOIN Pool ON Media.PoolId = Pool.PoolId"
   " WHERE VolumeName = \'" + m_fromVolume  + "\'";
   if (mainWin->m_sqlDebug) { Pmsg1(000, "query = %s\n", query.toUtf8().data()); }
   QStringList results;
   if (m_console->sql_cmd(query, results)) {
      QString field;
      /* Iterate through the lines of results, there should only be one. */
      foreach (QString resultline, results) {
         fieldlist = resultline.split("\t");
      } /* foreach resultline */
   } /* if results from query */
   return results.count();
}

void relabelDialog::accept()
{
   QString scmd;
   if (volumeName->text().toUtf8().data()[0] == 0) {
      QMessageBox::warning(this, tr("No Volume name"), tr("No Volume name given"),
                           QMessageBox::Ok, QMessageBox::Ok);
      return;
   }
   if (m_fromVolume == volumeName->text().toUtf8()) {
      QMessageBox::warning(this, tr("New name must be different"), 
                           tr("New name must be different"),
                           QMessageBox::Ok, QMessageBox::Ok);
      return;
   }

   this->hide();
   scmd = QString("relabel storage=\"%1\" oldvolume=\"%2\" volume=\"%3\" pool=\"%4\" slot=%5")
                  .arg(storageCombo->currentText())
                  .arg(m_fromVolume)
                  .arg(volumeName->text())
                  .arg(poolCombo->currentText())
                  .arg(slotSpin->value());
   if (mainWin->m_commandDebug) {
      Pmsg1(000, "sending command : %s\n",scmd.toUtf8().data());
   }
   m_console->write_dir(scmd.toUtf8().data());
   m_console->displayToPrompt();
   m_console->notify(true);
   delete this;
   mainWin->resetFocus();
}

void relabelDialog::reject()
{
   this->hide();
   m_console->notify(true);
   delete this;
   mainWin->resetFocus();
}
