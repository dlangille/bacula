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

#include "docker-fd.h"
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <libgen.h>

/*
 * libbac uses its own sscanf implementation which is not compatible with
 * libc implementation, unfortunately.
 * use bsscanf for Bacula sscanf flavor
 */
#ifdef sscanf
#undef sscanf
#endif

extern DLL_IMP_EXP int64_t       debug_level;

/* Forward referenced functions */
static bRC newPlugin(bpContext *ctx);
static bRC freePlugin(bpContext *ctx);
static bRC getPluginValue(bpContext *ctx, pVariable var, void *value);
static bRC setPluginValue(bpContext *ctx, pVariable var, void *value);
static bRC handlePluginEvent(bpContext *ctx, bEvent *event, void *value);
static bRC startBackupFile(bpContext *ctx, struct save_pkt *sp);
static bRC endBackupFile(bpContext *ctx);
static bRC pluginIO(bpContext *ctx, struct io_pkt *io);
static bRC startRestoreFile(bpContext *ctx, const char *cmd);
static bRC endRestoreFile(bpContext *ctx);
static bRC createFile(bpContext *ctx, struct restore_pkt *rp);
static bRC setFileAttributes(bpContext *ctx, struct restore_pkt *rp);
// Not used! static bRC checkFile(bpContext *ctx, char *fname);
static bRC handleXACLdata(bpContext *ctx, struct xacl_pkt *xacl);

/* Pointers to Bacula functions */
bFuncs *bfuncs = NULL;
bInfo *binfo = NULL;

static pFuncs pluginFuncs = {
   sizeof(pluginFuncs),
   FD_PLUGIN_INTERFACE_VERSION,

   /* Entry points into plugin */
   newPlugin,
   freePlugin,
   getPluginValue,
   setPluginValue,
   handlePluginEvent,
   startBackupFile,
   endBackupFile,
   startRestoreFile,
   endRestoreFile,
   pluginIO,
   createFile,
   setFileAttributes,
   NULL,
   handleXACLdata
};

#ifdef __cplusplus
extern "C" {
#endif

/* Plugin Information structure */
static pInfo pluginInfo = {
   sizeof(pluginInfo),
   FD_PLUGIN_INTERFACE_VERSION,
   FD_PLUGIN_MAGIC,
   DOCKER_LICENSE,
   DOCKER_AUTHOR,
   DOCKER_DATE,
   DOCKER_VERSION,
   DOCKER_DESCRIPTION,
};

/*
 * Plugin called here when it is first loaded.
 */
bRC DLL_IMP_EXP loadPlugin(bInfo *lbinfo, bFuncs *lbfuncs, pInfo ** pinfo, pFuncs ** pfuncs)
{
   bfuncs = lbfuncs;               /* set Bacula function pointers */
   binfo = lbinfo;

   Dmsg2(DINFO, PLUGINNAME " Plugin version %s %s (c) 2019 by Inteos\n",
      DOCKER_VERSION, DOCKER_DATE);

   *pinfo = &pluginInfo;           /* return pointer to our info */
   *pfuncs = &pluginFuncs;         /* return pointer to our functions */

   if (access(DOCKER_CMD, X_OK) < 0){
      berrno be;
      bfuncs->DebugMessage(NULL, __FILE__, __LINE__, DERROR,
         PLUGINPREFIX " Unable to use command tool: %s Err=%s\n", DOCKER_CMD,
         be.bstrerror());
      return bRC_Error;
   }

   return bRC_OK;
}

/*
 * Plugin called here when it is unloaded, normally when Bacula is going to exit.
 */
bRC DLL_IMP_EXP unloadPlugin()
{
   return bRC_OK;
}

#ifdef __cplusplus
}
#endif

/*
 * Main DOCKER Plugin class constructor.
 *  Initializes all variables required.
 */
DOCKER::DOCKER(bpContext *bpctx) :
      mode(DOCKER_NONE),
      JobId(0),
      JobName(NULL),
      since(0),
      where(NULL),
      regexwhere(NULL),
      replace(0),
      robjsent(false),
      estimate(false),
      accurate_warning(false),
      local_restore(false),
      backup_finish(false),
      unsupportedlevel(false),
      param_notrunc(false),
      errortar(false),
      volumewarning(false),
      dockerworkclear(0),
      dkcommctx(NULL),
      commandlist(NULL),
      fname(NULL),
      lname(NULL),
      dkfd(0),
      robjbuf(NULL),
      currdkinfo(NULL),
      restoredkinfo(NULL),
      listing_mode(DOCKER_LISTING_NONE),
      listing_objnr(0),
      parser(NULL),
      workingdir(NULL)
{
   /* TODO: we have a ctx variable stored internally, decide if we use it
    * for every method or rip it off as not required in our code */
   ctx = bpctx;
}

/*
 * Main DOCKER Plugin class destructor, handles variable release on delete.
 *
 * in: none
 * out: freed internal variables and class allocated during job execution
 */
DOCKER::~DOCKER()
{
   /* free standard variables */
   free_and_null_pool_memory(fname);
   free_and_null_pool_memory(lname);
   free_and_null_pool_memory(robjbuf);
   free_and_null_pool_memory(workingdir);
   /* free backend contexts */
   if (commandlist){
      /* free all backend contexts */
      foreach_alist(dkcommctx, commandlist){
         delete dkcommctx;
      }
      delete commandlist;
   }
   if (parser){
      delete parser;
   }
   if (restoredkinfo){
      delete restoredkinfo;
   }
}

/*
 * sets runtime workingdir variable used in working volume creation.
 *
 * in:
 *    workdir - the file daemon working directory parameter
 * out:
 *    none
 */
void DOCKER::setworkingdir(char* workdir)
{
   if (workingdir == NULL){
      /* not allocated yet */
      workingdir = get_pool_memory(PM_FNAME);
   }
   pm_strcpy(&workingdir, workdir);
   DMSG1(NULL, DVDEBUG, "workingdir: %s\n", workingdir);
};

/*
 * Parse a Restore Object saved during backup and modified by user during restore.
 *    Every RO received will allocate a dedicated command context which is used
 *    by bEventRestoreCommand to handle default parameters for restore.
 *
 * in:
 *    bpContext - Bacula Plugin context structure
 *    rop - a restore object structure to parse
 * out:
 *    bRC_OK - on success
 *    bRC_Error - on error
 */
bRC DOCKER::parse_plugin_restoreobj(bpContext *ctx, restore_object_pkt *rop)
{
   if (!rop){
      return bRC_OK;    /* end of rop list */
   }

   if (bstrcmp(rop->object_name, INI_RESTORE_OBJECT_NAME)){
      /* we have a single RO for every command */
      switch_commandctx(ctx, rop->plugin_name);
      /* all restore parameters are DKCOMMCTX specific, so forward parsing to it */
      return dkcommctx->parse_restoreobj(ctx, rop);
   }

   return bRC_OK;
}

/*
 * Parsing a plugin command.
 *    Plugin command e.g. plugin = <plugin-name>:[parameters [parameters]...]
 *
 * in:
 *    bpContext - Bacula Plugin context structure
 *    command - plugin command string to parse
 * out:
 *    bRC_OK - on success
 *    bRC_Error - on error
 */
bRC DOCKER::parse_plugin_command(bpContext *ctx, const char *command)
{
   int i, a;
   bRC status;

   DMSG(ctx, DINFO, "Parse command: %s\n", command);
   /* allocate a new parser if required */
   if (parser == NULL){
      parser = new cmd_parser();
   }

   /* and parse command */
   if (parser->parse_cmd(command) != bRC_OK) {
      DMSG0(ctx, DERROR, "Unable to parse Plugin command line.\n");
      JMSG0(ctx, M_FATAL, "Unable to parse Plugin command line.\n");
      return bRC_Error;
   }

   /* switch dkcommctx to the required context or allocate a new context */
   switch_commandctx(ctx, command);

   /* the first (zero) parameter is a plugin name, we should skip it */
   for (i = 1; i < parser->argc; i++) {
      /* loop over all parsed parameters */
      if (estimate && bstrcmp(parser->argk[i], "listing")){
         /* we have a listing parameter which for estimate means .ls command */
         listing_objnr = 1;
         listing_mode = DOCKER_LISTING_TOP;
         a = 0;
         while (docker_objects[a].name){
            if (bstrcmp(parser->argv[i], docker_objects[a].name) ||
                (*parser->argv[i] == '/' && bstrcmp(parser->argv[i]+1, docker_objects[a].name))){
               listing_mode = docker_objects[a].mode;
               break;
            }
            a++;
         }
         continue;
      }
      if (estimate && bstrcmp(parser->argk[i], "notrunc")){
         /* we are doing estimate and user requested notrunc in display */
         param_notrunc = true;
         continue;
      }
      /* handle it with dkcommctx */
      status = dkcommctx->parse_parameters(ctx, parser->argk[i], parser->argv[i]);
      switch (status){
         case bRC_OK:
            /* the parameter was handled by dkcommctx, proceed to the next */
            continue;
         case bRC_Error:
            /* parsing returned error, raise it up */
            return bRC_Error;
         default:
            break;
      }
      DMSG(ctx, DERROR, "Unknown parameter: %s\n", parser->argk[i]);
      JMSG(ctx, M_ERROR, "Unknown parameter: %s\n", parser->argk[i]);
   }
   return bRC_OK;
}

