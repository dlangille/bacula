/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2020 Kern Sibbald

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

#include "journal.h"

static int DBGLVL = 90;

bool Journal::beginTransaction(const char *mode)
{
   if (hasTransaction) {
      return true;
   }

   bool hasLock = false;
   int timeout = 1800; 

   for (int time = 0; time < timeout; time++) {
      _fp = bfopen(_jPath, mode);

      if (!_fp) {
         Dmsg0(0, "Tried to start transaction but Journal File was not found.\n");
         return false;
      }

      _fd = fileno(_fp);
      int rc = flock(_fd, LOCK_EX | LOCK_NB);

      //Success. Lock acquired.
      if (rc == 0) {
         hasLock = true;
         break;
      }

      fclose(_fp);
      sleep(1);
   }

   if (hasLock) {
      hasTransaction = true;
      return true;
   } else {
      Dmsg0(0, "Tried to start transaction but could not lock Journal File.\n");
      return false;
   }
}

void Journal::endTransaction()
{
   if (!hasTransaction) {
      return;
   }

   if (_fp != NULL) {
      int rc = flock(_fd, LOCK_UN);

      if (rc != 0) {
         Dmsg0(0, "could not release flock\n");
      }

      fclose(_fp);
      _fp = NULL;
   }

   _fd = -1;
   hasTransaction = false;
}

/**
 * Given a string formatted as 'key=val\n',
 * this function tries to return 'val'
 */
char *Journal::extract_val(const char *key_val)
{
   const int SANITY_CHECK = 10000;
   int max_idx = cstrlen(key_val) - 1;
   char *val = (char *) malloc(SANITY_CHECK * sizeof(char));

   int idx_keyend = 0;

   while(key_val[idx_keyend] != '=') {
      idx_keyend++;

      if(idx_keyend > max_idx) {
         free(val);
         return NULL;
      }
   }

   int i;
   int j = 0;
   for(i = idx_keyend + 1; key_val[i] != '\n' ; i++) {
      val[j] = key_val[i];
      j++;

      if(i > max_idx) {
         free(val);
         return NULL;
      }
   }

   val[j] = '\0';
   return val;
}

bool Journal::setJournalPath(const char *path)
{
   _jPath = bstrdup(path);
   FILE *jfile = bfopen(_jPath, "r");    

   if (!jfile) {
      if (this->beginTransaction("w")) {
         SettingsRecord rec;
         rec.journalVersion = JOURNAL_VERSION;
         this->writeSettings(rec);
      } else {
         Dmsg1(0, "(ERROR) Could not create Journal File: %s\n", path);
         return false;
      }
   } else {
      fclose(jfile);
   }

   return true;
}

bool Journal::setJournalPath(const char *path, const char *spoolDir)
{
   _jPath = bstrdup(path);
   FILE *jfile = fopen(_jPath, "r");

   if (!jfile) {
      if (this->beginTransaction("w")) {
         SettingsRecord rec;
         rec.journalVersion = JOURNAL_VERSION;
         rec.setSpoolDir(spoolDir);
         this->writeSettings(rec);
      } else {
         Dmsg1(0, "(ERROR) Could not create Journal File: %s\n", path);
         return false;
      }
   } else {
      fclose(jfile);
   }

   return true;
}


bool Journal::writeSettings(SettingsRecord &rec)
{
   int rc;
   bool success = true;
   const char *spoolDir;
   char jversion[50];
   char heartbeat[50];

   if(!this->beginTransaction("r+")) {
      Dmsg0(50, "Could not start transaction for writeSettings()\n");
      success = false;
      goto bail_out;
   }

   spoolDir = rec.getSpoolDir();
   
   if (spoolDir == NULL) {
      spoolDir = "<NULL>";
   } 

   edit_int64(rec.heartbeat, heartbeat);
   edit_int64(rec.journalVersion, jversion);
   rc = fprintf(_fp,
         "Settings {\n"
         "spooldir=%s\n"
         "heartbeat=%s\n"
         "jversion=%s\n"
         "}\n",
         spoolDir,
         heartbeat,
         jversion);

   if(rc < 0) {
      success = false;
      Dmsg1(50, "(ERROR) Could not write SettingsRecord. RC=%d\n", rc);
      goto bail_out;
   }

   Dmsg3(DBGLVL, 
         "WROTE RECORD:\n"
         " Settings {\n"
         "  spooldir=%s\n"
         "  heartbeat=%s\n"
         "  jversion=%s\n"
         " }\n",
         spoolDir,
         heartbeat,
         jversion);

bail_out:
   this->endTransaction();
   return success;
}

