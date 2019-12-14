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

#ifndef _DOCKER_FD_H_
#define _DOCKER_FD_H_

#include "dkcommctx.h"

/* Plugin Info definitions */
#define DOCKER_LICENSE              "Bacula AGPLv3"
#define DOCKER_AUTHOR               "Radoslaw Korzeniewski"
#define DOCKER_DATE                 "Oct 2019"
#define DOCKER_VERSION              "1.2.0"
#define DOCKER_DESCRIPTION          "Bacula Docker Plugin"

/* Plugin compile time variables */
#define PLUGINPREFIX                "docker:"
#define PLUGINNAME                  "Docker"
#define PLUGINNAMESPACE             "/@docker"
#define CONTAINERNAMESPACE          "/container"
#define IMAGENAMESPACE              "/image"
#define VOLUMENAMESPACE             "/volume"

/* types used by Plugin */
typedef enum {
   DOCKER_NONE = 0,
   DOCKER_BACKUP_FULL,
   DOCKER_BACKUP_INCR,
   DOCKER_BACKUP_DIFF,
   DOCKER_BACKUP_VOLUME_FULL,
   DOCKER_BACKUP_CONTAINER_VOLLIST,
   DOCKER_RESTORE,
   DOCKER_RESTORE_VOLUME,
} DOCKER_MODE_T;

#define pluginclass(ctx)     (DOCKER*)ctx->pContext;

/* listing mode */
typedef enum {
   DOCKER_LISTING_NONE = 0,
   DOCKER_LISTING_TOP,
   DOCKER_LISTING_IMAGE,
   DOCKER_LISTING_CONTAINER,
   DOCKER_LISTING_VOLUME,
} DOCKER_LISTING_MODE;

/* listing objects for plugin */
typedef struct {
   const char *name;
   DOCKER_LISTING_MODE mode;
} DOCKER_LISTING_T;

static DOCKER_LISTING_T docker_objects[] = {
   {"/",             DOCKER_LISTING_TOP},
   {"image",         DOCKER_LISTING_IMAGE},
   {"container",     DOCKER_LISTING_CONTAINER},
   {"volume",        DOCKER_LISTING_VOLUME},
   {NULL,            DOCKER_LISTING_NONE},
};

/*
 * This is a main plugin API class. It manages a plugin context.
 *  All the public methods correspond to a public Bacula API calls, even if
 *  a callback is not implemented.
 */
class DOCKER: public SMARTALLOC {
 public:
   bRC getPluginValue(bpContext *ctx, pVariable var, void *value);
   bRC setPluginValue(bpContext *ctx, pVariable var, void *value);
   bRC handlePluginEvent(bpContext *ctx, bEvent *event, void *value);
   bRC startBackupFile(bpContext *ctx, struct save_pkt *sp);
   bRC endBackupFile(bpContext *ctx);
   bRC startRestoreFile(bpContext *ctx, const char *cmd);
   bRC endRestoreFile(bpContext *ctx);
   bRC pluginIO(bpContext *ctx, struct io_pkt *io);
   bRC createFile(bpContext *ctx, struct restore_pkt *rp);
   bRC setFileAttributes(bpContext *ctx, struct restore_pkt *rp);
   bRC checkFile(bpContext *ctx, char *fname);
   bRC handleXACLdata(bpContext *ctx, struct xacl_pkt *xacl);
   void setworkingdir(char *workdir);
   DOCKER(bpContext *bpctx);
   ~DOCKER();

 private:
   bpContext *ctx;                     /* Bacula Plugin Context */
   DOCKER_MODE_T mode;                 /* Plugin mode of operation */
   int JobId;                          /* Job ID */
   char *JobName;                      /* Job name */
   time_t since;                       /* Job since parameter */
   char *where;                        /* the Where variable for restore job if set by user */
   char *regexwhere;                   /* the RegexWhere variable for restore job if set by user */
   char replace;                       /* the replace variable for restore job */
   bool robjsent;                      /* set when RestoreObject was sent during Full backup */
   bool estimate;                      /* used when mode is DOCKER_BACKUP_* but we are doing estimate only */
   bool accurate_warning;              /* for sending accurate mode warning once */
   bool local_restore;                 /* if where parameter is set to local path then make a local restore */
   bool backup_finish;                 /* the hack to force finish backup list */
   bool unsupportedfeature;            /* this flag show if plugin should report unsupported feature like unsupported backup level*/
   bool param_notrunc;                 /* when "notrunc" option specified, used in listing mode only */
   bool errortar;                      /* show if container tar for volume archive had errors */
   bool volumewarning;                 /* when set then a warning about remote docker volume restore was sent to user */
   int dockerworkclear;                /* set to 1 when docker working dir should be cleaned */
   /* TODO: define a variable which will signal job cancel */
   DKCOMMCTX *dkcommctx;               /* the current command tool execution context */
   alist *commandlist;                 /* the command context list for multiple config execution for a single job */
   POOLMEM *fname;                     /* current file name to backup or restore */
   POOLMEM *lname;                     /* current link name to estimate or listing */
   int dkfd;                           /* the file descriptor for local restore and volume backup */
   POOLMEM *robjbuf;                   /* the buffer for restore object data */
   DKINFO *currdkinfo;                 /* current docker object - container or image to backup */
   DKINFO *restoredkinfo;              /* the current docker object - container or image to restore */
   DKVOLS *currvols;                   /* current docker volume object for backup or restore */
   DOCKER_LISTING_MODE listing_mode;   /* the listing mode */
   int listing_objnr;                  /* when at listing top mode iterate through docker_objects */
   cmd_parser *parser;                 /* the plugin params parser */
   POOLMEM *workingdir;                /* runtime working directory from file daemon */

   bRC parse_plugin_command(bpContext *ctx, const char *command);
   bRC parse_plugin_restoreobj(bpContext *ctx, restore_object_pkt *rop);
   bRC prepare_bejob(bpContext* ctx, char *command);
   bRC prepare_estimate(bpContext *ctx, char *command);
   bRC prepare_backup(bpContext *ctx, char *command);
   bRC prepare_restore(bpContext *ctx, char *command);
   bRC perform_backup_open(bpContext *ctx, struct io_pkt *io);
   bRC perform_restore_open(bpContext *ctx, struct io_pkt *io);
   bRC perform_read_data(bpContext *ctx, struct io_pkt *io);
   bRC perform_read_volume_data(bpContext *ctx, struct io_pkt *io);
   bRC perform_write_data(bpContext *ctx, struct io_pkt *io);
   bRC perform_restore_close(bpContext *ctx, struct io_pkt *io);
   bRC perform_backup_close(bpContext *ctx, struct io_pkt *io);
   void new_commandctx(bpContext *ctx, const char *command);
   void switch_commandctx(bpContext *ctx, const char *command);
   DKINFO *search_docker_image(bpContext *ctx);
   DKINFO *search_docker_volume(bpContext *ctx);
   bool check_container_tar_error(bpContext *ctx, char *volname);
};

#endif   /* _DOCKER_FD_H_ */
