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

#ifndef _DKCOMMCTX_H_
#define _DKCOMMCTX_H_

#include "pluglib.h"
#include "lib/ini.h"
#include "lib/bregex.h"

#define USE_CMD_PARSER
#include "fd_common.h"
#include "dkinfo.h"

/* Plugin compile time variables */
#ifndef DOCKER_CMD
#ifndef HAVE_WIN32
#define DOCKER_CMD                  "/usr/bin/docker"
#else
#define DOCKER_CMD                  "C:/Program Files/Docker/Docker/resources/bin/docker.exe"
#endif
#endif

#ifndef WORKDIR
#define WORKDIR                     "/opt/bacula/working"
#endif

#define BACULATARIMAGE              "baculatar:" DOCKER_TAR_IMAGE

#define BACULACONTAINERFOUT         "fout"
#define BACULACONTAINERFIN          "fin"
#define BACULACONTAINERERRLOG       "docker.err"
#define BACULACONTAINERARCHLOG      "docker.log"

/*
 * Supported backup modes
 */
typedef enum {
   DKPAUSE,
   DKNOPAUSE,
} DOCKER_BACKUP_MODE_T;

/*
 * The list of restore options saved to the RestoreObject.
 */
static struct ini_items plugin_items_dump[] = {
//  name                         handler             comment                                      required  default
   {"container_create",          ini_store_bool,     "Create container on restore",                      0, "*Yes*"},
   {"container_run",             ini_store_bool,     "Run container on restore",                         0, "*No*"},
   {"container_imageid",         ini_store_bool,     "Use Image Id for container creation/start",        0, "*No*"},
   {"container_defaultnames",    ini_store_bool,     "Use default docker Names on container creation",   0, "*No*"},
   {"docker_host",               ini_store_str,      "Use defined docker host to restore",                0, "*local*"},
   {"timeout",                   ini_store_int32,    "Timeout connecting to volume container",           0, "*30*"},
   {NULL, NULL, NULL, 0, NULL}
};

/*
 * This is a low-level communication class which handles command tools execution.
 */
class DKCOMMCTX: public SMARTALLOC {
 public:
   char *command;

   alist *get_all_containers(bpContext *ctx);
   alist *get_all_images(bpContext *ctx);
   alist *get_all_volumes(bpContext *ctx);
   void release_all_dkinfo_list(alist **list);
   void release_all_pm_list(alist **list);
   void set_all_to_backup(bpContext *ctx);
   void set_all_containers_to_backup(bpContext *ctx);
   void set_all_images_to_backup(bpContext *ctx);
   void set_all_volumes_to_backup(bpContext *ctx);

   inline DKINFO *get_first_to_backup(bpContext *ctx) { return (DKINFO*)objs_to_backup->first(); };
   inline DKINFO *get_next_to_backup(bpContext *ctx) { return (DKINFO*)objs_to_backup->next(); };
   inline void finish_backup_list(bpContext *ctx) { objs_to_backup->last(); };

   bRC container_commit(bpContext *ctx, DKINFO *dkinfo, int jobid);
   bRC delete_container_commit(bpContext *ctx, DKINFO *dkinfo, int jobid);
   bRC image_save(bpContext *ctx, DKID *dkid);
   bRC backup_docker(bpContext *ctx, DKINFO *dkinfo, int jobid);
   bRC restore_docker(bpContext *ctx, DKINFO *dkinfo, int jobid);
   bRC docker_tag(bpContext* ctx, DKID &dkid, POOLMEM *tag);
   bRC docker_create_run_container(bpContext* ctx, DKINFO *dkinfo);
   bRC wait_for_restore(bpContext *ctx, DKID &dkid);
   void update_vols_mounts(bpContext* ctx, DKINFO *container, DKVOLS *volume);

   int32_t read_data(bpContext *ctx, POOLMEM *buf, int32_t len);
   int32_t read_output(bpContext *ctx, POOL_MEM &out);
   int32_t write_data(bpContext *ctx, POOLMEM *buf, int32_t len);
   void terminate(bpContext *ctx);
   inline int get_backend_pid() { if (bpipe){ return bpipe->worker_pid; } return -1;};

   bRC parse_parameters(bpContext *ctx, char *argk, char *argv);
   bRC parse_restoreobj(bpContext *ctx, restore_object_pkt *rop);
   bRC prepare_bejob(bpContext *ctx, bool estimate);
   bRC prepare_restore(bpContext *ctx);
   bRC prepare_working_volume(bpContext* ctx, int jobid);
   void clean_working_volume(bpContext* ctx);
   inline void render_working_volume_filename(POOL_MEM &buf, const char *fname)
      { Mmsg(buf, "%s/%s", workingvolume, fname); };
   void setworkingdir(char *workdir);

   inline bool is_open() { return bpipe != NULL; };
   inline bool is_closed() { return bpipe == NULL; };
   inline bool is_error() { return f_error || f_fatal; };
   inline void set_error() { f_error = true; };
   inline bool is_fatal() { return f_fatal || (f_error && abort_on_error); };
   inline bool is_eod() { return f_eod; };
   inline void clear_eod() { f_eod = false; };
   inline void set_eod() { f_eod = true; };
   inline void set_abort_on_error() { abort_on_error = true; };
   inline void clear_abort_on_error() { abort_on_error = false; };
   inline bool is_abort_on_error() { return abort_on_error; };
   inline bool is_all_vols_to_backup() { return all_vols_to_backup; };
   inline bool is_remote_docker() { return param_docker_host != NULL; };
   inline int32_t timeout() { return param_timeout; };

