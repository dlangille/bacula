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

#include "bacula.h"
#include "fd_plugins.h"
#include "fd_common.h"
#include "lib/cmd_parser.h"
#include "lib/mem_pool.h"
#include "findlib/bfile.h"
#include "journal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_LICENSE      "AGPLv3"
#define PLUGIN_AUTHOR       "Henrique Faria"
#define PLUGIN_DATE         "February 2019"
#define PLUGIN_VERSION      "0.1"
#define PLUGIN_DESCRIPTION  "CDP Plugin"

#ifdef HAVE_WIN32
#define CONCAT_PATH "%s\\%s"
#define WORKING_JOURNAL_TEMPLATE "%s\\%s_%d.journal"
#else
#define CONCAT_PATH "%s/%s"
#define WORKING_JOURNAL_TEMPLATE "%s/%s_%d.journal"
#endif

/* Forward referenced functions */
static bRC newPlugin(bpContext *ctx);
static bRC freePlugin(bpContext *ctx);
static bRC handlePluginEvent(bpContext *ctx, bEvent *event, void *value);
static bRC startBackupFile(bpContext *ctx, struct save_pkt *sp);
static bRC endBackupFile(bpContext *ctx);
static bRC pluginIO(bpContext *ctx, struct io_pkt *io);
static bRC startRestoreFile(bpContext *ctx, const char *cmd);
static bRC createFile(bpContext *ctx, struct restore_pkt *rp);
static bRC endRestoreFile(bpContext *ctx);
static bRC checkFile(bpContext *ctx, char *fname);

/* Pointers to Bacula functions */
static bFuncs *bfuncs = NULL;
static bInfo  *binfo = NULL;

/* Backup Variables */
static char *working = NULL;

static pInfo pluginInfo = {
   sizeof(pluginInfo),
   FD_PLUGIN_INTERFACE_VERSION,
   FD_PLUGIN_MAGIC,
   PLUGIN_LICENSE,
   PLUGIN_AUTHOR,
   PLUGIN_DATE,
   PLUGIN_VERSION,
   PLUGIN_DESCRIPTION
};

static pFuncs pluginFuncs = {
   sizeof(pluginFuncs),
   FD_PLUGIN_INTERFACE_VERSION,

   /* Entry points into plugin */
   newPlugin,                    /* new plugin instance */
   freePlugin,                   /* free plugin instance */
   NULL,
   NULL,
   handlePluginEvent,
   startBackupFile,
   endBackupFile,
   startRestoreFile,
   endRestoreFile,
   pluginIO,
   createFile,
   NULL,
   checkFile,
   NULL,                         /* No ACL/XATTR */
   NULL,                         /* No Restore file list */
   NULL                          /* No checkStream */
};

static int DBGLVL = 50;

class CdpContext: public SMARTALLOC
{
public:
   bpContext *ctx;

   /** Used by both Backup and Restore Cycles **/
   BFILE fd;
   POOLMEM *fname;
   bool is_in_use;

   /** Used only by the Backup Cycle **/
   POOLMEM *clientJPath;
   POOLMEM *jobJPath;
   POOLMEM *drivesList; // Windows only
   char *jobName;
   
   bool accurate_warning;
   bool started_backup;
   bool canceled;
   alist userHomes;
   alist journals;
   int jIndex;
   cmd_parser parser;
   Journal *journal;

   CdpContext(bpContext *actx):
     ctx(actx), fname(NULL), is_in_use(false), clientJPath(NULL),
     jobJPath(NULL), drivesList(NULL), jobName(NULL), accurate_warning(false),
     started_backup(false), canceled(false),
     userHomes(100, owned_by_alist), journals(100, not_owned_by_alist), jIndex(0)
   {
      fname = get_pool_memory(PM_FNAME);
      clientJPath = get_pool_memory(PM_FNAME);
      jobJPath = get_pool_memory(PM_FNAME);
#ifdef HAVE_WIN32
      drivesList = get_pool_memory(PM_FNAME);
      *drivesList = 0;
#endif
      *fname = *clientJPath = *jobJPath = 0;
   };

