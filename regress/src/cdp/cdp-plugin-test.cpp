/*
   Bacula® - The Network Backup Solution

   Copyright (C) 2004-2019 Bacula Systems SA
   All rights reserved.

   The main author of Bacula is Kern Sibbald, with contributions from many
   others, a complete list can be found in the file AUTHORS.

   Licensees holding a valid Bacula Systems SA license may use this file
   and others of this release in accordance with the proprietary license
   agreement provided in the LICENSE file.  Redistribution of any part of
   this release is not permitted.

   Bacula® is a registered trademark of Kern Sibbald.
 */

#include "fd-plugin-driver.h"
#include "journal.h"
#include "cdp-fd.c"

class CdpPluginTest
{
   char *_tmpDirPath;
   char *_pluginPath;
   char *_jobName;
   char *_workingPath;
   char *_spoolPath;
   char *_jPath;
   Journal _journal;
   PluginTestDriver _d;

public:

   void setPaths() {
#ifndef HAVE_WIN32
      _pluginPath = (char *) "./cdp-fd.so";
#else
      _pluginPath = (char *) "./cdp-fd.dll";
#endif

      _tmpDirPath = BTU::getTmpPath(); 
      _workingPath = BTU::concat(_tmpDirPath, "/working");
      _spoolPath = BTU::concat(_tmpDirPath, "/spool-dir");
      _jPath = BTU::concat3(_tmpDirPath, "/", JOURNAL_CLI_FNAME);
      _jobName = (char *) "my_test_job";
   }

   void makeTestFolders() {
      BTU::rmDir(_tmpDirPath);
      BTU::mkDir(_tmpDirPath);
      BTU::mkDir(_workingPath);
      BTU::mkDir(_spoolPath);
   }

   void logParams() {
      logParam("Plugin Path  ", _pluginPath);
      logParam("Test Dir     ", _tmpDirPath);
      logParam("Journal File ", _jPath);
      logParam("Working Dir  ", _workingPath);
      logParam("Spool Dir    ", _spoolPath);
      logParam("Fake Job Name", _jobName);
   }

   CdpPluginTest() {
      setupTitle("CdpPluginTest");

      OSDependentInit();
      this->setPaths();
      this->makeTestFolders();
      this->logParams();
      
      _journal.setJournalPath(_jPath, _spoolPath);
      _d.setWorkingDir(_workingPath);
      debug_level = 60;
   }

   ~CdpPluginTest() {
//      BTU::rmDir(_tmpDirPath);
   }

private:

   void resetJournal() {
      BTU::rmFile(_jPath);
      _journal.setJournalPath(_jPath, _spoolPath);
   }
   
   void makeFolderRecord(const char *fpath) {
      FolderRecord rec;
      rec.path = bstrdup(fpath);
      _journal.writeFolderRecord(rec);
   }

   FileRecord makeTestFile(const char *fname, const char *fcontents) {
      FileRecord rec;
      rec.name = BTU::concat3(_tmpDirPath, "/", fname);
      BTU::mkfile(rec.name, fcontents);
      rec.encode_attrs();

      POOLMEM *tmp_sname = get_pool_memory(PM_FNAME);
      Mmsg(tmp_sname, "%s/%d_%s", _spoolPath, rec.mtime, fname);
      rec.sname = bstrdup(tmp_sname);
      free_and_null_pool_memory(tmp_sname);
      BTU::mkfile(rec.sname, fcontents);

      _journal.writeFileRecord(rec);
      return rec;
   }

   void verifySavePacket(FileRecord &rec, save_pkt &sp) {
      POOLMEM *expected_fname = get_pool_memory(PM_FNAME);
      rec.getBaculaName(expected_fname);
      BTU::verifyStrings("SavePacket.sname", expected_fname, sp.fname);
      BTU::verifyInt64("SavePacket.mtime", rec.mtime, (int64_t) sp.statp.st_mtime);
      free_and_null_pool_memory(expected_fname);
   }  

   void sendBackupEvents() {
      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      POOLMEM *command = get_pool_memory(PM_FNAME);
      Mmsg(command, "cdp: userHome=%s", _tmpDirPath);
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) command);
      BTU::verifyBRC("Backup userHome param parsing", bRC_OK, rc);
      free_and_null_pool_memory(command);
   }

   void simulateBackup(FileRecord &rec, const char *fdata) {
      struct save_pkt sp;
      bRC rc = _d.startBackupFile(&sp);
      BTU::verifyBRC("Start Backup RC", bRC_OK, rc);
      verifySavePacket(rec, sp);
     
      _d.openFileIO(O_RDONLY);
      _d.readFileIO(fdata, strlen(fdata));
      _d.closeFileIO();
      _d.endBackupFile();
   }

   void simulateRestore(const char *fname, const char *fc) {
      struct restore_pkt rp;
      rp.ofname = BTU::concat3(_tmpDirPath, "/", fname);

      bRC rc = _d.startRestoreFile();
      BTU::verifyBRC("Start Restore RC", bRC_Core, rc);

      rc = _d.createFile(&rp);
      BTU::verifyBRC("Create File RC", bRC_OK, rc);
      BTU::verifyInt("Create Status", CF_CORE, rp.create_status);

      _d.openFileIO(O_WRONLY);
      _d.writeFileIO(fc);
      _d.closeFileIO();
      _d.endRestoreFile();

      char *fc2 = BTU::readFile(rp.ofname);
      BTU::verifyStrings("Restored file contents", fc, fc2);
   }

