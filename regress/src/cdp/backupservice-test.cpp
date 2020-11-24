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

#include "backupservice.h"
#include "journal.h"
#include "btestutils.h"

class BackupServiceTest
{
    char *_tmpDirPath;
    char *_jPath;
    char *_spoolPath;
    Journal *_journal;
    BackupService *_bservice;

public:
    BackupServiceTest() {
        setupTitle("BackupServiceTest");
       
        _tmpDirPath = BTU::getTmpPath(); 
        _spoolPath = BTU::concat(_tmpDirPath, "/cdp-spool-dir");
        _jPath = BTU::concat(_tmpDirPath, "/cdp-client.journal");
        
        BTU::rmDir(_tmpDirPath);
        BTU::mkDir(_tmpDirPath);
        BTU::mkpath(_tmpDirPath, "cdp-spool-dir");

        _journal = new Journal();
        _journal->setJournalPath(_jPath, _spoolPath);
        _bservice = new BackupService();

        logParam("Tmp Folder", _tmpDirPath);
        logParam("Journal File", _jPath);
        logParam("Spool Directory", _spoolPath);

        char *file1 = BTU::concat(_tmpDirPath, "/test_f1.txt");
        BTU::mkfile(file1);
    }

    ~BackupServiceTest() {
//        BTU::rmDir(_tmpDirPath);
    }

private:

#ifndef HAVE_WIN32
    char *verifyFileWasCopied(const char *srcfpath, const char *srcfname, const char *destDirPath) {
        struct dirent *de;
        char *destFile = NULL;
        DIR *destDir = opendir(destDirPath);

        if (destDir == NULL) {
            printf("ERROR: Destination directory not found: %s", destDirPath);
            exit(-1);
        }

        while ((de = readdir(destDir)) != NULL) {
            if (strstr(de->d_name, srcfname) != 0) {
                destFile = bstrdup(de->d_name);
                break;
            }
        }

        if (destFile == NULL) {
            printf("ERROR: Destination file not found. Spool File not created for %s", srcfpath);
            exit(-1);
        }

        destFile = BTU::concat3(destDirPath, "/", destFile);
        char *srcContents = BTU::readFile(srcfpath);
        char *dstContents = BTU::readFile(destFile);
        BTU::verifyStrings("File Copy in the Spool Dir", srcContents, dstContents);
        closedir(destDir);
        return destFile;
    }

#endif

    void verifyFileRecord(const char *fpath, const char *spoolfpath) {
        _journal->beginTransaction("r");
        FileRecord *rec = _journal->readFileRecord();

        if(rec == NULL) {
            printf("FileRecord belonging to file %s was not found!", fpath);
            exit(-1);
        }

        BTU::verifyStrings("fname", fpath, rec->name);
        BTU::verifyStrings("sname", spoolfpath, rec->sname);
        BTU::verifyLength("mtime", rec->mtime, 10);
        BTU::verifyNotNull("fattrs", rec->fattrs);
        _journal->endTransaction();
    }

public:

    void bservice_should_update_spool_dir() {
         title("Service should update the Spool Directory"
              " inside the Journal's Settings Record");

        _bservice->start(_spoolPath, _journal);
        SettingsRecord *rec = _journal->readSettings();
        BTU::verifyNotNull("Spool Dir", rec->getSpoolDir());
        BTU::verifyStrings("Spool Dir", _spoolPath, rec->getSpoolDir());

        delete _bservice;
        _bservice = new BackupService();
    }

    void bservice_should_save_file_and_record() {
        title("Service should save changed file in the Spool Directory"
              " and save the file's metadata in the Journal");

        char *fname = (char *) "test_f1.txt";
        char *fpath = BTU::concat3(_tmpDirPath, "/", fname);
        
        _bservice->start(_spoolPath, _journal);
        _bservice->onChange(fpath);

#ifndef HAVE_WIN32        
        char *spoolFilePath = verifyFileWasCopied(fpath, fname, _spoolPath);
        verifyFileRecord(fpath, spoolFilePath);
#endif
    }
   
    void bservice_should_stop_watching_folder() {
        title("Service should stop watching folder");
        
        FolderRecord rec;
        rec.path = BTU::concat3(_tmpDirPath, "/", "folder1");
        BTU::mkDir(rec.path);
        _journal->writeFolderRecord(rec); 

        rec.path = BTU::concat3(_tmpDirPath, "/", "folder2");
        BTU::mkDir(rec.path);
        _journal->writeFolderRecord(rec);

        rec.path = BTU::concat3(_tmpDirPath, "/", "folder3");
        BTU::mkDir(rec.path);
        _journal->writeFolderRecord(rec);

        _bservice->start(_spoolPath, _journal);
        _bservice->stopWatching("folder2");
    }

};

int main(int argc, char *argv[])
{
    lmgr_init_thread();
    BackupServiceTest test;
    test.bservice_should_update_spool_dir();
    printf("OK\n");
    test.bservice_should_save_file_and_record();
    printf("OK\n");
    test.bservice_should_stop_watching_folder();
    printf("OK\n");
    lmgr_cleanup_thread();
}