/*
 * Allocate and initialize new command context list at commandlist.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    command - a Plugin command for a job as a first backend context
 * out:
 *    New backend contexts list at commandlist allocated and initialized.
 *    The only error we can get here is out of memory error, handled internally
 *    by Bacula itself.
 */
void DOCKER::new_commandctx(bpContext *ctx, const char *command)
{
   /* our new command context */
   dkcommctx = New(DKCOMMCTX(command));
   /* add command context to our list */
   commandlist->append(dkcommctx);
   DMSG(ctx, DINFO, "Command context allocated for: %s\n", command);
   /* setup runtime workingdir */
   dkcommctx->setworkingdir(workingdir);
}

/*
 * The function manages the command contexts list.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    command - a Plugin command for a job as a first backend context
 * out:
 *    this.dkcommctx - the DKCOMMCTX allocated/switched for command
 */
void DOCKER::switch_commandctx(bpContext *ctx, const char *command)
{
   DKCOMMCTX *dkctx;

   if (commandlist == NULL){
      /* new command list required, we assumed 8 command contexts at start, should be sufficient */
      commandlist = New(alist(8, not_owned_by_alist));
      /* our first command context */
      new_commandctx(ctx, command);
   } else {
      /* command list available, so search for already allocated context */
      foreach_alist(dkctx, commandlist){
         if (bstrcmp(dkctx->command, command)){
            /* found, set dkcommctx to it and return */
            dkcommctx = dkctx;
            DMSG(ctx, DINFO, "Command context switched to: %s\n", command);
            return;
         }
      }
      /* well, command context not found, so allocate a new one */
      new_commandctx(ctx, command);
   }
}

/*
 * Prepares a single Plugin command for backup or estimate job.
 *  Make a preparation by parse a plugin command and check the
 *  backup/estimate/listing mode and make a proper dkcommctx initialization.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    command - a Plugin command to prepare
 * out:
 *    bRC_OK - when preparation was successful
 *    bRC_Error - on any error
 */
bRC DOCKER::prepare_bejob(bpContext* ctx, char *command)
{
   /* check if it is our Plugin command */
   if (isourplugincommand(PLUGINPREFIX, command)){
      /* first, parse backup command */
      if (parse_plugin_command(ctx, command) != bRC_OK){
         return bRC_Error;
      }

      switch (listing_mode){
         case DOCKER_LISTING_NONE:
            /* other will prepare backup job in dkcommctx context */
            return dkcommctx->prepare_bejob(ctx, estimate);
         case DOCKER_LISTING_CONTAINER:
            /* listing require all */
            if (!dkcommctx->get_all_containers(ctx)){
               return bRC_Error;
            }
            dkcommctx->set_all_containers_to_backup(ctx);
            break;
         case DOCKER_LISTING_IMAGE:
            if (!dkcommctx->get_all_images(ctx)){
               return bRC_Error;
            }
            dkcommctx->set_all_images_to_backup(ctx);
            break;
         case DOCKER_LISTING_VOLUME:
            if (!dkcommctx->get_all_volumes(ctx)){
               return bRC_Error;
            }
            dkcommctx->set_all_volumes_to_backup(ctx);
            break;
         default:
            break;
      }
   }

   return bRC_OK;
}

/*
 * Prepares a single Plugin command for backup.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    command - a Plugin command to prepare
 * out:
 *    bRC_OK - when preparation was successful
 *    bRC_Error - on any error
 */
bRC DOCKER::prepare_backup(bpContext* ctx, char *command)
{
   estimate = false;
   if (prepare_bejob(ctx, command) != bRC_OK){
      return bRC_Error;
   }
   return bRC_OK;
}

/*
 * Prepares a single Plugin command for estimate/listing.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    command - a Plugin command to prepare
 * out:
 *    bRC_OK - when preparation was successful
 *    bRC_Error - on any error
 */
bRC DOCKER::prepare_estimate(bpContext* ctx, char *command)
{
   estimate = true;
   if (prepare_bejob(ctx, command) != bRC_OK){
      return bRC_Error;
   }
   dkcommctx->clear_abort_on_error();
   return bRC_OK;
}

/*
 * Prepares a single Plugin command for restore.
 *  Make a preparation by parse a plugin command and make a proper dkcommctx
 *  initialization.
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    command - a Plugin command to prepare
 * out:
 *    bRC_OK - when preparation was successful
 *    bRC_Error - on any error
 */
bRC DOCKER::prepare_restore(bpContext* ctx, char *command)
{
   /* check if it is our Plugin command */
   if (isourplugincommand(PLUGINPREFIX, command)){
      /* first, parse backup command */
      if (parse_plugin_command(ctx, command) != bRC_OK){
         return bRC_Error;
      }

      /* prepare restore */
      return dkcommctx->prepare_restore(ctx);
   }
   return bRC_OK;
}

/*
 * This is the main method for handling events generated by Bacula.
 *    The behavior of the method depends on event type generated, but there are
 *    some events which does nothing, just return with bRC_OK. Every event is
 *    tracked in debug trace file to verify the event flow during development.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    event - a Bacula event structure
 *    value - optional event value
 * out:
 *    bRC_OK - in most cases signal success/no error
 *    bRC_Error - in most cases signal error
 *    <other> - depend on Bacula Plugin API if applied
 */
bRC DOCKER::handlePluginEvent(bpContext *ctx, bEvent *event, void *value)
{
   switch (event->eventType) {
   case bEventJobStart:
      DMSG_EVENT_STR(event, value);
      getBaculaVar(bVarJobId, (void *)&JobId);
      getBaculaVar(bVarJobName, (void *)&JobName);
      break;

   case bEventJobEnd:
      DMSG_EVENT_STR(event, value);
      if (dockerworkclear == 1){
         dkcommctx->clean_working_volume(ctx);
         dockerworkclear = 0;
      }
      break;

   case bEventLevel:
      char lvl;
      lvl = (char)((intptr_t) value & 0xff);
      DMSG_EVENT_CHAR(event, lvl);
      /* the plugin support a FULL backup only as Docker does not support other levels */
      mode = DOCKER_BACKUP_FULL;
      if (lvl != 'F'){
         unsupportedlevel = true;
      }
      break;

   case bEventSince:
      since = (time_t) value;
      DMSG_EVENT_LONG(event, since);
      break;

   case bEventStartBackupJob:
      DMSG_EVENT_STR(event, value);
      break;

   case bEventEndBackupJob:
      DMSG_EVENT_STR(event, value);
      break;

   case bEventStartRestoreJob:
      DMSG_EVENT_STR(event, value);
      getBaculaVar(bVarWhere, &where);
      DMSG(ctx, DINFO, "Where=%s\n", NPRT(where));
      getBaculaVar(bVarReplace, &replace);
      DMSG(ctx, DINFO, "Replace=%c\n", replace);
      mode = DOCKER_RESTORE;
      break;

   case bEventEndRestoreJob:
      DMSG_EVENT_STR(event, value);
      break;

   /* Plugin command e.g. plugin = <plugin-name>:parameters */
   case bEventEstimateCommand:
      DMSG_EVENT_STR(event, value);
      estimate = true;
      free_and_null_pool_memory(fname);
      return prepare_estimate(ctx, (char*) value);

   /* Plugin command e.g. plugin = <plugin-name>:parameters */
   case bEventBackupCommand:
      DMSG_EVENT_STR(event, value);
      robjsent = false;
      free_and_null_pool_memory(fname);
      return prepare_backup(ctx, (char*)value);

   /* Plugin command e.g. plugin = <plugin-name>:parameters */
   case bEventRestoreCommand:
      DMSG_EVENT_STR(event, value);
      getBaculaVar(bVarRegexWhere, &regexwhere);
      DMSG(ctx, DINFO, "RegexWhere=%s\n", NPRT(regexwhere));
      if (regexwhere){
         /* the plugin cannot support regexwhere, so raise the error */
         DMSG0(ctx, DERROR, "Cannot support RegexWhere restore parameter. Aborting Job.\n");
         JMSG0(ctx, M_FATAL, "Cannot support RegexWhere restore parameter. Aborting Job.\n");
         return bRC_Error;
      }
      return prepare_restore(ctx, (char*)value);

   /* Plugin command e.g. plugin = <plugin-name>:parameters */
   case bEventPluginCommand:
      DMSG_EVENT_STR(event, value);
      if (isourplugincommand(PLUGINPREFIX, (char*)value)){
         // Check supported level
         if (unsupportedlevel){
            DMSG0(ctx, DERROR, "Unsupported backup level. Doing FULL backup.\n");
            JMSG0(ctx, M_ERROR, "Unsupported backup level. Doing FULL backup.\n");
            /* single error message is enough */
            unsupportedlevel = false;
         }

         // check accurate mode backup
         int accurate;
         getBaculaVar(bVarAccurate, &accurate);
         DMSG(ctx, DINFO, "Accurate=%d\n", accurate);
         if (accurate > 0 && !accurate_warning){
            DMSG0(ctx, DERROR, "Accurate mode is not supported. Please disable Accurate mode for this job.\n");
            JMSG0(ctx, M_WARNING, "Accurate mode is not supported. Please disable Accurate mode for this job.\n");
            /* single error message is enough */
            accurate_warning = true;
         }
      }
      break;

   case bEventOptionPlugin:
   case bEventHandleBackupFile:
      if (isourplugincommand(PLUGINPREFIX, (char*)value)){
         DMSG0(ctx, DERROR, "Invalid handle Option Plugin called!\n");
         JMSG0(ctx, M_FATAL,
               "The " PLUGINNAME " plugin doesn't support the Option Plugin configuration.\n"
               "Please review your FileSet and move the Plugin=" PLUGINPREFIX
               "... command into the Include {} block.\n");
         return bRC_Error;
      }
      break;

   case bEventEndFileSet:
      DMSG_EVENT_STR(event, value);
      break;

   case bEventRestoreObject:
      /* Restore Object handle - a plugin configuration for restore and user supplied parameters */
      if (!value){
         DMSG0(ctx, DINFO, "End restore objects.\n");
         break;
      }
      DMSG_EVENT_PTR(event, value);
      return parse_plugin_restoreobj(ctx, (restore_object_pkt *) value);

   default:
      // enabled only for Debug
      DMSG2(ctx, D2, "Unknown event: %s (%d) \n", eventtype2str(event), event->eventType);
   }

   return bRC_OK;
}

