/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2019 Kern Sibbald

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
/*
 * This is a Bacula plugin for backup/restore Docker using native tools.
 *
 * Author: Radosław Korzeniewski, MMXIX
 * radoslaw@korzeniewski.net, radekk@inteos.pl
 * Inteos Sp. z o.o. http://www.inteos.pl/
 */

#ifndef _DKID_H_
#define _DKID_H_

#include "bacula.h"

#define DKIDDIGESTSIZE        64       // the size of string array for hex chars, without trailing nul
#define DKIDDIGESTShortSIZE   12       // the number of hex characters in short digest, without trailing nul
#define DKIDInvalid           -256     // the non-trivial negative value :)

/*
 * This is a simple storage class to handle Docker Container IDs
 */
class DKID: public SMARTALLOC {
 public:
    DKID();
    DKID(const char *data);
    DKID(POOL_MEM &data);
    ~DKID() {};

    inline int64_t id() { return ShortD; };
    inline char *digest() { return Digest; };
    inline char *digest_short() { return DigestShort; };
    inline operator int64_t () { return ShortD; };
    inline operator char* () { return Digest; };
    DKID& operator= (char *data);
    DKID& operator= (DKID &other);
    DKID& operator= (POOL_MEM &data);
    bool operator== (DKID &other);
    bool operator!= (DKID &other);
#ifdef TEST_PROGRAM
    void dump();
#endif

 private:
   char Digest[DKIDDIGESTSIZE + 1];
   char DigestShort[DKIDDIGESTShortSIZE + 1];
   int64_t ShortD; // default short digest on Docker is 48bits/6bytes/12hex chars
                   // https://github.com/docker/cli/blob/master/vendor/github.com/docker/docker/pkg/stringid/stringid.go
   bool shortonly;

   void init(const char* d);
};

#endif   /* _DKID_H_ */