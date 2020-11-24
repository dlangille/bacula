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

#ifndef folder_record_H
#define folder_record_H

#include "bacula.h"
#include <string.h>

/**
 * @brief Data that is saved and retrieved by using the @class Journal
 */
class FolderRecord
{
public:
    char *path;

    FolderRecord():
        path(NULL)
    {}

    ~FolderRecord() {
        if (path != NULL) {
            free(path);
        }

    }
};

#endif
