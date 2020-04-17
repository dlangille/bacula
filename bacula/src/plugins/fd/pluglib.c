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
/*
 * Common definitions and utility functions for Inteos plugins.
 * Functions defines a common framework used in our utilities and plugins.
 *
 * Author: Radosław Korzeniewski, MMXIX
 * radoslaw@korzeniewski.net, radekk@inteos.pl
 * Inteos Sp. z o.o. http://www.inteos.pl/
 */

#include "pluglib.h"

/* Events that are passed to plugin
typedef enum {
  bEventJobStart                        = 1,
  bEventJobEnd                          = 2,
  bEventStartBackupJob                  = 3,
  bEventEndBackupJob                    = 4,
  bEventStartRestoreJob                 = 5,
  bEventEndRestoreJob                   = 6,
  bEventStartVerifyJob                  = 7,
  bEventEndVerifyJob                    = 8,
  bEventBackupCommand                   = 9,
  bEventRestoreCommand                  = 10,
  bEventEstimateCommand                 = 11,
  bEventLevel                           = 12,
  bEventSince                           = 13,
  bEventCancelCommand                   = 14,
  bEventVssBackupAddComponents          = 15,
  bEventVssRestoreLoadComponentMetadata = 16,
  bEventVssRestoreSetComponentsSelected = 17,
  bEventRestoreObject                   = 18,
  bEventEndFileSet                      = 19,
  bEventPluginCommand                   = 20,
  bEventVssBeforeCloseRestore           = 21,
  bEventVssPrepareSnapshot              = 22,
  bEventOptionPlugin                    = 23,
  bEventHandleBackupFile                = 24,
  bEventComponentInfo                   = 25
} bEventType;
*/

const char *eventtype2str(bEvent *event){
   switch (event->eventType){
      case bEventJobStart:
         return "bEventJobStart";
      case bEventJobEnd:
         return "bEventJobEnd";
      case bEventStartBackupJob:
         return "bEventStartBackupJob";
      case bEventEndBackupJob:
         return "bEventEndBackupJob";
      case bEventStartRestoreJob:
         return "bEventStartRestoreJob";
      case bEventEndRestoreJob:
         return "bEventEndRestoreJob";
      case bEventStartVerifyJob:
         return "bEventStartVerifyJob";
      case bEventEndVerifyJob:
         return "bEventEndVerifyJob";
      case bEventBackupCommand:
         return "bEventBackupCommand";
      case bEventRestoreCommand:
         return "bEventRestoreCommand";
      case bEventEstimateCommand:
         return "bEventEstimateCommand";
      case bEventLevel:
         return "bEventLevel";
      case bEventSince:
         return "bEventSince";
      case bEventCancelCommand:
         return "bEventCancelCommand";
      case bEventVssBackupAddComponents:
         return "bEventVssBackupAddComponents";
      case bEventVssRestoreLoadComponentMetadata:
         return "bEventVssRestoreLoadComponentMetadata";
      case bEventVssRestoreSetComponentsSelected:
         return "bEventVssRestoreSetComponentsSelected";
      case bEventRestoreObject:
         return "bEventRestoreObject";
      case bEventEndFileSet:
         return "bEventEndFileSet";
      case bEventPluginCommand:
         return "bEventPluginCommand";
      case bEventVssBeforeCloseRestore:
         return "bEventVssBeforeCloseRestore";
      case bEventVssPrepareSnapshot:
         return "bEventVssPrepareSnapshot";
      case bEventOptionPlugin:
         return "bEventOptionPlugin";
      case bEventHandleBackupFile:
         return "bEventHandleBackupFile";
      case bEventComponentInfo:
         return "bEventComponentInfo";
      default:
         return "Unknown";
   }
}


/*
 * Return the real size of the disk based on the size suffix.
 *
 * in:
 *    disksize - the numeric value of the disk size to compute
 *    suff - the suffix for a disksize value
 * out:
 *    uint64_t - the size of the disk computed with suffix
 */