/*
 * Reads a data from command tool on backup.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 */
bRC DOCKER::perform_read_data(bpContext *ctx, struct io_pkt *io)
{
   int rc;

   if (dkcommctx->is_eod()){
      /* TODO: we signal EOD as rc=0, so no need to explicity check for EOD, right? */
      io->status = 0;
   } else {
      rc = dkcommctx->read_data(ctx, io->buf, io->count);
      io->status = rc;
      if (rc < 0){
         io->io_errno = EIO;
         return bRC_Error;
      }
   }
   return bRC_OK;
}

/*
 * Reads a data from command tool on backup.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 */
bRC DOCKER::perform_read_volume_data(bpContext *ctx, struct io_pkt *io)
{
   io->status = read(dkfd, io->buf, io->count);
   if (io->status < 0){
      io->io_errno = errno;
      return bRC_Error;
   }
   return bRC_OK;
}

/*
 * Writes data to command tool on restore.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 */
bRC DOCKER::perform_write_data(bpContext *ctx, struct io_pkt *io)
{
   int rc = 0;

   if (dkfd){
      rc = write(dkfd, io->buf, io->count);
   } else {
      rc = dkcommctx->write_data(ctx, io->buf, io->count);
   }
   io->status = rc;
   if (rc < 0){
      io->io_errno = EIO;
      return bRC_Error;
   }
   return bRC_OK;
}

/*
 * Execute a backup command.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 *    io->status, io->io_errno - set to error on any error
 */
bRC DOCKER::perform_backup_open(bpContext *ctx, struct io_pkt *io)
{
   POOL_MEM wname(PM_FNAME);
   struct stat statp;
   btimer_t *timer;

   DMSG1(ctx, DDEBUG, "perform_backup_open called: %s\n", io->fname);
   /* prepare backup for DOCKER_VOLUME */
   if (currdkinfo->type() == DOCKER_VOLUME){
      if (dkcommctx->prepare_working_volume(ctx, JobId) != bRC_OK){
         io->status = -1;
         io->io_errno = EIO;
         return bRC_Error;
      }
      dkcommctx->render_working_volume_filename(wname, BACULACONTAINERFOUT);
      if (stat(wname.c_str(), &statp) != 0){
         berrno be;
         /* if the path does not exist then create one */
         if (be.code() != ENOENT || mkfifo(wname.c_str(), 0600) != 0){
            /* error creating named pipe */
            berrno be;
            io->status = -1;
            io->io_errno = be.code();
            dkcommctx->set_error();
            DMSG2(ctx, DERROR, "cannot create file: %s Err=%s\n", wname.c_str(), be.bstrerror());
            JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                  "Cannot create file: %s Err=%s\n", wname.c_str(), be.bstrerror());
            return bRC_Error;
         }
      } else {
         /* check if it is a proper file */
         if (!S_ISFIFO(statp.st_mode)){
            /* not fifo, not good */
            DMSG2(ctx, DERROR, "file is not fifo: %s [%o]\n", wname.c_str(), statp.st_mode);
            JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                  "Improper file type: %s [%o]\n", wname.c_str(), statp.st_mode);
            return bRC_Error;
         }
      }
   }

   /* execute backup docker */
   if (dkcommctx->backup_docker(ctx, currdkinfo, JobId) != bRC_OK){
      io->status = -1;
      io->io_errno = EIO;
      if (dkcommctx->is_abort_on_error()){
         /* abort_on_error set, so terminate other backup for other container */
         dkcommctx->finish_backup_list(ctx);
      }
      return bRC_Error;
   }

   /* finish preparation for DOCKER_VOLUME */
   if (currdkinfo->type() == DOCKER_VOLUME){
      timer = start_thread_timer(NULL, pthread_self(), dkcommctx->timeout());
      dkfd = open(wname.c_str(), O_RDONLY);
      stop_thread_timer(timer);
      if (dkfd < 0){
         /* error opening file to read */
         berrno be;
         io->status = -1;
         io->io_errno = be.code();
         dkcommctx->set_error();
         DMSG2(ctx, DERROR, "cannot open archive file: %s Err=%s\n", wname.c_str(), be.bstrerror());
         JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
               "Cannot open archive file: %s Err=%s\n", wname.c_str(), be.bstrerror());
         return bRC_Error;
      }
      mode = DOCKER_BACKUP_VOLUME_FULL;
   }

   dkcommctx->clear_eod();

   return bRC_OK;
}

/*
 * Perform a restore file creation and open when restore to local server or
 *  restore command execution when restore to Docker.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 *    io->status, io->io_errno - set to error on any error
 */
bRC DOCKER::perform_restore_open(bpContext* ctx, io_pkt* io)
{
   POOL_MEM wname(PM_FNAME);
   int status;
   btimer_t *timer;

   /* first local restore as a simpler case */
   if (local_restore){
      /* restore local */
      dkfd = open(fname, O_CREAT|O_WRONLY, 0640);
      if (dkfd < 0){
         /* error opening file to write */
         io->status = -1;
         io->io_errno = errno;
         return bRC_Error;
      }
   } else {
      /* prepare restore for DOCKER_VOLUME */
      if (restoredkinfo->type() == DOCKER_VOLUME){
         if (dkcommctx->prepare_working_volume(ctx, JobId) != bRC_OK){
            io->status = -1;
            io->io_errno = EIO;
            return bRC_Error;
         }
         dkcommctx->render_working_volume_filename(wname, BACULACONTAINERFIN);
         status = mkfifo(wname.c_str(), 0600);
         if (status < 0){
            /* error creating named pipe */
            berrno be;
            io->status = -1;
            io->io_errno = be.code();
            dkcommctx->set_error();
            DMSG2(ctx, DERROR, "cannot create file: %s Err=%s\n", wname.c_str(), be.bstrerror());
            JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                  "Cannot create file: %s Err=%s\n", wname.c_str(), be.bstrerror());
            return bRC_Error;
         }
      }

      /* execute backup docker */
      if (dkcommctx->restore_docker(ctx, restoredkinfo, JobId) != bRC_OK){
         io->status = -1;
         io->io_errno = EIO;
         return bRC_Error;
      }

      /* finish preparation for DOCKER_VOLUME */
      if (restoredkinfo->type() == DOCKER_VOLUME){
         timer = start_thread_timer(NULL, pthread_self(), dkcommctx->timeout());
         dkfd = open(wname.c_str(), O_WRONLY);
         stop_thread_timer(timer);
         if (dkfd < 0){
            /* error opening file to write */
            berrno be;
            io->status = -1;
            io->io_errno = be.code();
            dkcommctx->set_error();
            DMSG2(ctx, DERROR, "cannot open archive file: %s Err=%s\n", wname.c_str(), be.bstrerror());
            JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                  "Cannot open archive file: %s Err=%s\n", wname.c_str(), be.bstrerror());
            return bRC_Error;
         }
         mode = DOCKER_RESTORE_VOLUME;
      }

      dkcommctx->clear_eod();
   }

   return bRC_OK;
}