   /** Methods called during Backup */
   void migrateJournal() {
      char *uh;
      int i = 0;

      foreach_alist(uh, &userHomes) {
         Journal *j = new Journal();
         Mmsg(clientJPath, CONCAT_PATH, uh, JOURNAL_CLI_FNAME);
         j->setJournalPath(clientJPath);
 
         Mmsg(jobJPath, WORKING_JOURNAL_TEMPLATE, working, jobName, i);
         j->migrateTo(jobJPath);
         journals.append(j);
         i++;
      }
   };

   bool handleBackupCommand(bpContext *ctx, char *cmd) {
      int i;
      POOLMEM *userHome;
      parser.parse_cmd(cmd);
      for (i = 1; i < parser.argc ; i++) {

         if (strcasecmp(parser.argk[i], "userhome") == 0 && parser.argv[i]) {
            userHome = get_pool_memory(PM_FNAME);
            pm_strcpy(userHome, parser.argv[i]);
            struct stat sp;

            if (stat(userHome, &sp) != 0) {
               Jmsg(ctx, M_ERROR, _("Parameter userhome not found: %s\n"), userHome);
               return false;
            }

            if (!S_ISDIR(sp.st_mode)) {
               Jmsg(ctx, M_ERROR, _("Paramater userhome is not a directory: %s\n"), userHome);
               return false;
            }

            Dmsg(ctx, DBGLVL, "User Home: %s\n", userHome);
            userHomes.append(bstrdup(userHome));
            free_and_null_pool_memory(userHome);
         } else if (strcasecmp(parser.argk[i], "user") == 0 && parser.argv[i]) {
            userHome = get_pool_memory(PM_FNAME);
            int rc = get_user_home_directory(parser.argv[i], userHome);

            if (rc != 0) {
               Jmsg(ctx, M_ERROR, _("User not found in the system: %s\n"), parser.argv[i]);
               return false;
            }

            userHomes.append(bstrdup(userHome));
            Dmsg(ctx, DBGLVL, "User Home: %s\n", userHome);
            free_and_null_pool_memory(userHome);
            return true;
         } else if (strcasecmp(parser.argk[i], "group") == 0 && parser.argv[i]) {
            int rc = get_home_directories(parser.argv[i], &userHomes);

            if (rc != 0) {
               return false;
            }

            return true;
     } else {
            Jmsg(ctx, M_ERROR, _("Can't analyse plugin command line %s\n"), cmd);
            return false;
         }
      }

      return true;
   };

   FileRecord *nextRecord() {
      if (canceled) {
         if (journal) {
            journal->endTransaction();
         }
         return NULL;
      }

      if (!started_backup) { 
         if (jIndex >= journals.size()) {
            return NULL;
         }

         journal = (Journal *) journals[jIndex];

         if (!journal->beginTransaction("r")) {
            return NULL;
         }

         started_backup = true;
      }

      FileRecord *fc = journal->readFileRecord();

      if (fc == NULL) {
         journal->endTransaction();
         started_backup = false;
         unlink(journal->_jPath);
         Dmsg(ctx, DBGLVL, "No more files to backup. Deleting journal: %s\n", journal->_jPath);
         delete(journal);
         jIndex++;
      }

      return fc;
   };

   ~CdpContext() {
      if (journal) {
         // Clear any possible pending transaction (e.g canceled job)
         journal->endTransaction();
         canceled = true;
      }

      free_and_null_pool_memory(clientJPath);
      free_and_null_pool_memory(jobJPath);
      free_and_null_pool_memory(fname);
#ifdef HAVE_WIN32
      free_and_null_pool_memory(drivesList);
#endif
   };

   /* Adjust the current fileset depending on what we find in the Journal */
   void adapt(Journal *j) {
      SettingsRecord *settings = j->readSettings();

      /* We should not backup the Spool Directory */
      if (settings != NULL) {
         char *sdir = bstrdup(settings->getSpoolDir());
         bfuncs->AddExclude(ctx, sdir);
         Dmsg(ctx, DBGLVL, "Excluded Spool Directory from FileSet %s\n", sdir);
         delete settings;
      }

      /* Foreach folder watched, we add the folder to the backup */
      if (!j->beginTransaction("r")) {
         return;
      }

      FolderRecord *rec;

#ifdef HAVE_WIN32
      int i = 0;

      for(;;) {
         rec = j->readFolderRecord();

         if (rec == NULL) {
            drivesList[i] = '\0';
            break;
         }

         /*On Windows, we must also add the folder drives to create
           the VSS Snapshot */
         if (!strchr(drivesList, rec->path[0])) {
            drivesList[i++] = toupper(rec->path[0]);
            Dmsg(ctx, DBGLVL, "Included Drive %c\n", rec->path[0]);
         }
         
         bfuncs->AddInclude(ctx, rec->path);
         Dmsg(ctx, DBGLVL, "Included Directory into the FileSet %s\n", rec->path);
         delete rec;
      }

#else 
      for(;;) {
         rec = j->readFolderRecord();

         if (rec == NULL) {
            break;
         }

         bfuncs->AddInclude(ctx, rec->path);
         Dmsg(ctx, DBGLVL, "Included Directory %s\n", rec->path);
         delete rec;
      }
#endif

      j->endTransaction();
   };