SettingsRecord *Journal::readSettings()
{
   const int SANITY_CHECK = 10000;
   bool corrupted = false;
   char tmp[SANITY_CHECK];
   char jversion[SANITY_CHECK];
   char heartbeat[SANITY_CHECK];
   char spoolpath[SANITY_CHECK];
   char *jvstr = NULL;
   char *hbstr = NULL;
   SettingsRecord *rec = NULL;

   if(!this->beginTransaction("r+")) {
      Dmsg0(0, "Could not start transaction for readSettings()\n");
      goto bail_out;
   }

   //reads line "Settings {\n"
   if(!bfgets(tmp, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }

   rec = new SettingsRecord();

   //reads Spool Dir
   if(!bfgets(spoolpath, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   rec->setSpoolDir(extract_val(spoolpath));
   if(rec->getSpoolDir() == NULL) {
      corrupted = true;
      goto bail_out;
   }

   //reads Heartbeat
   if(!bfgets(heartbeat, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   hbstr = extract_val(heartbeat);
   if(hbstr == NULL) {
      corrupted = true;
      goto bail_out;
   }
   rec->heartbeat = atoi(hbstr);

   //reads Journal Version
   if(!bfgets(jversion, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   jvstr = extract_val(jversion);
   if(jvstr == NULL) {
      corrupted = true;
      goto bail_out;
   }
   rec->journalVersion = atoi(jvstr);

   //reads line "}\n"
   if(!bfgets(tmp, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }

   Dmsg3(DBGLVL,
         "READ RECORD:\n"
         " Settings {\n"
         "  spooldir=%s\n"
         "  heartbeat=%s\n"
         "  jversion=%s\n"
         " }\n",
         rec->getSpoolDir(),
         hbstr,
         jvstr);

bail_out:
   if(jvstr != NULL) {
      free(jvstr);
   }

   if(hbstr != NULL) {
      free(hbstr);
   }

   if(rec != NULL && rec->getSpoolDir() != NULL && strcmp(rec->getSpoolDir(), "<NULL>") == 0) {
      free(rec->getSpoolDir());
      rec->setSpoolDir(NULL);
   }

   if(corrupted) {
      Dmsg0(0, "Could not read Settings Record. Journal is Corrupted.\n");

      if(rec != NULL) {
         delete rec;
         rec = NULL;
      }
   }

   this->endTransaction();
   return rec;
}

bool Journal::writeFileRecord(const FileRecord &record)
{
   int rc;
   bool success = true;
   char mtime_str[50];

   if(!this->beginTransaction("a")) {
      success = false;
      Dmsg0(0, "Could not start transaction for writeFileRecord()\n");
      goto bail_out;
   }

   edit_int64(record.mtime, mtime_str);
   rc = fprintf(_fp,
         "File {\n"
         "name=%s\n"
         "sname=%s\n"
         "mtime=%s\n"
         "attrs=%s\n"
         "}\n",
         record.name,
         record.sname,
         mtime_str,
         record.fattrs);

   if(rc < 0) {
      success = false;
      Dmsg1(50, "(ERROR) Could not write FileRecord. RC=%d\n", rc);
      goto bail_out;
   }

   Dmsg4(DBGLVL,
         "NEW RECORD:\n"
         " File {\n"
         "  name=%s\n"
         "  sname=%s\n"
         "  mtime=%s"
         "  attrs=%s\n"
         " }\n",
         record.name,
         record.sname,
         mtime_str,
         record.fattrs);

bail_out:
   this->endTransaction();
   return success;
}

FileRecord *Journal::readFileRecord()
{
   const int SANITY_CHECK = 10000;
   bool corrupted = false;
   char tmp[SANITY_CHECK];
   char fname[SANITY_CHECK];
   char sname[SANITY_CHECK];
   char fattrs[SANITY_CHECK];
   char mtime[SANITY_CHECK];
   char *mstr = NULL;
   FileRecord *rec = NULL;

   if(!hasTransaction) {
      Dmsg0(0, "(ERROR) Journal::readFileRecord() called without any transaction\n");
      goto bail_out;
   }

   //Reads lines until it finds a FileRecord, or until EOF
   for(;;) {
      if(!bfgets(tmp, SANITY_CHECK, _fp)) {
         goto bail_out;
      }

      if(strstr(tmp, "File {\n") != NULL) {
         break;
      }
   }

   rec = new FileRecord();

   //reads filename
   if(!bfgets(fname, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   rec->name = extract_val(fname);
   if(rec->name == NULL) {
      corrupted = true;
      goto bail_out;
   }

   //reads spoolname
   if(!bfgets(sname, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   rec->sname = extract_val(sname);
   if(rec->sname == NULL) {
      corrupted = true;
      goto bail_out;
   }

   //reads mtime
   if(!bfgets(mtime, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   mstr = extract_val(mtime);
   if(mstr == NULL) {
      corrupted = true;
      goto bail_out;
   }
   rec->mtime = atoi(mstr);

   //reads encoded attributes
   if(!bfgets(fattrs, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   rec->fattrs = extract_val(fattrs);
   if(rec->fattrs == NULL) {
      corrupted = true;
      goto bail_out;
   }

   Dmsg4(DBGLVL, 
         "READ RECORD:\n"
         " File {\n"
         "  name=%s\n"
         "  sname=%s\n"
         "  mtime=%s\n"
         "  attrs=%s\n"
         " }\n",
         rec->name,
         rec->sname,
         mstr,
         rec->fattrs);

   //reads line "}\n"
   if(!bfgets(tmp, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }

bail_out:
   if(mstr != NULL) {
      free(mstr);
   }

   if(corrupted) {
      Dmsg0(0, "Could not read File Record. Journal is Corrupted.\n");

      if(rec != NULL) {
         delete rec;
         rec = NULL;
      }
   }

   return rec;
}

bool Journal::writeFolderRecord(const FolderRecord &record)
{
   int rc;
   bool success = true;

   if(!this->beginTransaction("a")) {
      success = false;
      Dmsg0(0, "Could not start transaction for writeFileRecord()\n");
      goto bail_out;
   }

   rc = fprintf(_fp,
         "Folder {\n"
         "path=%s\n"
         "}\n",
         record.path);

   if(rc < 0) {
      success = false;
      Dmsg1(0, "(ERROR) Could not write FolderRecord. RC=%d\n", rc);
      goto bail_out;
   }

   Dmsg1(DBGLVL,
         "NEW RECORD:\n"
         " Folder {\n"
         "  path=%s\n"
         " }\n",
         record.path);

bail_out:
   this->endTransaction();
   return success;
}

FolderRecord *Journal::readFolderRecord()
{
   const int SANITY_CHECK = 10000;
   bool corrupted = false;
   char tmp[SANITY_CHECK];
   char path[SANITY_CHECK];
   FolderRecord *rec = NULL;

   if(!hasTransaction) {
      Dmsg0(0, "(ERROR) Journal::readFolderRecord() called without any transaction\n");
      goto bail_out;
   }

   //Reads lines until it finds a FolderRecord, or until EOF
   for(;;) {
      if(!bfgets(tmp, SANITY_CHECK, _fp)) {
         //No need to set 'corrupted = true' here
         goto bail_out;
      }

      if(strstr(tmp, "Folder {\n") != NULL) {
         break;
      }
   }

   rec = new FolderRecord();

   //reads folder path
   if(!bfgets(path, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }
   rec->path = extract_val(path);
   if(rec->path == NULL) {
      corrupted = true;
      goto bail_out;
   }

   Dmsg1(DBGLVL, 
         "READ RECORD:\n"
         " Folder {\n"
         "  path=%s\n"
         " }\n",
         rec->path);

   //reads line "}\n"
   if(!bfgets(tmp, SANITY_CHECK, _fp)) {
      corrupted = true;
      goto bail_out;
   }

bail_out:
   if(corrupted) {
      Dmsg0(0, "Could not read FolderRecord. Journal is Corrupted.\n");

      if(rec != NULL) {
         delete rec;
         rec = NULL;
      }
   }

   return rec;
}

bool Journal::removeFolderRecord(const char* folder)
{
   bool success = false;
   const int SANITY_CHECK = 10000;
   char path[SANITY_CHECK];
   char tmp[SANITY_CHECK];
   char *recPath;
   FILE *tmpFp = NULL;
   int rc;

   POOL_MEM tmp_jPath;
   Mmsg(tmp_jPath, "%s.temp", _jPath);

   if(!this->beginTransaction("r")) {
      goto bail_out;
   }

   tmpFp = bfopen(tmp_jPath.c_str(), "w");

   if(tmpFp == NULL) {
      goto bail_out;
   }

   for(;;) {
      if(!bfgets(tmp, SANITY_CHECK, _fp)) {
         goto bail_out;
      }

      if(strstr(tmp, "Folder {\n") != NULL) {
         //reads folder path
         if(!bfgets(path, SANITY_CHECK, _fp)) {
            goto bail_out;
         }

         recPath = extract_val(path);

         if(recPath == NULL) {
            goto bail_out;
         }

         //reads line "}\n"
         if(!bfgets(tmp, SANITY_CHECK, _fp)) {
            goto bail_out;
         }

         if(bstrcmp(folder, recPath) == 0) {
            //Didn't found the Record that needs to be removed
            rc = fprintf(tmpFp,
                  "Folder {\n"
                  "path=%s\n"
                  "}\n",
                  recPath);

            if(rc < 0) {
               goto bail_out;
            }
         } else {
            //Found the Folder Record that needs to be removed
            success = true;
         }

      } else {
         fprintf(tmpFp, "%s", tmp);
      }
   }

bail_out:

   if(tmpFp != NULL) {
      fclose(tmpFp);
   }

   if(success) {
      fclose(_fp);
      _fp = NULL;
      unlink(_jPath);
      rc = rename(tmp_jPath.c_str(), _jPath);

      if(rc != 0) {
         Dmsg0(0, "Could not rename TMP Journal\n");
      }
   }

   this->endTransaction();
   return success;
}

bool Journal::migrateTo(const char *newPath)
{
   bool success = true;
   const int SANITY_CHECK = 10000;
   char tmp[SANITY_CHECK];
   FILE *tmpFp = NULL;
   FILE *newFp = NULL;
   int rc;

   POOLMEM *tmp_jPath = get_pool_memory(PM_FNAME);
   Mmsg(tmp_jPath, "%s.temp", newPath);

   if(!this->beginTransaction("r")) {
      success = false;
      goto bail_out;
   }

   Dmsg2(DBGLVL, "Migrating Journal %s to %s...\n", _jPath, newPath);
   tmpFp = bfopen(tmp_jPath, "w");
   newFp = bfopen(newPath, "w");

   if (tmpFp == NULL) { 
      Dmsg1(0, "Could not bfopen %s. Aborting migration.\n", tmp_jPath);
      success = false;
      goto bail_out;
   }

   if (newFp == NULL) {
      Dmsg1(0, "Could not bfopen %s. Aborting migration.\n", newPath);
      success = false;
      goto bail_out;
   }

   // Migrate everything to the new Journal ('newFp')
   // Remove FileRecords from the old Journal
   // by not saving them in 'tmpFp'
   for(;;) {
      if(!bfgets(tmp, SANITY_CHECK, _fp)) {
         break;
      }

      if(strstr(tmp, "File {") != NULL) {
         //Found a FileRecord
         //Write it only in the New Jornal
         fprintf(newFp, "%s", tmp);

         for(int i = 0; i < 5; i++) {
            if(!bfgets(tmp, SANITY_CHECK, _fp)) {
               //Found a corrupted FileRecord
               Dmsg0(0, "Found a corrupt FileRecord. Canceling Migration");
               success = false;
               goto bail_out;
            }

            fprintf(newFp, "%s", tmp);
         }

      } else {
         fprintf(newFp, "%s", tmp);
         fprintf(tmpFp, "%s", tmp);
      }
   }

bail_out:

   if(newFp != NULL) {
      fclose(newFp);
   }

   if(tmpFp != NULL) {
      fclose(tmpFp);
   }

   if(success) {
      fclose(_fp);
      _fp = NULL;
      unlink(_jPath);
      rc = rename(tmp_jPath, _jPath);

      if(rc != 0) {
         Dmsg0(0, "Could not rename TMP Journal\n");
      }

      free(_jPath);
      _jPath = bstrdup(newPath);
      Dmsg0(DBGLVL, "Journal migration completed\n");
   }

   free_and_null_pool_memory(tmp_jPath);
   this->endTransaction();
   return success;
}
