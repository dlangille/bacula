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

#ifndef journal_H
#define journal_H

#include "settings-record.h"
#include "folder-record.h"
#include "file-record.h"

#ifndef HAVE_WIN32
#include <sys/file.h>
#endif

#ifdef HAVE_WIN32
#define JOURNAL_CLI_FNAME "bcdp-cli.journal"
#else
#define JOURNAL_CLI_FNAME ".bcdp-cli.journal"
#endif

#define JOURNAL_VERSION 1

/**
 * @brief The Journal persists and retrieves @class FileRecord objects.
 *
 * Used by:
 *
 * 1-) The CDP Client, to store information about files that
 * should backed up.
 *
 * 2-) The CDP FD Plugin, to decide which file should be backed
 * up on a specific Job.
 *
 * The current implementation uses a plain text file to store the records.
 */
class Journal
{

private:
    FILE * _fp;
    int _fd;

public:
    char *_jPath;
    bool hasTransaction;

    Journal():
        _fp(NULL), _fd(-1), _jPath(NULL), hasTransaction(false)
    {}

    ~Journal() {}

    bool setJournalPath(const char *path);
    bool setJournalPath(const char *path, const char *spoolDir);
    bool migrateTo(const char* newPath);

    bool beginTransaction(const char *mode);
    void endTransaction();

    bool writeSettings(SettingsRecord &record);
    SettingsRecord *readSettings();

    bool writeFileRecord(const FileRecord &record);
    FileRecord *readFileRecord();

    bool removeFolderRecord(const char *folder);
    bool writeFolderRecord(const FolderRecord &record);
    FolderRecord *readFolderRecord();

    /** Public only because it's used by Unit Tests */
    char *extract_val(const char *key_val);

    //TODO: warnSizeFull();
    //TODO: removeRecord()
    //TODO: pruneRecords()
};

#endif