   void adaptFileSet() {
      for (int i = 0; i < journals.size(); i++) {
         Journal *j = (Journal *) journals[i];
         adapt(j);
      }

   }
};

/*
 * Plugin called here when it is first loaded
 */
bRC DLL_IMP_EXP
loadPlugin(bInfo *lbinfo, bFuncs *lbfuncs, pInfo **pinfo, pFuncs **pfuncs)
{
   bfuncs = lbfuncs;                  /* set Bacula funct pointers */
   binfo  = lbinfo;

   *pinfo  = &pluginInfo;             /* return pointer to our info */
   *pfuncs = &pluginFuncs;            /* return pointer to our functions */

   bfuncs->getBaculaValue(NULL, bVarWorkingDir, (void *)&working);
   return bRC_OK;
}

/*
 * Plugin called here when it is unloaded, normally when
 *  Bacula is going to exit.
 */
bRC DLL_IMP_EXP
unloadPlugin()
{
   return bRC_OK;
}

/*
 * Called here to make a new instance of the plugin -- i.e. when
 *  a new Job is started.  There can be multiple instances of
 *  each plugin that are running at the same time.  Your
 *  plugin instance must be thread safe and keep its own
 *  local data.
 */
static bRC newPlugin(bpContext *ctx)
{
  CdpContext *pCtx = New(CdpContext(ctx));
  ctx->pContext = (void *) pCtx;        /* set our context pointer */
  Dmsg(ctx, DBGLVL, "Working Directory: %s\n", working);
  return bRC_OK;
}

/*
 * Release everything concerning a particular instance of a
 *  plugin. Normally called when the Job terminates.
 */
static bRC freePlugin(bpContext *ctx)
{
   CdpContext *pCtx = (CdpContext *) ctx->pContext;
   delete(pCtx);
   return bRC_OK;
}

static bRC handlePluginEvent(bpContext *ctx, bEvent *event, void *value)
{
   CdpContext *pCtx = (CdpContext *) ctx->pContext;

   switch (event->eventType) {

   case bEventPluginCommand:
      if (!pCtx->handleBackupCommand(ctx, (char *) value)) {
         return bRC_Error;
      };
      pCtx->is_in_use = true;
      pCtx->migrateJournal();
      pCtx->adaptFileSet();
      break;

   case bEventEstimateCommand:
      Jmsg(ctx, M_FATAL, _("The CDP plugin doesn't support estimate\n"));
      return bRC_Error;

   case bEventJobStart:
      bfuncs->getBaculaValue(NULL, bVarJobName, (void *) &(pCtx->jobName));
      
      if (pCtx->jobName == NULL) {
         pCtx->jobName = (char *) "backup_job";
      }

      Dmsg(ctx, DBGLVL, "Job Name: %s\n", pCtx->jobName);
      break;

   case bEventCancelCommand:
      pCtx->canceled = true;
      Dmsg(ctx, DBGLVL, "Job canceled\n");
      break;

#ifdef HAVE_WIN32
   case bEventVssPrepareSnapshot:
      strcpy((char *) value, pCtx->drivesList);
      Dmsg(ctx, DBGLVL, "VSS Drives list: %s\n", pCtx->drivesList);
      break; 
#endif

   default:
      break;
   }

   return bRC_OK;
}

/*
 * Called when starting to backup a file.  Here the plugin must
 *  return the "stat" packet for the directory/file and provide
 *  certain information so that Bacula knows what the file is.
 *  The plugin can create "Virtual" files by giving them a
 *  name that is not normally found on the file system.
 */