/*
 * Perform command tool termination when backup finish.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 */
bRC DOCKER::perform_backup_close(bpContext *ctx, struct io_pkt *io)
{
   bRC status = bRC_OK;

   dkcommctx->terminate(ctx);
   if (currdkinfo->type() == DOCKER_VOLUME){
      if (close(dkfd) < 0){
         io->status = -1;
         io->io_errno = errno;
         status = bRC_Error;
      }
      mode = DOCKER_BACKUP_FULL;
      errortar = check_container_tar_error(ctx, currdkinfo->get_volume_name());
   }
   return status;
}

/*
 * Perform a restore file close when restore to local server or wait for restore
 *  command finish execution when restore to Docker.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 */
bRC DOCKER::perform_restore_close(bpContext *ctx, struct io_pkt *io)
{
   bRC status = bRC_OK;
   DKID dkid;
   POOL_MEM buf(PM_NAME);
   POOL_MEM names(PM_NAME);

   /* both local_restore and volume restore uses dkfd */
   if (dkfd > 0){
      if (close(dkfd) < 0){
         io->status = -1;
         io->io_errno = errno;
         status = bRC_Error;
      }
      dkfd = 0;
      if (mode == DOCKER_RESTORE_VOLUME && restoredkinfo && restoredkinfo->type() == DOCKER_VOLUME){
         mode = DOCKER_RESTORE;
         errortar = check_container_tar_error(ctx, restoredkinfo->get_volume_name());
      }
   } else {
      status = dkcommctx->wait_for_restore(ctx, dkid);
      if (status != bRC_OK){
         io->status = -1;
         io->io_errno = EIO;
      } else {
         switch (restoredkinfo->type()){
            case DOCKER_IMAGE:
               /* when restore image then rename it only */
               status = dkcommctx->docker_tag(ctx, dkid, restoredkinfo->get_image_repository_tag());
               break;
            case DOCKER_CONTAINER:
               /* on container image we need to create a container itself, first tag the restored image */
               Mmsg(buf, "%s/%s/%d:restore", restoredkinfo->name(), restoredkinfo->id()->digest_short(), JobId);
               status = dkcommctx->docker_tag(ctx, dkid, buf.c_str());
               if (status != bRC_OK){
                  DMSG1(ctx, DERROR, "perform_restore_close cannot tag restored image: %s\n", buf.c_str());
                  JMSG1(ctx, M_ERROR, "perform_restore_close cannot tag restored image: %s\n", buf.c_str());
                  break;
               }
               /* update image information on restoring container */
               restoredkinfo->set_container_imagesave(dkid);
               restoredkinfo->set_container_imagesave_tag(buf);
               /* update a container name */
               pm_strcpy(names, restoredkinfo->get_container_names());
               Mmsg(buf, "%s_%d", names.c_str(), JobId);
               restoredkinfo->set_container_names(buf);
               status = dkcommctx->docker_create_run_container(ctx, restoredkinfo);
               if (status != bRC_OK){
                  DMSG1(ctx, DERROR, "perform_restore_close cannot create container: %s\n",
                        restoredkinfo->get_container_names());
                  JMSG1(ctx, M_ERROR, "perform_restore_close cannot create container: %s\n",
                        restoredkinfo->get_container_names());
                  break;
               }
               break;
            case DOCKER_VOLUME:
               /* XXX */
               break;
         }
      }
   }
   return status;
}

/*
 * This is to check how Bacula archive container finish its job.
 * We are doing this by examining docker.err file contents.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 * out:
 *    false - when no errors found
 *    true - errors found and reported to user
 */
bool DOCKER::check_container_tar_error(bpContext* ctx, char *volname)
{
   struct stat statp;
   POOL_MEM flog(PM_FNAME);
   int rc;

   if (dockerworkclear == 0){
      dockerworkclear = 1;
   }
   dkcommctx->render_working_volume_filename(flog, BACULACONTAINERERRLOG);
   if (stat(flog.c_str(), &statp) == 0){
      if (statp.st_size > 0){
         /* the error file has some content, so archive command was unsuccessful, report it */
         POOL_MEM errlog(PM_MESSAGE);
         int fd;
         char *p;

         fd = open(flog.c_str(), O_RDONLY);
         if (fd < 0){
            /* error opening errorlog, strange */
            berrno be;
            DMSG2(ctx, DERROR, "error opening archive errorlog file: %s Err=%s\n",
                  flog.c_str(), be.bstrerror());
            JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                     "Error opening archive errorlog file: %s Err=%s\n", flog.c_str(), be.bstrerror());
            return true;
         }
         rc = read(fd, errlog.c_str(), errlog.size() - 1);
         close(fd);
         if (rc < 0){
            /* we should read some data, right? */
            berrno be;
            DMSG2(ctx, DERROR, "error reading archive errorlog file: %s Err=%s\n",
                  flog.c_str(), be.bstrerror());
            JMSG2(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                  "Error reading archive errorlog file: %s Err=%s\n", flog.c_str(), be.bstrerror());
            return true;
         }
         /* clear last newline */
         p = errlog.c_str();
         if (p[rc-1] == '\n')
            p[rc-1] = 0;
         /* display error to user */
         DMSG1(ctx, DERROR, "errorlog: %s\n", errlog.c_str());
         JMSG1(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
               "Archive error: %s\n", errlog.c_str());
         /* rename log files for future use */
         if (debug_level > 200){
            POOL_MEM nflog(PM_FNAME);
            dockerworkclear = 2;
            Mmsg(nflog, "%s.%s", flog.c_str(), volname);
            rc = rename(flog.c_str(), nflog.c_str());
            if (rc < 0){
               /* error renaming, report */
               berrno be;
               DMSG2(ctx, DERROR, "error renaming archive errorlog to: %s Err=%s\n",
                     nflog.c_str(), be.bstrerror());
               JMSG2(ctx, M_ERROR,
                     "Error renaming archive errorlog file to: %s Err=%s\n", nflog.c_str(), be.bstrerror());
            }
            dkcommctx->render_working_volume_filename(flog, BACULACONTAINERARCHLOG);
            Mmsg(nflog, "%s.%s", flog.c_str(), volname);
            rc = rename(flog.c_str(), nflog.c_str());
            if (rc < 0){
               /* error renaming, report */
               berrno be;
               DMSG2(ctx, DERROR, "error renaming archive log to: %s Err=%s\n",
                     nflog.c_str(), be.bstrerror());
               JMSG2(ctx, M_ERROR,
                     "Error renaming archive log file to: %s Err=%s\n", nflog.c_str(), be.bstrerror());
            }
         }
         return true;
      }
   } else {
      /* error access to BACULACONTAINERERRLOG, strange, report it */
      berrno be;
      DMSG2(ctx, DERROR, "error access archive errorlog file: %s Err=%s\n", flog.c_str(), be.bstrerror());
      JMSG2(ctx, M_ERROR, "Error access archive errorlog file: %s Err=%s\n", flog.c_str(), be.bstrerror());
   }

   return false;
};

/*
 * Handle Bacula Plugin I/O API for backend
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    io - Bacula Plugin API I/O structure for I/O operations
 * out:
 *    bRC_OK - when successful
 *    bRC_Error - on any error
 *    io->status, io->io_errno - correspond to a plugin io operation status
 */
