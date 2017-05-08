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
 * Generic routines for writing Cloud Volumes
 *
 * Written by Kern Sibbald, May MMXVI
 */

#ifndef _CLOUD_DEV_H_
#define _CLOUD_DEV_H_

#include "bacula.h"
#include "stored.h"
#include "cloud_driver.h"

class cloud_dev: public file_dev {
public:
   int64_t obj_len;
   int status;

public:
   cloud_dev(JCR *jcr, DEVRES *device);
   ~cloud_dev();

   cloud_driver *driver;

   /* DEVICE virtual interfaces that we redefine */
   boffset_t lseek(DCR *dcr, off_t offset, int whence);
   bool rewind(DCR *dcr);
   bool reposition(DCR *dcr, uint64_t raddr);
   bool open_device(DCR *dcr, int omode);
   bool open_next_part(DCR *dcr);
   bool truncate(DCR *dcr);
   int  truncate_cache(DCR *dcr, const char *VolName, int64_t *size);
   bool upload_cache(DCR *dcr, const char *VolName, POOLMEM *&err);
   bool close(DCR *dcr);
   bool update_pos(DCR *dcr);
   bool is_eod_valid(DCR *dcr);
   bool eod(DCR *dcr);
   int read_dev_volume_label(DCR *dcr);
   const char *print_type();
   DEVICE *get_dev(DCR *dcr);
   uint32_t get_hi_addr();
   uint32_t get_low_addr();
   uint64_t get_full_addr();
   uint64_t get_full_addr(boffset_t addr);
   char *print_addr(char *buf, int32_t buf_len);
   char *print_addr(char *buf, int32_t maxlen, boffset_t addr);
   bool do_size_checks(DCR *dcr, DEV_BLOCK *block);
   bool write_volume_label(DCR *dcr,
           const char *VolName, const char *PoolName,
           bool relabel, bool no_prelabel);
   bool rewrite_volume_label(DCR *dcr, bool recycle);
   bool start_of_job(DCR *dcr);
   bool end_of_job(DCR *dcr);
   bool get_cloud_volumes_list(DCR* dcr, alist *volumes, POOLMEM *&err) { return driver->get_cloud_volumes_list(dcr, volumes, err); };
   bool get_cloud_volume_parts_list(DCR *dcr, const char *VolumeName, ilist *parts, POOLMEM *&err) { return driver->get_cloud_volume_parts_list(dcr, VolumeName, parts, err);};
   uint32_t get_cloud_upload_transfer_status(POOL_MEM &msg, bool verbose);
   uint32_t get_cloud_download_transfer_status(POOL_MEM &msg, bool verbose);
};

#endif /* _CLOUD_DEV_H_ */
