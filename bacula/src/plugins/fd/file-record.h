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

#ifndef journalfilerecord_H
#define journalfilerecord_H

#include "bacula.h"
#include <string.h>

/**
 * Implementation is part of Bacula's findlib module, and is on:
 * "/src/findlib/attribs.c"
 */
extern int decode_stat(char *buf, struct stat *statp, int stat_size, int32_t *LinkFI);
extern void encode_stat(char *buf, struct stat *statp, int stat_size, int32_t LinkFI, int data_stream);

/**
 * @brief Data that is saved and retrieved by using the @class Journal
 */
class FileRecord
{
   public:
      char *name;
      char *sname;
      char *fattrs;
      int64_t mtime;

      FileRecord():
         name(NULL), sname(NULL), fattrs(NULL), mtime(0)
      {}

      bool encode_attrs() {
         struct stat statbuf;
#ifndef HAVE_WIN32
         if(lstat(this->name, &statbuf) != 0) {
            return false;
         }

         this->mtime = (int64_t) statbuf.st_mtime;
         this->fattrs = (char *) malloc(500 * sizeof(char));
         encode_stat(this->fattrs, &statbuf, sizeof(statbuf), 0, 0); 
#else
         FILE *fp = fopen(this->name, "r");

         if (!fp) {
            Dmsg1(0, "Could not open file %s\n", this->name);
            return false;
         }

         int fd = _fileno(fp);

         if(fstat(fd, &statbuf) != 0) {
            fclose(fp);
            Dmsg1(0, "Could not encode attributes of file %s\n", this->name);
            return false;
         }

         this->mtime = (int64_t) statbuf.st_mtime;
         this->fattrs = (char *) malloc(500 * sizeof(char));
         encode_stat(this->fattrs, &statbuf, sizeof(statbuf), 0, 0); 
         fclose(fp);
#endif
         return true;
      }

      void decode_attrs(struct stat &sbuf) {
         int32_t lfi;
         decode_stat(this->fattrs, &sbuf, sizeof(sbuf), &lfi);
      }

      bool equals(const FileRecord *rec) {
         return strcmp(this->name, rec->name) == 0
            && strcmp(this->fattrs, rec->fattrs) == 0
            && this->mtime == rec->mtime;
      }

      void getBaculaName(POOLMEM *target) {
         char mtime_date[200];
         time_t t = (time_t) mtime;
         struct tm *timeinfo = localtime(&t);
         strftime(mtime_date, 200, "%Y%m%d_%H%M%S", timeinfo);
         Mmsg(target, "%s.%s", name, mtime_date);
      }

      ~FileRecord() {
         if (name != NULL) {
            free(name);
         }

         if (sname != NULL) {
            free(sname);
         }

         if (fattrs != NULL) {
            free(fattrs);
         }
      }
};

#endif