static bRC startBackupFile(bpContext *ctx, struct save_pkt *sp)
{
   CdpContext *pCtx = (CdpContext *) ctx->pContext;
   FileRecord *rec = pCtx->nextRecord();

   if(rec != NULL) {
      //Fill save_pkt struct
      POOLMEM *bacula_fname = get_pool_memory(PM_FNAME);
      rec->getBaculaName(bacula_fname);
      sp->fname = bstrdup(bacula_fname);
      sp->type = FT_REG;
      rec->decode_attrs(sp->statp);
      
      //Save the name of the file that's inside the Spool Dir
      //That's the file that will be backed up
      pm_strcpy(pCtx->fname, rec->sname);
      delete(rec);
      free_and_null_pool_memory(bacula_fname);
      Dmsg(ctx, DBGLVL, "Starting backup of file: %s\n", sp->fname);
      return bRC_OK;
   } else {
      return bRC_Stop;
   }

}

/*
 * Done backing up a file.
 */
static bRC endBackupFile(bpContext *ctx)
{
   return bRC_More;
}

/*
 * Do actual I/O.  Bacula calls this after startBackupFile
 *   or after startRestoreFile to do the actual file
 *   input or output.
 */
static bRC pluginIO(bpContext *ctx, struct io_pkt *io)
{
   CdpContext *pCtx = (CdpContext *) ctx->pContext;

   io->status = -1;
   io->io_errno = 0;

   if (!pCtx) {
      return bRC_Error;
   }

   switch (io->func) {
   case IO_OPEN:
      if (bopen(&pCtx->fd, pCtx->fname, io->flags, io->mode) < 0) {
            io->io_errno = errno;
            io->status = -1;
            Jmsg(ctx, M_ERROR, "Open file %s failed: ERR=%s\n",
                 pCtx->fname, strerror(errno));
            return bRC_Error;
      }
      io->status = 1;
      break;

   case IO_READ:
      if (!is_bopen(&pCtx->fd)) {
         Jmsg(ctx, M_FATAL, "Logic error: NULL read FD\n");
         return bRC_Error;
      }

      /* Read data from file */
      io->status = bread(&pCtx->fd, io->buf, io->count);
      break;

   case IO_WRITE:
      if (!is_bopen(&pCtx->fd)) {
         Jmsg(ctx, M_FATAL, "Logic error: NULL write FD\n");
         return bRC_Error;
      }

      io->status = bwrite(&pCtx->fd, io->buf, io->count);
      break;

   case IO_SEEK:
      if (!is_bopen(&pCtx->fd)) {
         Jmsg(ctx, M_FATAL, "Logic error: NULL FD on delta seek\n");
         return bRC_Error;
      }
      /* Seek not needed for this plugin, we don't use real sparse file */
      io->status = blseek(&pCtx->fd, io->offset, io->whence);
      break;

   /* Cleanup things during close */
   case IO_CLOSE:
      io->status = bclose(&pCtx->fd);
      break;
   }

   return bRC_OK;
}

static bRC startRestoreFile(bpContext *ctx, const char *cmd)
{
   Dmsg(ctx, DBGLVL, "Started file restoration\n");
   return bRC_Core;
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
   CdpContext *pCtx = (CdpContext *) ctx->pContext;
   pm_strcpy(pCtx->fname, rp->ofname);
   rp->create_status = CF_CORE;
   Dmsg(ctx, DBGLVL, "Creating file %s\n", rp->ofname);
   return bRC_OK;
}

static bRC endRestoreFile(bpContext *ctx)
{
   Dmsg(ctx, DBGLVL, "Finished file restoration\n");
   return bRC_OK;
}

/* When using Incremental dump, all previous dumps are necessary */
static bRC checkFile(bpContext *ctx, char *fname)
{
   CdpContext *pCtx = (CdpContext *) ctx->pContext;

   if (pCtx->is_in_use) {
      if (!pCtx->accurate_warning) {
         pCtx->accurate_warning = true;
         Jmsg(ctx, M_WARNING, "Accurate mode is not supported. Please disable Accurate mode for this job.\n");
      }

      return bRC_Seen;
   } else {
      return bRC_OK;
   }
}


#ifdef __cplusplus
}
#endif

