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
/*
 * Routines for writing to the Cloud using S3 protocol.
 *
 * Written by Kern Sibbald, May MMXVI
 */

#ifndef _FILE_DRV_H
#define _FILE_DRV_H

#include "bacula.h"
#include "stored.h"
#include "cloud_driver.h"   /* get base class definitions */

class file_driver: public cloud_driver {
public:
   file_driver() {
   };
   ~file_driver() {
   };
};

#endif /* _FILE_DRV_H */
