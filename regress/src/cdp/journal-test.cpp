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

#include "cdp.h"
#include "journal.h"
#include "btestutils.h"
#include "string.h"

#ifndef HAVE_WIN32
#include <sys/file.h>
#endif

/**
  Test thread used to confirm that the Journal class waits for the
  Journal File to be available
 */
void *thread_journal(void *arg) {
   printf("PARENT - thread_journal started....\n");
   Journal *journal = (Journal *) arg;
   printf("PARENT - Calling beginTransaction()\n");
   journal->beginTransaction("r");
   printf("PARENT - beginTransaction() completed\n");
   return NULL;
}


class JournalTest
{

   char *_tmpDirPath;
   char *_jPath;
   Journal *_journal;

   public:
   JournalTest() {
      setupTitle("Journal Tests");

      _tmpDirPath = BTU::getTmpPath(); 
      _jPath = BTU::concat(_tmpDirPath, "/cdp-cli.journal");
      _journal = new Journal();

      logParam("TmpDir", _tmpDirPath);
      logParam("Journal File", _jPath);

      BTU::rmDir(_tmpDirPath);
      BTU::mkDir(_tmpDirPath);
   }

   ~JournalTest() {
      //        BTU::rmDir(_tmpDirPath);
   }

   private:

   void verifyFileContainsRecord(const char *fname, const FileRecord &rec) {
      const int SANITY_CHECK = 10000;
      char *fcontents = BTU::readFile(fname);
      char expected[SANITY_CHECK];
      bsnprintf(expected, SANITY_CHECK,
            "File {" NL
            "name=%s" NL
            "sname=%s" NL
            "mtime=%lld" NL
            "attrs=%s" NL
            "}" NL,
            rec.name,
            rec.sname,
            rec.mtime,
            rec.fattrs);
      if (strstr(fcontents, expected) == NULL) {
         printf("ERROR: FileRecord of file %s not found.\n"
               "FCONTENTS:\n%s\n"
               "EXPECTED:\n%s\n", rec.name, fcontents, expected);
         exit(-1);
      }
   }

   void verifyFolderNotContainsRecord(const char *fname, const FolderRecord &rec) {
      const int SANITY_CHECK = 1000000;
      char *fcontents = BTU::readFile(fname);
      char expected[SANITY_CHECK];
      bsnprintf(expected, SANITY_CHECK,
            "Folder {" NL
            "path=%s" NL
            "}" NL,
            rec.path);
      if (strstr(fcontents, expected) != NULL) {
         printf("ERROR: FolderRecord of folder %s not found.", rec.path);
         exit(-1);
      }
   }

   void verifyFolderContainsRecord(const char *fname, const FolderRecord &rec) {
      const int SANITY_CHECK = 1000000;
      char *fcontents = BTU::readFile(fname);
      char expected[SANITY_CHECK];
      bsnprintf(expected, SANITY_CHECK,
            "Folder {" NL
            "path=%s" NL
            "}" NL,
            rec.path);
      if (strstr(fcontents, expected) == NULL) {
         printf("ERROR: FolderRecord of folder %s not found.", rec.path);
         exit(-1);
      }
   }

   void do_update_settings_test(SettingsRecord &rec) {
      if (!_journal->writeSettings(rec)) {
         printf("ERROR: could not write SettingsRecord");
         exit(-1);
      }
      SettingsRecord *read = _journal->readSettings();
      BTU::verifyNotNull("SettingsRecord", read);
      BTU::verifyInt64("jversion", rec.journalVersion, read->journalVersion);
      BTU::verifyInt64("heartbeat", rec.heartbeat, read->heartbeat);
      BTU::verifyStrings("spooldir", rec.getSpoolDir(), read->getSpoolDir());
      delete read;
      delete read;
   }