uint64_t pluglib_size_suffix(int disksize, char suff)
{
   uint64_t size;

   switch (suff){
      case 'G':
         size = (uint64_t)disksize * 1024 * 1048576;
         break;
      case 'M':
         size = (uint64_t)disksize * 1048576;
         break;
      case 'T':
         size = (uint64_t)disksize * 1048576 * 1048576;
         break;
      case 'K':
      case 'k':
         size = (uint64_t)disksize * 1024;
         break;
      default:
         size = disksize;
   }
   return size;
}

/*
 * Return the real size of the disk based on the size suffix.
 *    This version uses a floating point numbers (double) for computation.
 *
 * in:
 *    disksize - the numeric value of the disk size to compute
 *    suff - the suffix for a disksize value
 * out:
 *    uint64_t - the size of the disk computed with suffix
 */
uint64_t pluglib_size_suffix(double disksize, char suff)
{
   uint64_t size;

   switch (suff){
      case 'G':
         size = disksize * 1024.0 * 1048576.0;
         break;
      case 'M':
         size = disksize * 1048576.0;
         break;
      case 'T':
         size = disksize * 1048576.0 * 1048576.0;
         break;
      case 'K':
      case 'k':
         size = disksize * 1024.0;
         break;
      default:
         size = disksize;
   }
   return size;
}

/*
 * Creates a path hierarchy on local FS.
 *  It is used for local restore mode to create a required directory.
 *  The functionality is similar to 'mkdir -p'.
 *
 * TODO: make a support for relative path
 * TODO: check if we can use findlib/makepath implementation instead
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    path - a full path to create, does not check if the path is relative,
 *           could fail in this case
 * out:
 *    bRC_OK - path creation was successful
 *    bRC_Error - on any error
 */
bRC pluglib_mkpath(bpContext* ctx, char* path, bool isfatal)
{
#ifdef PLUGINPREFIX
#define _OLDPREFIX   PLUGINPREFIX
#endif
#define PLUGINPREFIX    "pluglibmkpath:"
   struct stat statp;
   POOL_MEM dir(PM_FNAME);
   char *p, *q;

   if (!path){
      return bRC_Error;
   }
   if (stat(path, &statp) == 0){
      if (S_ISDIR(statp.st_mode)){
         return bRC_OK;
      } else {
         DMSG(ctx, DERROR, "Path %s is not directory\n", path);
         JMSG(ctx, isfatal ? M_FATAL : M_ERROR, "Path %s is not directory\n", path);
         return bRC_Error;
      }
   }
   DMSG(ctx, DDEBUG, "mkpath verify dir: %s\n", path);
   pm_strcpy(dir, path);
   p = dir.addr() + 1;
   while (*p && (q = strchr(p, (int)PathSeparator)) != NULL){
      *q = 0;
      DMSG(ctx, DDEBUG, "mkpath scanning(1): %s\n", dir.c_str());
      if (stat(dir.c_str(), &statp) == 0){
         *q = PathSeparator;
         p = q + 1;
         continue;
      }
      DMSG0(ctx, DDEBUG, "mkpath will create dir(1).\n");
      if (mkdir(dir.c_str(), 0750) < 0){
         /* error */
         berrno be;
         DMSG2(ctx, DERROR, "Cannot create directory %s Err=%s\n", dir.c_str(), be.bstrerror());
         JMSG2(ctx, isfatal ? M_FATAL : M_ERROR, "Cannot create directory %s Err=%s\n", dir.c_str(), be.bstrerror());
         return bRC_Error;
      }
      *q = PathSeparator;
      p = q + 1;
   }
   DMSG0(ctx, DDEBUG, "mkpath will create dir(2).\n");
   if (mkdir(path, 0750) < 0){
      /* error */
      berrno be;
      DMSG2(ctx, DERROR, "Cannot create directory %s Err=%s\n", path, be.bstrerror());
      JMSG2(ctx, isfatal ? M_FATAL : M_ERROR, "Cannot create directory %s Err=%s\n", path, be.bstrerror());
      return bRC_Error;
   }
   DMSG0(ctx, DDEBUG, "mkpath finish.\n");
#ifdef _OLDPREFIX
#define PLUGINPREFIX    _OLDPREFIX
#undef _OLDPREFIX
#else
#undef PLUGINPREFIX
#endif
   return bRC_OK;
}