public:
   void plugin_should_handle_check_file() {
      title("Plugin should have checkFile() function implemented");

      subtitle("Plugin is NOT USED by Bacula - should always return bRC_OK");
      _d.startTest(_pluginPath);

      bRC rc = _d.checkFile("/tmp/foo/test_file.txt");
      BTU::verifyBRC("Check File RC", bRC_OK, rc);

      rc = _d.checkFile("/tmp/foo/test_file2.txt");
      BTU::verifyBRC("Check File RC", bRC_OK, rc);

      _d.endTest();


      subtitle("Plugin is USED by Bacula - should always return bRC_Seen");
      _d.startTest(_pluginPath);

      this->sendBackupEvents();

      rc = _d.checkFile("/tmp/foo/test_file.txt");
      BTU::verifyBRC("Check File RC", bRC_Seen, rc);

      rc = _d.checkFile("/tmp/foo/test_file2.txt");
      BTU::verifyBRC("Check File RC", bRC_Seen, rc);

      _d.endTest();
   }

   void plugin_should_handle_backup_cycle() {
      title("Plugin should handle Backup Cycle");
      
      makeFolderRecord(_tmpDirPath);

      const char *fc1 = "akfkaklfass";
      FileRecord r1 = makeTestFile("test_file1.txt", fc1);

      const char *fc2 = "kajeçkfjafkçsjfçlasflsaç";
      FileRecord r2 = makeTestFile("my_cat.png", fc2);

      const char *fc3 = "jaeaij";
      FileRecord r3 = makeTestFile("funny_meme.gif", fc3);
      char *ojc = BTU::readFile(_jPath);

      _d.startTest(_pluginPath);

      this->sendBackupEvents();

      POOLMEM *migratedJournal = get_pool_memory(PM_FNAME);
      Mmsg(migratedJournal, "%s/%s_0.journal", _workingPath, _jobName);
      char *mjc = BTU::readFile(migratedJournal);
      BTU::verifyNotNull("Migrated Journal", mjc);
      BTU::verifyStrings("Migrated Journal", ojc, mjc);

      this->simulateBackup(r1, fc1);
      this->simulateBackup(r2, fc2);
      this->simulateBackup(r3, fc3);

      _d.tryBackupStop();
      _d.endTest();

      BTU::verifyFileDoesntExist(migratedJournal);
   }

   void plugin_should_handle_cancel_backup_event() {
      POOLMEM *newJournal = get_pool_memory(PM_FNAME);
      Mmsg(newJournal, "%s/%s_0.journal", _workingPath, _jobName);

      title("Plugin should handle cancel Backup event");

      const char *fc1 = "akfkaklfass";
      FileRecord r1 = makeTestFile("test_file1.txt", fc1);

      const char *fc2 = "kajeçkfjafkçsjfçlasflsaç";
      FileRecord r2 = makeTestFile("my_cat.png", fc2);

      const char *fc3 = "jaeaij";
      FileRecord r3 = makeTestFile("funny_meme.gif", fc3);

      _d.startTest(_pluginPath);

      this->sendBackupEvents();
      this->simulateBackup(r1, fc1);
      this->simulateBackup(r2, fc2);

      bEvent cancelEvent = { bEventCancelCommand };
      _d.sendPluginEvent(cancelEvent, NULL);

      _d.endTest();

      BTU::verifyFileExists(newJournal);
      BTU::rmDir(_workingPath);
      BTU::mkpath(_tmpDirPath, "working");
   }

   void plugin_should_handle_restore_cycle() {
      title("Plugin should handle Restore Cycle");

      _d.startTest(_pluginPath);

      this->simulateRestore("rfile_t1.txt", "kjfsakjfsalk");
      this->simulateRestore("rfile_t2.txt", "jksakjfsakjfsalkyas7");
      this->simulateRestore("rfile_t3.txt", "kjfsak");

      _d.endTest();
   }

   void plugin_should_handle_userhome_param() {
      title("Plugin should handle \'userhome\' parameter");

      _d.startTest(_pluginPath);
      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) "cdp: userhome=/root");
      CdpContext *pCtx = (CdpContext *) _d.ctx.pContext;
      alist *userHomes = &(pCtx->userHomes);
      alist *journals = &(pCtx->journals);
      BTU::verifyBRC("User parsing", bRC_OK, rc);
      BTU::verifyBiggerThan("User Homes", userHomes->size(), 0);
      BTU::verifyBiggerThan("Journals", journals->size(), 0);
      BTU::verifyInt64("Sizes should be the same", userHomes->size(), journals->size());
      BTU::verifyStrings("Expected User Home", "/root", (char *) (*userHomes)[0]);

      _d.endTest();
   }

   void plugin_should_handle_invalid_userhome_path() {
      title("Plugin should handle invalid \'userHome\' path");

      _d.startTest(_pluginPath);

      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) "cdp: userHome=/non/existent/path");
      BTU::verifyBRC("userHome parsing", bRC_Error, rc);

      _d.endTest();
   }

   void plugin_should_handle_user_param() {
      title("Plugin should handle \'user\' parameter");

      _d.startTest(_pluginPath);
      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) "cdp: user=root");
      CdpContext *pCtx = (CdpContext *) _d.ctx.pContext;
      alist *userHomes = &(pCtx->userHomes);
      alist *journals = &(pCtx->journals);
      BTU::verifyBRC("User parsing", bRC_OK, rc);
      BTU::verifyBiggerThan("User Homes", userHomes->size(), 0);
      BTU::verifyBiggerThan("Journals", journals->size(), 0);
      BTU::verifyInt64("Sizes should be the same", userHomes->size(), journals->size());
      BTU::verifyStrings("Expected User Home", "/root", (char *) (*userHomes)[0]);

      _d.endTest();
   }

   void plugin_should_handle_invalid_user() {
      title("Plugin should handle invalid \'user\' parameter");

      _d.startTest(_pluginPath);
      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) "cdp: user=this-user-does-not-exit");
      BTU::verifyBRC("User parsing", bRC_Error, rc);

      _d.endTest();
   }

   void plugin_should_handle_group_param() {
      title("Plugin should handle \'group\' parameter");

      _d.startTest(_pluginPath);
      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) "cdp: group=sudo");
      CdpContext *pCtx = (CdpContext *) _d.ctx.pContext;
      alist *userHomes = &(pCtx->userHomes);
      alist *journals = &(pCtx->journals);
      BTU::verifyBRC("Group parsing", bRC_OK, rc);
      BTU::verifyBiggerThan("User Homes", userHomes->size(), 0);
      BTU::verifyBiggerThan("Journals", journals->size(), 0);
      BTU::verifyInt64("Sizes should be the same", userHomes->size(), journals->size());
      _d.endTest();
   }

   void plugin_should_handle_invalid_group() {
      title("Plugin should handle invalid \'group\' parameter");

      _d.startTest(_pluginPath);
      _d.setJobName(bstrdup(_jobName));
      bEvent jobStart = { bEventJobStart };
      _d.sendPluginEvent(jobStart, NULL);

      bEvent pluginComm = { bEventPluginCommand };
      bRC rc = _d.sendPluginEvent(pluginComm, (void *) "cdp: group=non-exisent");
      CdpContext *pCtx = (CdpContext *) _d.ctx.pContext;
      BTU::verifyBRC("Group parsing", bRC_Error, rc);

      _d.endTest();
   }