   void do_write_record_test(const char* fname) {
      FileRecord rec;
      rec.name = BTU::concat3(_tmpDirPath, "/" ,fname);
      char *tmp = BTU::concat("12345_", fname);
      rec.sname = BTU::concat3(_tmpDirPath, "/" , tmp);
      BTU::mkfile(rec.name);

      if (!rec.encode_attrs()) {
         printf("ERROR: could not encode file attributes: %s", rec.name);
         exit(-1);
      }

      if (!_journal->writeFileRecord(rec)) {
         printf("ERROR: could not write FileRecord: %s", rec.name);
         exit(-1);     
      }
      verifyFileContainsRecord(_jPath, rec);
   }

   void do_write_folder_test(const char* fname) {
      FolderRecord rec;
      rec.path = BTU::concat3(_tmpDirPath, "/" ,fname);
      if (!_journal->writeFolderRecord(rec)) {
         printf("ERROR: could not write FolderRecord: %s", fname);
         exit(-1); 
      }
      verifyFolderContainsRecord(_jPath, rec);
   }

   void do_read_file_test(const char* fname) {
      FileRecord *fr = _journal->readFileRecord();

      if(fname != NULL) {
         char *fpath = BTU::concat3(_tmpDirPath, "/", fname);
         BTU::verifyStrings("File Path", fpath, fr->name);
         free(fpath);
      } else {
         BTU::verifyNull("FileRecord", fr);
      }

      delete fr;
   }

   void do_read_folder_test(const char* folder) {
      FolderRecord *fr = _journal->readFolderRecord();

      if(fr == NULL && folder != NULL) {
         printf("ERROR: expected a non-null FolderRecord");
         exit(-1);
      } else if(folder != NULL) {
         char *fpath = BTU::concat3(_tmpDirPath, "/", folder);
         BTU::verifyStrings("File Path", fpath, fr->path);
         free(fpath);
      } else {
         BTU::verifyNull("FolderRecord", fr);
      }

      delete fr;
   }

   void do_remove_folder_test(const char* folder) {
      FolderRecord rec;
      rec.path = BTU::concat3(_tmpDirPath, "/" , folder);
      if (!_journal->removeFolderRecord(rec.path)) {
         printf("ERROR: could not remove FolderRecord: %s", rec.path);
         exit(-1); 
      }

      verifyFolderNotContainsRecord(_jPath, rec);
   }

   public:

   void journal_filerecord_should_encode_and_decode_file_attributes() {
      title("File Record should encode and decode attributes");

      FileRecord rec;
      struct stat sbuf;

      rec.name = BTU::concat3(_tmpDirPath, "/", "testde.txt");
      BTU::mkfile(rec.name, "çasfkas");

      if (!rec.encode_attrs()) {
         printf("ERROR: could not encode attributes of file %s", rec.name);
         exit(-1);
      }

      rec.decode_attrs(sbuf);
      BTU::verifyInt64("mtime", rec.mtime, (int64_t) sbuf.st_mtime);
   }

   void journal_should_create_jfile_on_set_path() {
      title("setJournalPath() should create a new Journal File"
            " with a default SettingsRecord"
            " if this File does not exist");

      if (!_journal->setJournalPath(_jPath)) {
         printf("ERROR: could not set journal path to %s", _jPath);
         exit(-1);
      }

      SettingsRecord *rec = _journal->readSettings();
      BTU::verifyNotNull("SettingsRecord", rec);
      BTU::verifyInt64("heartbeat", -1, rec->heartbeat);
      BTU::verifyInt64("jversion", JOURNAL_VERSION, rec->journalVersion);
   }

   // void journal_should_update_settings_record() {
   //      title("writeSettings() should update Settings Record");

   //      char *spath = CDP::spoolDir();
   //      logParam("Test Spool Dir", spath);

   //      SettingsRecord r1;
   //      r1.heartbeat = 500000;
   //      r1.journalVersion = 2;
   //      r1.spoolDir = (char *) "/home/foo/bar";
   //      do_update_settings_test(r1);

   //      SettingsRecord r2;
   //      r2.heartbeat = 999999;
   //      r2.journalVersion = 40;
   //      do_update_settings_test(r2);
   // }