bRC DOCKER::pluginIO(bpContext *ctx, struct io_pkt *io)
{
   static int rw = 0;      // this variable handles single debug message

   /* assume no error from the very beginning */
   io->status = 0;
   io->io_errno = 0;
   switch (io->func) {
      case IO_OPEN:
         DMSG(ctx, D2, "IO_OPEN: (%s)\n", io->fname);
         switch (mode){
            case DOCKER_BACKUP_FULL:
            case DOCKER_BACKUP_INCR:
            case DOCKER_BACKUP_DIFF:
            case DOCKER_BACKUP_VOLUME_FULL:
               return perform_backup_open(ctx, io);
            case DOCKER_RESTORE:
            case DOCKER_RESTORE_VOLUME:
               return perform_restore_open(ctx, io);
            default:
               return bRC_Error;
         }
         break;
      case IO_READ:
         if (!rw) {
            rw = 1;
            DMSG2(ctx, D2, "IO_READ buf=%p len=%d\n", io->buf, io->count);
         }
         switch (mode){
            case DOCKER_BACKUP_FULL:
            case DOCKER_BACKUP_INCR:
            case DOCKER_BACKUP_DIFF:
               return perform_read_data(ctx, io);
            case DOCKER_BACKUP_VOLUME_FULL:
               return perform_read_volume_data(ctx, io);
            default:
               return bRC_Error;
         }
         break;
      case IO_WRITE:
         if (!rw) {
            rw = 1;
            DMSG2(ctx, D2, "IO_WRITE buf=%p len=%d\n", io->buf, io->count);
         }
         switch (mode){
            case DOCKER_RESTORE:
            case DOCKER_RESTORE_VOLUME:
               return perform_write_data(ctx, io);
            default:
               return bRC_Error;
         }
         break;
      case IO_CLOSE:
         DMSG0(ctx, D2, "IO_CLOSE\n");
         rw = 0;
         switch (mode){
            case DOCKER_RESTORE:
            case DOCKER_RESTORE_VOLUME:
               return perform_restore_close(ctx, io);
            case DOCKER_BACKUP_FULL:
            case DOCKER_BACKUP_VOLUME_FULL:
            case DOCKER_BACKUP_INCR:
            case DOCKER_BACKUP_DIFF:
               return perform_backup_close(ctx, io);
            default:
               return bRC_Error;
         }
         break;
   }

   return bRC_OK;
}

/*
 * Unimplemented, always return bRC_OK.
 */
bRC DOCKER::getPluginValue(bpContext *ctx, pVariable var, void *value)
{
   return bRC_OK;
}

/*
 * Unimplemented, always return bRC_OK.
 */
bRC DOCKER::setPluginValue(bpContext *ctx, pVariable var, void *value)
{
   return bRC_OK;
}

/*
 * Get all required information from Docker to populate save_pkt for Bacula.
 *  It handles a Restore Object (FT_PLUGIN_CONFIG) for every backup and
 *  new Plugin Backup Command if setup in FileSet. It handles
 *  backup/estimate/listing modes of operation.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    save_pkt - Bacula Plugin API save packet structure
 * out:
 *    bRC_OK - when save_pkt prepared successfully and we have file to backup
 *    bRC_Max - when no more files to backup
 *    bRC_Error - in any error
 */
bRC DOCKER::startBackupFile(bpContext *ctx, struct save_pkt *sp)
{
   /* handle listing mode if requested */
   if (estimate && listing_mode == DOCKER_LISTING_TOP){
      sp->fname = (char*)docker_objects[listing_objnr++].name;
      sp->type = FT_DIREND;
      sp->statp.st_size = 0;
      sp->statp.st_nlink = 1;
      sp->statp.st_uid = 0;
      sp->statp.st_gid = 0;
      sp->statp.st_mode = 040750;
      sp->statp.st_blksize = 4096;
      sp->statp.st_blocks = 1;
      sp->statp.st_atime = sp->statp.st_mtime = sp->statp.st_ctime = time(NULL);
      return bRC_OK;
   }

   /* The first file in Full backup, is the RestoreObject */
   if (!estimate && mode == DOCKER_BACKUP_FULL && robjsent == false) {
      ConfigFile ini;

      /* robj for the first time, allocate the buffer */
      if (!robjbuf){
         robjbuf = get_pool_memory(PM_FNAME);
      }

      ini.register_items(plugin_items_dump, sizeof(struct ini_items));
      sp->object_name = (char *)INI_RESTORE_OBJECT_NAME;
      sp->object_len = ini.serialize(&robjbuf);
      sp->object = robjbuf;
      sp->type = FT_PLUGIN_CONFIG;
      DMSG0(ctx, DINFO, "Prepared RestoreObject sent.\n");
      return bRC_OK;
   }

   /* check for forced backup finish */
   if (backup_finish){
      DMSG0(ctx, DINFO, "forced backup finish!\n");
      backup_finish = false;
      return bRC_Max;
   }

   /* check if this is the first container to backup/estimate/listing */
   if (currdkinfo == NULL){
      /* set all_to_backup list at first element */
      currdkinfo = dkcommctx->get_first_to_backup(ctx);
      if (!currdkinfo){
         /* no docker objects to backup at all */
         DMSG0(ctx, DDEBUG, "No Docker containers or objects to backup found.\n");
         JMSG0(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
               "No Docker containers or objects to backup found.\n");
         return bRC_Max;
      }
   }

   /* in currdkinfo we have all info about docker object to backup */
   if (!estimate && mode != DOCKER_BACKUP_CONTAINER_VOLLIST){
      if (currdkinfo->type() != DOCKER_VOLUME){
         DMSG3(ctx, DINFO, "Start Backup %s: %s (%s)\n",
               currdkinfo->type_str(), currdkinfo->name(), currdkinfo->id()->digest_short());
         JMSG3(ctx, M_INFO, "Start Backup %s: %s (%s)\n",
               currdkinfo->type_str(), currdkinfo->name(), currdkinfo->id()->digest_short());
      } else {
         DMSG2(ctx, DINFO, "Start Backup %s: %s\n",
               currdkinfo->type_str(), currdkinfo->name());
         JMSG2(ctx, M_INFO, "Start Backup %s: %s\n",
               currdkinfo->type_str(), currdkinfo->name());
      }
   }

   /* generate the filename in backup/estimate */
   if (!fname){
      fname = get_pool_memory(PM_FNAME);
   }
   if (!lname){
      lname = get_pool_memory(PM_FNAME);
   }

   /* populate common statp */
   sp->statp.st_nlink = 1;
   sp->statp.st_uid = 0;
   sp->statp.st_gid = 0;
   sp->portable = true;
   sp->statp.st_blksize = 4096;
   // TODO: use created time of image and volume objects
   sp->statp.st_atime = sp->statp.st_mtime = sp->statp.st_ctime = time(NULL);
   sp->statp.st_mode = S_IFREG | 0640;     // standard file with '-rw-r----' permissions

   if (mode == DOCKER_BACKUP_CONTAINER_VOLLIST && currvols){
      sp->statp.st_size = currvols->vol->size();
      sp->statp.st_blocks = sp->statp.st_size / 4096 + 1;
      sp->type = FT_LNK;
      if (!estimate){
         Mmsg(fname, "%s%s/%s/volume: %s -> %s", PLUGINNAMESPACE, CONTAINERNAMESPACE,
               currdkinfo->name(), currvols->vol->get_volume_name(), currvols->destination);
         *lname = 0;
      } else {
         Mmsg(fname, "%s%s/%s/volume: %s", PLUGINNAMESPACE, CONTAINERNAMESPACE,
               currdkinfo->name(), currvols->vol->get_volume_name());
         lname = currvols->destination;
      }
      sp->link = lname;
      sp->statp.st_mode = S_IFLNK | 0640;
   } else {
      sp->statp.st_size = currdkinfo->size();
      sp->statp.st_blocks = sp->statp.st_size / 4096 + 1;
      sp->type = FT_REG;               // exported archive is a standard file

      switch(listing_mode){
         case DOCKER_LISTING_NONE:
            /* generate a backup/estimate filename */
            switch (currdkinfo->type()){
               case DOCKER_CONTAINER:
                  Mmsg(fname, "%s%s/%s/%s.tar", PLUGINNAMESPACE, CONTAINERNAMESPACE,
                        currdkinfo->name(), (char*)currdkinfo->id());
                  break;
               case DOCKER_IMAGE:
                  Mmsg(fname, "%s%s/%s/%s.tar", PLUGINNAMESPACE, IMAGENAMESPACE,
                        currdkinfo->name(), (char*)currdkinfo->id());
                  break;
               case DOCKER_VOLUME:
                  Mmsg(fname, "%s%s/%s.tar", PLUGINNAMESPACE, VOLUMENAMESPACE,
                        currdkinfo->name());
                  break;
               default:
                  DMSG1(ctx, DERROR, "unknown object type to backup: %s\n", currdkinfo->type_str());
                  JMSG1(ctx, M_ERROR, "Unknown object type to backup: %s\n", currdkinfo->type_str());
                  return bRC_Error;
            }
            break;
         case DOCKER_LISTING_VOLUME:
            sp->statp.st_mode = S_IFBLK | 0640;     // standard block device with 'brw-r----' permissions
            Mmsg(fname, "%s", currdkinfo->name());
            break;
         case DOCKER_LISTING_IMAGE:
            sp->statp.st_mode = S_IFBLK | 0640;     // standard block device with 'brw-r----' permissions
         case DOCKER_LISTING_CONTAINER:
            Mmsg(lname, "%s", param_notrunc?(char*)currdkinfo->id():currdkinfo->id()->digest_short());
            Mmsg(fname, "%s", currdkinfo->name());
            sp->link = lname;
            sp->type = FT_LNK;
            break;
         default:
            /* error */
            break;
      }
   }

   /* populate rest of statp */
   sp->fname = fname;

   return bRC_OK;
}