   DKCOMMCTX(const char *cmd);
   ~DKCOMMCTX();

 private:
   BPIPE *bpipe;                          /* this is our bpipe to communicate with command tools */
   alist *param_include_container;        /* the include parameter list which filter what container name to backup as regex */
   alist *param_include_image;            /* the include parameter list which filter what image name to backup as regex */
   alist *param_exclude_container;        /* the exclude parameter list which filter what container name to exclude from backup */
   alist *param_exclude_image;            /* the exclude parameter list which filter what image name to exclude from backup */
   alist *param_container;                /* the container parameter list which filter what container name or id to backup */
   alist *param_image;                    /* the image parameter list which filter what image name or id to backup */
   alist *param_volume;                   /* the volume parameter list which filter what volume name to backup */
   DOCKER_BACKUP_MODE_T param_mode;       /* the mode parameter which is used with docker commit, default is pause */
   bool param_container_create;           /* the restore parameter for container creation */
   bool param_container_run;              /* the restore parameter for container creation and execution */
   bool param_container_imageid;          /* the restore parameter for setting imageid during container creation/run */
   bool param_container_defaultnames;     /* the restore parameter for setting default docker names on container creation */
   POOLMEM *param_docker_host;            /* use defined docker host to docker operations */
   int32_t param_timeout;                 /* a timeout opening container communication pipe, the default is 30 */
   regex_t preg;                          /* this is a regex context for include/exclude */
   bool abort_on_error;                   /* abort on error flag */
   alist *all_containers;                 /* the list of all containers defined on Docker */
   alist *all_images;                     /* the list of all docker images defined on Docker */
   alist *all_volumes;                    /* the list of all docker volumes defined on Docker */
   alist *objs_to_backup;                 /* the list of all docker objects selected to backup or filtered */
   bool all_to_backup;                    /* if true use all_containers list to backup or containers_to_backup list when false */
   bool all_vols_to_backup;               /* if true use all volumes for container to backup */
   bool f_eod;                            /* the command tool signaled EOD */
   bool f_error;                          /* the plugin signaled an error */
   bool f_fatal;                          /* the plugin signaled a fatal error */
   ConfigFile *ini;                       /* restore object config parser */
   POOLMEM *workingvolume;                /* */
   POOLMEM *workingdir;                   /* runtime working directory from file daemon */

   bool execute_command(bpContext *ctx, POOLMEM *args);
   bool execute_command(bpContext *ctx, const char *args);
   bool execute_command(bpContext *ctx, POOL_MEM &args);
   void parse_parameters(bpContext *ctx, ini_items &item);
   bool render_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *fmt, const char *name, char *value);
   bool render_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *fmt, const char *name, int value);
   bool render_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *name, char *value);
   bool render_param(bpContext *ctx, bool *param, const char *pname, const char *name, bool value);
   bool render_param(bpContext *ctx, int32_t *param, const char *pname, const char *name, int32_t value);
   bool add_param_str(bpContext *ctx, alist **list, const char *pname, const char *name, char *value);
   bool parse_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *name, char *value);
   bool parse_param(bpContext *ctx, bool *param, const char *pname, const char *name, char *value);
   bool parse_param(bpContext *ctx, int32_t *param, const char *pname, const char *name, char *value);
   bool parse_param(bpContext *ctx, DOCKER_BACKUP_MODE_T *param, const char *pname, const char *name, char *value);

   void filter_param_to_backup(bpContext *ctx, alist *params, alist *dklist, bool estimate);
   void filter_incex_to_backup(bpContext *ctx, alist *params_include, alist *params_exclude, alist *dklist);
   void add_container_volumes_to_backup(bpContext *ctx);
   void select_container_vols(bpContext *ctx);
   alist *get_all_list_from_docker(bpContext* ctx, const char *cmd, int cols, alist **dklist, DKINFO_OBJ_t type);
   void setup_dkinfo(bpContext* ctx, DKINFO_OBJ_t type, char *paramtab[], DKINFO *dkinfo);
   void setup_container_dkinfo(bpContext* ctx, char *paramtab[], DKINFO *dkinfo);
   void setup_image_dkinfo(bpContext* ctx, char *paramtab[], DKINFO *dkinfo);
   void setup_volume_dkinfo(bpContext* ctx, char *paramtab[], DKINFO *dkinfo);
   bRC run_container_volume_cmd(bpContext* ctx, const char *cmd, POOLMEM *volname, int jobid);
   bRC run_container_volume_save(bpContext* ctx, POOLMEM *volname, int jobid);
   bRC run_container_volume_load(bpContext* ctx, POOLMEM *volname, int jobid);
   bool check_for_docker_errors(bpContext* ctx, char *buf);
   inline void render_imagesave_name(POOL_MEM &out, DKINFO *dkinfo, int jobid)
      { Mmsg(out, "%s/%s/%d:backup", dkinfo->get_container_names(),
            dkinfo->get_container_id()->digest_short(), jobid); };
   void dump_robjdebug(bpContext *ctx, restore_object_pkt *rop);
};

#endif   /* _DKCOMMCTX_H_ */