   void journal_should_add_new_file_records() {
      title("Journal should add new File Records");

      do_write_record_test("file_test1.txt");
      do_write_record_test("file_test2.pdf");
      do_write_record_test("file_test3.png");
      do_write_record_test("file_test4.odt");
   }

   void journal_should_add_new_folder_records() {
      title("Journal should add new Folder Records");

      do_write_folder_test("folder1");
      do_write_folder_test("folder2");
      do_write_folder_test("folder3");
   }

   void journal_should_add_new_file_records_2() {
      title("Journal should add new File Records (2)");

      do_write_record_test("file_test5.gif");
      do_write_record_test("file_test6.cpp");
   }

   void journal_should_add_new_folder_records_2() {
      title("Journal should add new Folder Records (2)");

      do_write_folder_test("folder4");
      do_write_folder_test("folder5");
   }

   void journal_should_read_all_file_records() {
      title("Journal should read all File Records");

      _journal->beginTransaction("r");
      do_read_file_test("file_test1.txt");
      do_read_file_test("file_test2.pdf");
      do_read_file_test("file_test3.png");
      do_read_file_test("file_test4.odt");
      do_read_file_test("file_test5.gif");
      do_read_file_test("file_test6.cpp");
      do_read_file_test(NULL);
      _journal->endTransaction();
   }

   void journal_should_read_all_folder_records() {
      title("Journal should read all Folder Records");

      _journal->beginTransaction("r");
      do_read_folder_test("folder1");
      do_read_folder_test("folder2");
      do_read_folder_test("folder3");
      do_read_folder_test("folder4");
      do_read_folder_test("folder5");
      do_read_folder_test(NULL);
      _journal->endTransaction();
   }

   void journal_should_remove_folder_records() {
      title("Journal should remove Folder Records");

      char *oldJournal = BTU::concat(_tmpDirPath, "/old.journal");
      BTU::cpFile(_jPath, oldJournal);

      do_remove_folder_test("folder3");
      do_remove_folder_test("folder4");
      do_remove_folder_test("folder1");
      do_remove_folder_test("folder2");

      BTU::rmFile(_jPath);
      BTU::cpFile(oldJournal, _jPath);
   }

   void journal_should_migrate_to_new_path() {
      title("Journal should migrate to New Path");

      char *f1 = BTU::readFile(_jPath);
      char *newPath = BTU::concat(_tmpDirPath, "/migrated.journal");

      if (!_journal->migrateTo(newPath)) {
         printf("ERROR: could not migrate Journal to %s", newPath);
         exit(-1);    
      }

      // Verify that the migrated Journal has the same contents
      // as the old one
      char *f2 = BTU::readFile(newPath);
      BTU::verifyNotNull("Migrated Journal", f2);
      BTU::verifyStrings("Migrated Journal", f1, f2);
      free(f1);
      free(f2);

      // Verify that the old Journal now only have:
      // 1-) SettingsRecord
      // 2-) FolderRecords
      _journal = new Journal();
      _journal->setJournalPath(_jPath);

      SettingsRecord *rec = _journal->readSettings();
      BTU::verifyNotNull("SettingsRecord", rec);
      BTU::verifyInt64("jversion", 1, rec->journalVersion);
      BTU::verifyInt64("heartbeat", -1, rec->heartbeat);

      _journal->beginTransaction("r");
      do_read_folder_test("folder1");
      do_read_folder_test("folder2");
      do_read_folder_test("folder3");
      do_read_folder_test("folder4");
      do_read_folder_test("folder5");
      do_read_folder_test(NULL);
      _journal->endTransaction();

      _journal->beginTransaction("r");
      do_read_file_test(NULL);
      _journal->endTransaction();
   }

   void journal_should_handle_errors_on_reading() {
      title("Journal should handle errors while reading a Record");

      subtitle("extract_val() subroutine should handle invalid key-value pairs");
      char *val;

      // No '=' character
      val = _journal->extract_val("noequalssign\n");
      BTU::verifyNull("extract_val() test", val);

      // No '\n' character
      val = _journal->extract_val("noendline=true");
      BTU::verifyNull("extract_val() test", val);

      // No '=' and '\n' character
      val = _journal->extract_val("no_both");
      BTU::verifyNull("extract_val() test", val);
   }