/*
 * Finish the Docker backup and clean temporary objects.
 *  For estimate/listing modes it handles next object to display.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    save_pkt - Bacula Plugin API save packet structure
 * out:
 *    bRC_OK - when no more files to backup
 *    bRC_More - when Bacula should expect a next file
 *    bRC_Error - in any error
 */
bRC DOCKER::endBackupFile(bpContext *ctx)
{
   if (!estimate && mode != DOCKER_BACKUP_CONTAINER_VOLLIST){
      /* If the current file was the restore object, so just ask for the next file */
      if (mode == DOCKER_BACKUP_FULL && robjsent == false) {
         robjsent = true;
         return bRC_More;
      }
      switch (currdkinfo->type()){
         case DOCKER_CONTAINER:
            /* delete backup commit image */
            if (dkcommctx->delete_container_commit(ctx, currdkinfo, JobId) != bRC_OK){
               /* TODO: report problem to the user but not abort backup */
               return bRC_Error;
            }
         case DOCKER_IMAGE:
            DMSG4(ctx, DINFO, "Backup of %s: %s (%s) %s.\n", currdkinfo->type_str(), currdkinfo->name(),
                  currdkinfo->id()->digest_short(), dkcommctx->is_error() ? "Failed" : "OK");
            JMSG4(ctx, M_INFO, "Backup of %s: %s (%s) %s.\n", currdkinfo->type_str(), currdkinfo->name(),
                  currdkinfo->id()->digest_short(), dkcommctx->is_error() ? "Failed" : "OK");
            break;
         case DOCKER_VOLUME:
            /* check */
            DMSG3(ctx, DINFO, "Backup of %s: %s %s.\n", currdkinfo->type_str(), currdkinfo->name(),
                  dkcommctx->is_error() || errortar ? "Failed" : "OK");
            JMSG3(ctx, M_INFO, "Backup of %s: %s %s.\n", currdkinfo->type_str(), currdkinfo->name(),
                  dkcommctx->is_error() || errortar ? "Failed" : "OK");
            break;
      };
   }

   /* handle listing and next file to backup */
   if (listing_mode == DOCKER_LISTING_TOP){
      /* handle top-level listing mode */
      if (docker_objects[listing_objnr].name){
         /* next object available */
         return bRC_More;
      }
   } else {
      /* check if container we just backup has any vols mounted */
      if (currdkinfo->type() == DOCKER_CONTAINER && !currvols && currdkinfo->container_has_vols() &&
            mode != DOCKER_BACKUP_CONTAINER_VOLLIST){
         /* yes, so prepare the flow for symbolic link backup */
         currvols = currdkinfo->container_first_vols();
         mode = DOCKER_BACKUP_CONTAINER_VOLLIST;
         DMSG0(ctx, DDEBUG, "docker vols to backup found\n");
         return bRC_More;
      }
      /* check if we already in symbolic link backup mode */
      if (mode == DOCKER_BACKUP_CONTAINER_VOLLIST && currvols){
         /* yes, so check for next symbolic link to backup */
         currvols = currdkinfo->container_next_vols();
         if (currvols){
            DMSG0(ctx, DDEBUG, "docker next vols to backup found\n");
            return bRC_More;
         } else {
            /* it was the last symbolic link, so finish this mode */
            mode = DOCKER_BACKUP_FULL;
            currvols = NULL;
         }
      }
      /* check if next object to backup/estimate/listing */
      currdkinfo = dkcommctx->get_next_to_backup(ctx);
      if (currdkinfo){
         DMSG0(ctx, DDEBUG, "next docker object to backup found\n");
         return bRC_More;
      }
   }

   return bRC_OK;
}

/*
 * Start Restore File.
 */
bRC DOCKER::startRestoreFile(bpContext *ctx, const char *cmd)
{
   return bRC_OK;
}

/*
 * End Restore File.
 *    Handles the next vm state.
 */
bRC DOCKER::endRestoreFile(bpContext *ctx)
{
   /* release restore dkinfo */
   if (restoredkinfo){
      delete restoredkinfo;
      restoredkinfo = NULL;
   }
   return bRC_OK;
}

/*
 * Search in Docker all available images if image we are restoring already exist.
 *
 * in:
 *    bpContext - bacula plugin context
 *    this->restoredkinfo - current image to restore
 * out:
 *    *DKINFO from Docker all_images if image to restore found
 *    NULL when not found
 */
DKINFO *DOCKER::search_docker_image(bpContext *ctx)
{
   alist *allimages;
   DKINFO *image = NULL;

   allimages = dkcommctx->get_all_images(ctx);
   if (allimages){
      DMSG1(ctx, DDEBUG, "search allimages for: %s\n", (char*)restoredkinfo->get_image_id());
      /* check if image which we are restoring exist on Docker already */
      foreach_alist(image, allimages){
         DMSG1(ctx, DDEBUG, "compare: %s\n", (char*)image->get_image_id());
         if (image && *image->get_image_id() == *restoredkinfo->get_image_id()){
            DMSG0(ctx, DINFO, "image to restore found available\n");
            break;
         }
      };
   }
   return image;
};

/*
 * Search in Docker all available volumes if volume we are restoring already exist.
 *
 * in:
 *    bpContext - bacula plugin context
 *    this->restoredkinfo - current volume to restore
 * out:
 *    *DKINFO from Docker all_volumes if volume to restore found
 *    NULL when not found
 */
DKINFO *DOCKER::search_docker_volume(bpContext *ctx)
{
   alist *allvolumes;
   DKINFO *volume = NULL;

   allvolumes = dkcommctx->get_all_volumes(ctx);
   if (allvolumes){
      DMSG1(ctx, DDEBUG, "search allvolumes for: %s\n", restoredkinfo->get_volume_name());
      /* check if image which we are restoring exist on Docker already */
      foreach_alist(volume, allvolumes){
         DMSG1(ctx, DDEBUG, "compare: %s\n", volume->get_volume_name());
         if (volume && bstrcmp(volume->get_volume_name(), restoredkinfo->get_volume_name())){
            DMSG0(ctx, DINFO, "volume to restore found available\n");
            break;
         }
      };
   }
   return volume;
};

/*
 * When restore to local server then handle restored file creation else
 *  inform user about starting a restore.
 *
 * in:
 *    bpContext - bacula plugin context
 *    restore_pkt - Bacula Plugin API restore packet structure
 * out:
 *    bRC_OK - when success reported from backend
 *    rp->create_status = CF_EXTRACT - the backend will restore the file
 *                                     with pleasure
 *    rp->create_status = CF_SKIP - the backend wants to skip restoration, i.e.
 *                                  the file already exist and Replace=n was set
 *    bRC_Error, rp->create_status = CF_ERROR - in any error
 */