#ifdef HAVE_WIN32
   void plugin_should_add_drivers_to_vss_snapshot() {
      title("Plugin should add drivers to VSS Snapshot");
      
      this->resetJournal();
      this->makeFolderRecord("A:/FolderA1");
      this->makeFolderRecord("A:/FolderA2/xyz/jkl");
      this->makeFolderRecord("D:/FolderD");
      this->makeFolderRecord("Z:/FolderZ1");
      this->makeFolderRecord("Z:/FolderZ2/jkl");
      this->makeFolderRecord("Z:/FolderZ3/abcd/ghi");
            
      _d.startTest(_pluginPath);

      this->sendBackupEvents();

      char drives[50];
      bEvent pluginComm = { bEventVssPrepareSnapshot };
      _d.sendPluginEvent(pluginComm, (void *) drives);      
      BTU::verifyStrings("VSS Drives List", "ADZ", drives);

      _d.endTest();
   }
#endif
};

int main(int argc, char *argv[])
{
   CdpPluginTest test;
   test.plugin_should_handle_check_file();
   printf("OK\n");
   test.plugin_should_handle_backup_cycle();
   printf("OK\n");
   test.plugin_should_handle_cancel_backup_event();
   printf("OK\n");
   test.plugin_should_handle_restore_cycle();
   printf("OK\n");
#ifdef HAVE_WIN32
   test.plugin_should_add_drivers_to_vss_snapshot();
   printf("OK\n");
#else
   test.plugin_should_handle_userhome_param();
   printf("OK\n");
   test.plugin_should_handle_invalid_userhome_path();
   printf("OK\n");
   test.plugin_should_handle_user_param();
   printf("OK\n");
   test.plugin_should_handle_invalid_user();
   printf("OK\n");
   test.plugin_should_handle_group_param();
   printf("OK\n");
   test.plugin_should_handle_invalid_group();
   printf("OK\n");
#endif
}