   void journal_should_wait_for_jfile_to_be_available() {
      title("Journal should wait for Jornal File to be available");

#ifndef HAVE_WIN32
      pid_t pid;
      pid = fork();

      if (pid < 0) {
         printf("ERROR: Could not create Test Process\n");
         exit(-1);
      }

      if (pid == 0) {
         printf("Created Child Process\n");
         FILE *fp = bfopen(_jPath, "r");
         int fd = fileno(fp);
         printf("CHILD  - got lock for Journal File\n");
         flock(fd, LOCK_EX);
         sleep(3);
         printf("CHILD  - released lock for Journal File\n");
         fclose(fp);
         exit(0);
      } else {
         sleep(1);
         pthread_t jthread;     
         pthread_create(&jthread, NULL, thread_journal, (void *) _journal);
         if (_journal->hasTransaction) {
            printf("ERROR: Journal didn't respect the File Lock \n");
            exit(-1);
         }
         pthread_join(jthread, NULL);
         if (!_journal->hasTransaction) {
            printf("ERROR: Journal didn't have a transaction after the Journal File was available \n");
            exit(-1);
         }
      }
#else
      STARTUPINFO si;
      PROCESS_INFORMATION pi;
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);     
      ZeroMemory(&pi, sizeof(pi));

      POOLMEM *_jPathWrapper = get_pool_memory(PM_FNAME);
      Mmsg(_jPathWrapper, "c:\\Program Files\\Bacula\\win_test_process.exe \"%s\"", _jPath);
      const size_t wsize = (strlen(_jPathWrapper) + 1) * 2; 
      wchar_t *wjpath = new wchar_t[wsize];
      mbstowcs(wjpath, (char *) _jPathWrapper, wsize);   

      if(!CreateProcess(NULL,   // No module name (use command line)
               wjpath,         // Command line
               NULL,           // Process handle not inheritable
               NULL,           // Thread handle not inheritable
               FALSE,          // Set handle inheritance to FALSE
               0,              // No creation flags
               NULL,           // Use parent's environment block
               NULL,           // Use parent's starting directory 
               &si,            // Pointer to STARTUPINFO structure
               &pi )           // Pointer to PROCESS_INFORMATION structure
        ) 
      {
         printf( "CreateProcess failed (%lu).\n", GetLastError());
         return;
      }

      sleep(1);
     _journal->beginTransaction("r");
      if (!_journal->hasTransaction) {
         printf("ERROR: Journal didn't have a transaction after the Journal File was available \n");
         exit(-1);
      }

      // Wait until child process exits.
      WaitForSingleObject(pi.hProcess, INFINITE);

      // Close process and thread handles. 
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread); 

#endif
   }

};

int main(int argc, char *argv[])
{
   lmgr_init_thread();
   JournalTest test;
   test.journal_filerecord_should_encode_and_decode_file_attributes();
   printf("OK\n");
   test.journal_should_create_jfile_on_set_path();
   printf("OK\n");
   //TODO fix update settings;
   //test.journal_should_update_settings_record();
   test.journal_should_add_new_file_records();
   printf("OK\n");
   test.journal_should_add_new_folder_records();
   printf("OK\n");
   test.journal_should_add_new_file_records_2();
   printf("OK\n");
   test.journal_should_add_new_folder_records_2();
   printf("OK\n");
   test.journal_should_read_all_file_records();
   printf("OK\n");
   test.journal_should_read_all_folder_records();
   printf("OK\n");
   test.journal_should_remove_folder_records();
   printf("OK\n");
   test.journal_should_migrate_to_new_path();
   printf("OK\n");
   test.journal_should_handle_errors_on_reading();
   printf("OK\n");
   test.journal_should_wait_for_jfile_to_be_available();
   printf("OK\n");
}