bRC DOCKER::createFile(bpContext *ctx, struct restore_pkt *rp)
{
   POOL_MEM fmt(PM_FNAME);
   POOL_MEM fmt2(PM_FNAME);
   POOL_MEM imageid(PM_FNAME);
   POOL_MEM label(PM_FNAME);
   struct stat statp;
   char *dir, *p;
   int len;
   int status;
   DKINFO *image;

   /* skip a support volume file link */
   if (rp->type == FT_LNK && S_ISLNK(rp->statp.st_mode)){
      DMSG1(ctx, DDEBUG, "skipping support file: %s\n", rp->ofname);
      rp->create_status = CF_SKIP;
   } else {
      /* it seems something to restore */
      if (!fname){
         fname = get_pool_memory(PM_FNAME);
      }
      /* where=/ then we'll restore to Docker else we'll restore local */
      if (where && strlen(where) > 1 && *where == PathSeparator){
         local_restore = true;
         len = strlen(where);
         DMSG(ctx, DINFO, "local restore to: %s\n", where);
         pm_strcpy(fmt, rp->ofname);
         dir = strrchr(fmt.c_str(), '.');
         if (dir && bstrcmp(dir, ".tar")){
            *dir = 0;
            JMSG(ctx, M_INFO, "Docker local restore: %s\n", fmt.c_str() + len + strlen(PLUGINNAMESPACE) + 1);
         }
         /* compose a destination fname */
         pm_strcpy(fname, where);
         pm_strcat(fname, rp->ofname + len + strlen(PLUGINNAMESPACE));
         DMSG(ctx, DDEBUG, "composed fname: %s\n", fname);
         /* prepare a destination directory */
         pm_strcpy(fmt, fname);
         dir = dirname(fmt.c_str());
         if (!dir){
            berrno be;
            DMSG2(ctx, DERROR, "dirname error for %s Err=%s\n", fmt.c_str(), be.bstrerror());
            JMSG2(ctx, dkcommctx->is_fatal() ? M_FATAL : M_ERROR, "dirname error for %s Err=%s\n", fmt.c_str(), be.bstrerror());
            rp->create_status = CF_ERROR;
            return bRC_Error;
         }
         DMSG(ctx, DDEBUG, "dirname: %s\n", fmt.c_str());
         if (pluglib_mkpath(ctx, dir, dkcommctx->is_fatal()) != bRC_OK){
            rp->create_status = CF_ERROR;
            return bRC_Error;
         }
         switch (replace){
            case REPLACE_ALWAYS:
               rp->create_status = CF_EXTRACT;
               break;
            case REPLACE_NEVER:
               /* check if file exist locally */
               if (stat(fname, &statp) == 0){
                  /* exist, so skip restore */
                  rp->create_status = CF_SKIP;
                  break;
               }
               rp->create_status = CF_EXTRACT;
               break;
            case REPLACE_IFNEWER:
               if (stat(fname, &statp) == 0){
                  /* exist, so check if newer */
                  if (statp.st_mtime < rp->statp.st_mtime){
                     rp->create_status = CF_SKIP;
                     break;
                  }
               }
               rp->create_status = CF_EXTRACT;
               break;
            case REPLACE_IFOLDER:
               if (stat(fname, &statp) == 0){
                  /* exist, so check if newer */
                  if (statp.st_mtime > rp->statp.st_mtime){
                     rp->create_status = CF_SKIP;
                     break;
                  }
               }
               rp->create_status = CF_EXTRACT;
               break;
         }
      } else {
         /* TODO: report docker restore start */
         local_restore = false;
         pm_strcpy(fname, rp->ofname + strlen(PLUGINNAMESPACE));
         DMSG(ctx, DINFO, "scanning fname to restore: %s\n", fname);

         /*
          * first scan for Container backup file
          * the dirtmp variable has a sscanf format to scan which is dynamically generated
          * based on the size of label and imageid variables. this limits the size of the scan
          * and prevents any memory overflow. the destination scan format is something like this:
          * "/container/%256[^/]/%256[^.]", so it will scan two string variables up to
          * 256 characters long
          */
         Mmsg(fmt, "%s/%%%d[^/]/%%%d[^.]", CONTAINERNAMESPACE,
               label.size(), imageid.size());
         // DMSG(ctx, DVDEBUG, "container scan str: %s\n", dirtmp.c_str());
         status = sscanf(fname, fmt.c_str(), label.c_str(), imageid.c_str());
         if (status == 2){
            /* insanity check for memleak */
            if (restoredkinfo != NULL){
               delete restoredkinfo;
            }
            restoredkinfo = New(DKINFO(DOCKER_CONTAINER));
            restoredkinfo->set_container_id(imageid);
            restoredkinfo->set_container_names(label);
            pm_strcpy(fmt, label.c_str());   // Well there is no a pm_strcpy(POOL_MEM&, POOL_MEM&), strange
            Mmsg(label, "%s/%s", fmt.c_str(), restoredkinfo->get_container_id()->digest_short());
            DMSG2(ctx, DINFO, "scanned: %s %s\n", restoredkinfo->get_container_names(),
                  (char*)restoredkinfo->get_container_id());
            /* we replace container always? */
            rp->create_status = CF_EXTRACT;
         } else {
            /*
             * scan for Volume backup file
             * the dirtmp variable has a sscanf format to scan which is dynamically generated
             * based on the size of label variable. this limits the size of the scan
             * and prevents any memory overflow. the destination scan format is something like this:
             * "/volume/%256s", so it will scan a single string variable up to
             * 256 characters long
             */
            Mmsg(fmt, "%s/%%%ds", VOLUMENAMESPACE, label.size());
            // DMSG(ctx, DVDEBUG, "volume scan str: %s\n", dirtmp.c_str());
            status = sscanf(fname, fmt.c_str(), label.c_str());
            if (status == 1){
               /* terminate volume name, so fname without '.tar. */
               p = strstr(label.c_str(), ".tar");
               *p = 0;
               /* insanity check for memleak */
               if (restoredkinfo != NULL){
                  delete restoredkinfo;
               }
               restoredkinfo = New(DKINFO(DOCKER_VOLUME));
               restoredkinfo->set_volume_name(label);
               DMSG1(ctx, DINFO, "scanned: %s\n", restoredkinfo->get_volume_name());

               /* check for remote docker operations as this is not supported currently */
               if (dkcommctx->is_remote_docker()){
                  DMSG1(ctx, DINFO, "volume %s restore with docker_host skipped.\n", restoredkinfo->get_volume_name());
                  if (!volumewarning){
                     JMSG0(ctx, M_WARNING, "Docker Volume restore with docker_host is unsupported! All volumes restore skipped.\n");
                     volumewarning = true;
                  }
                  rp->create_status = CF_SKIP;
                  return bRC_OK;
               }

               switch (replace){
                  case REPLACE_ALWAYS:
                     rp->create_status = CF_EXTRACT;
                     break;
                  case REPLACE_NEVER:
                  case REPLACE_IFNEWER:
                  case REPLACE_IFOLDER:
                  default:
                     /*
                      * check if volume exist on docker,
                      * as we cannot check if the volume was modified
                      * then we will treat it the same as REPLACE_NEVER flag
                      */
                     if ((image = search_docker_volume(ctx)) != NULL){
                        /* exist, so skip restore */
                        DMSG1(ctx, DINFO, "volume exist, skipping restore of: %s\n",
                              restoredkinfo->get_volume_name());
                        JMSG1(ctx, M_INFO, "Volume exist, skipping restore of: %s\n",
                              restoredkinfo->get_volume_name());
                        rp->create_status = CF_SKIP;
                        break;
                     }
                     rp->create_status = CF_EXTRACT;
                     break;
                  }
            } else {
               /* now scan for Image backup */
               p = strrchr(fname, '/');
               if (p){
                  /* found the last (first in reverse) path_separator,
                   * so before $p we have a path and after $p we have digest to scan */
                  *p++ = 0;
               }
               /*
                * scan path to separate image repository:tag data from filename
                * the dirtmp and tmp2 variables have a sscanf format to scan which is dynamically
                * generated based on the size of label and imageid variables. this limits the size
                * of the scan and prevents any memory overflow. the destination scan format is
                * something like this:
                * "/image/%256s", for image repository:tag encoded in filename and
                * "%256[^.]", for imageid part of the encoded filename, so it will scan
                * two string variables in two sscanf up to 256 characters long each
                */
               Mmsg(fmt, "%s/%%%ds", IMAGENAMESPACE, label.size());
               Mmsg(fmt2, "%%%d[^.]", imageid.size());
               // DMSG(ctx, DVDEBUG, "image scan str: %s\n", dirtmp.c_str());
               if (sscanf(fname, fmt.c_str(), label.c_str()) == 1 &&
                     sscanf(p, fmt2.c_str(), imageid.c_str()) == 1){
                  /* insanity check for memleak */
                  if (restoredkinfo != NULL){
                     delete restoredkinfo;
                  }
                  /* we will restore the Docker Image */
                  restoredkinfo = New(DKINFO(DOCKER_IMAGE));
                  restoredkinfo->set_image_id(imageid);
                  restoredkinfo->scan_image_repository_tag(label);
                  DMSG2(ctx, DINFO, "scanned: %s %s\n", restoredkinfo->get_image_repository_tag(),
                        (char*)restoredkinfo->get_image_id());
                  switch (replace){
                     case REPLACE_ALWAYS:
                        rp->create_status = CF_EXTRACT;
                        break;
                     case REPLACE_NEVER:
                        /* check if image exist on docker */
                        if ((image = search_docker_image(ctx)) != NULL){
                           /* exist, so skip restore */
                           DMSG1(ctx, DINFO, "image exist, skipping restore of: %s\n",
                                 restoredkinfo->get_image_repository_tag());
                           JMSG1(ctx, M_INFO, "Image exist, skipping restore of: %s\n",
                                 restoredkinfo->get_image_repository_tag());
                           rp->create_status = CF_SKIP;
                           break;
                        }
                        rp->create_status = CF_EXTRACT;
                        break;
                     case REPLACE_IFNEWER:
                        if ((image = search_docker_image(ctx)) != NULL){
                           /* exist, so check if newer */
                           if (image->get_image_created() < rp->statp.st_mtime){
                              DMSG1(ctx, DINFO, "image exist and is newer, skipping restore of: %s\n",
                                    restoredkinfo->get_image_repository_tag());
                              JMSG1(ctx, M_INFO, "Image exist and is newer, skipping restore of: %s\n",
                                    restoredkinfo->get_image_repository_tag());
                              rp->create_status = CF_SKIP;
                              break;
                           }
                        }
                        rp->create_status = CF_EXTRACT;
                        break;
                     case REPLACE_IFOLDER:
                        if ((image = search_docker_image(ctx)) != NULL){
                           /* exist, so check if newer */
                           if (image->get_image_created() > rp->statp.st_mtime){
                              rp->create_status = CF_SKIP;
                              DMSG1(ctx, DINFO, "image exist and is older, skipping restore of: %s\n",
                                    restoredkinfo->get_image_repository_tag());
                              JMSG1(ctx, M_INFO, "Image exist and is older, skipping restore of: %s\n",
                                    restoredkinfo->get_image_repository_tag());
                              break;
                           }
                        }
                        rp->create_status = CF_EXTRACT;
                        break;
                  }
               } else {
                  // fname scanning error
                  DMSG1(ctx, DERROR, "Filename scan error on: %s\n", fmt.c_str());
                  JMSG1(ctx, dkcommctx->is_abort_on_error() ? M_FATAL : M_ERROR,
                        "Filename scan error on: %s\n", fmt.c_str());
                  rp->create_status = CF_ERROR;
                  return bRC_Error;
               }
            }
         }
         if (rp->create_status == CF_EXTRACT){
            /* display info about a restore to the user */
            DMSG2(ctx, DINFO, "%s restore: %s\n", restoredkinfo-> type_str(), label.c_str());
            JMSG2(ctx, M_INFO, "%s restore: %s\n", restoredkinfo->type_str(), label.c_str());
         }
      }
   }
   return bRC_OK;
}

/*
 * Unimplemented, always return bRC_OK.
 */
bRC DOCKER::setFileAttributes(bpContext *ctx, struct restore_pkt *rp)
{
   return bRC_OK;
}

#if 0
/*
 * Unimplemented, always return bRC_Seen.
 */
bRC DOCKER::checkFile(bpContext *ctx, char *fname)
{
   if (!accurate_warning){
      accurate_warning = true;
      JMSG0(ctx, M_WARNING, "Accurate mode is not supported. Please disable Accurate mode for this job.\n");
   }
   return bRC_Seen;
}
#endif

/*
 * We will not generate any acl/xattr data, always return bRC_OK.
 */
bRC DOCKER::handleXACLdata(bpContext *ctx, struct xacl_pkt *xacl)
{
   return bRC_OK;
}

/*
 * Called here to make a new instance of the plugin -- i.e. when
 * a new Job is started.  There can be multiple instances of
 * each plugin that are running at the same time.  Your
 * plugin instance must be thread safe and keep its own
 * local data.
 */
static bRC newPlugin(bpContext *ctx)
{
   int JobId;
   DOCKER *self = New(DOCKER(ctx));
   char *workdir;

   if (!self){
      return bRC_Error;
   }
   ctx->pContext = (void*) self;

   getBaculaVar(bVarJobId, (void *)&JobId);
   DMSG(ctx, DINFO, "newPlugin JobId=%d\n", JobId);

   /* we are very paranoid here to double check it */
   if (access(DOCKER_CMD, X_OK) < 0){
      berrno be;
      DMSG2(ctx, DERROR, "Unable to use command tool: %s Err=%s\n", DOCKER_CMD, be.bstrerror());
      JMSG2(ctx, M_FATAL, "Unable to use command tool: %s Err=%s\n", DOCKER_CMD, be.bstrerror());
      return bRC_Error;
   }

   /* get dynamic working directory from file daemon */
   getBaculaVar(bVarWorkingDir, (void *)&workdir);
   self->setworkingdir(workdir);

   return bRC_OK;
}

/*
 * Release everything concerning a particular instance of
 *  a plugin. Normally called when the Job terminates.
 */
static bRC freePlugin(bpContext *ctx)
{
   if (!ctx){
      return bRC_Error;
   }
   DOCKER *self = pluginclass(ctx);
   DMSG(ctx, D1, "freePlugin this=%p\n", self);
   if (!self){
      return bRC_Error;
   }
   delete self;
   return bRC_OK;
}

/*
 * Called by core code to get a variable from the plugin.
 *   Not currently used.
 */
static bRC getPluginValue(bpContext *ctx, pVariable var, void *value)
{
   ASSERT_CTX;

   DMSG0(ctx, D3, "getPluginValue called.\n");
   DOCKER *self = pluginclass(ctx);
   return self->getPluginValue(ctx,var, value);
}

/*
 * Called by core code to set a plugin variable.
 *  Not currently used.
 */
static bRC setPluginValue(bpContext *ctx, pVariable var, void *value)
{
   ASSERT_CTX;

   DMSG0(ctx, D3, "setPluginValue called.\n");
   DOCKER *self = pluginclass(ctx);
   return self->setPluginValue(ctx, var, value);
}

/*
 * Called by Bacula when there are certain events that the
 *   plugin might want to know.  The value depends on the
 *   event.
 */
static bRC handlePluginEvent(bpContext *ctx, bEvent *event, void *value)
{
   ASSERT_CTX;

   DMSG(ctx, D1, "handlePluginEvent (%i)\n", event->eventType);
   DOCKER *self = pluginclass(ctx);
   return self->handlePluginEvent(ctx, event, value);
}

/*
 * Called when starting to backup a file. Here the plugin must
 *  return the "stat" packet for the directory/file and provide
 *  certain information so that Bacula knows what the file is.
 *  The plugin can create "Virtual" files by giving them
 *  a name that is not normally found on the file system.
 */
static bRC startBackupFile(bpContext *ctx, struct save_pkt *sp)
{
   ASSERT_CTX;
   if (!sp){
      return bRC_Error;
   }
   DMSG0(ctx, D1, "startBackupFile.\n");
   DOCKER *self = pluginclass(ctx);
   return self->startBackupFile(ctx, sp);
}

/*
 * Done backing up a file.
 */
static bRC endBackupFile(bpContext *ctx)
{
   ASSERT_CTX;

   DMSG0(ctx, D1, "endBackupFile.\n");
   DOCKER *self = pluginclass(ctx);
   return self->endBackupFile(ctx);
}

/*
 * Called when starting restore the file, right after a createFile().
 */
static bRC startRestoreFile(bpContext *ctx, const char *cmd)
{
   ASSERT_CTX;

   DMSG0(ctx, D1, "startRestoreFile.\n");
   DOCKER *self = pluginclass(ctx);
   return self->startRestoreFile(ctx, cmd);
}

/*
 * Done restore the file.
 */
static bRC endRestoreFile(bpContext *ctx)
{
   ASSERT_CTX;

   DMSG0(ctx, D1, "endRestoreFile.\n");
   DOCKER *self = pluginclass(ctx);
   return self->endRestoreFile(ctx);
}

/*
 * Do actual I/O. Bacula calls this after startBackupFile
 *   or after startRestoreFile to do the actual file
 *   input or output.
 */
static bRC pluginIO(bpContext *ctx, struct io_pkt *io)
{
   ASSERT_CTX;

   DMSG0(ctx, DVDEBUG, "pluginIO.\n");
   DOCKER *self = pluginclass(ctx);
   return self->pluginIO(ctx, io);
}

/*
 * Called here to give the plugin the information needed to
 *  re-create the file on a restore.  It basically gets the
 *  stat packet that was created during the backup phase.
 *  This data is what is needed to create the file, but does
 *  not contain actual file data.
 */
static bRC createFile(bpContext *ctx, struct restore_pkt *rp)
{
   ASSERT_CTX;

   DMSG0(ctx, D1, "createFile.\n");
   DOCKER *self = pluginclass(ctx);
   return self->createFile(ctx, rp);
}

#if 0
/*
 * checkFile used for accurate mode backup
 */
static bRC checkFile(bpContext *ctx, char *fname)
{
   ASSERT_CTX;

   DMSG(ctx, D1, "checkFile: %s\n", fname);
   DOCKER *self = pluginclass(ctx);
   return self->checkFile(ctx, fname);
}
#endif

/*
 * Called after the file has been restored. This can be used to
 *  set directory permissions, ...
 */
static bRC setFileAttributes(bpContext *ctx, struct restore_pkt *rp)
{
   ASSERT_CTX;

   DMSG0(ctx, D1, "setFileAttributes.\n");
   DOCKER *self = pluginclass(ctx);
   return self->setFileAttributes(ctx, rp);
}

/*
 * handleXACLdata used for ACL/XATTR backup and restore
 */
static bRC handleXACLdata(bpContext *ctx, struct xacl_pkt *xacl)
{
   ASSERT_CTX;

   DMSG(ctx, D1, "handleXACLdata: %i\n", xacl->func);
   DOCKER *self = pluginclass(ctx);
   return self->handleXACLdata(ctx, xacl);
}
