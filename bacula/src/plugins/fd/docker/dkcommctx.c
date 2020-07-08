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

#include "dkcommctx.h"

#define PLUGINPREFIX                "dkcommctx:"

/*
 * Constructor, does simple initialization.
 */
DKCOMMCTX::DKCOMMCTX(const char *cmd) :
      bpipe(NULL),
      param_include_container(NULL),
      param_include_image(NULL),
      param_exclude_container(NULL),
      param_exclude_image(NULL),
      param_container(NULL),
      param_image(NULL),
      param_volume(NULL),
      param_mode(DKPAUSE),  /* default backup mode */
      param_container_create(true),
      param_container_run(false),
      param_container_imageid(false),
      param_container_defaultnames(false),
      param_docker_host(NULL),
      param_timeout(0),

      abort_on_error(false),
      all_containers(NULL),
      all_images(NULL),
      all_volumes(NULL),
      objs_to_backup(NULL),
      all_to_backup(false),
      all_vols_to_backup(false),
      f_eod(false),
      f_error(false),
      f_fatal(false),
      ini(NULL),
      workingvolume(NULL),
      workingdir(NULL)
{
   /* setup initial plugin command here */
   command = bstrdup(cmd);
   /* prepare backup list */
   objs_to_backup = New(alist(32, not_owned_by_alist));
   param_timeout = 30;     // this is a default you can overwrite
};

/*
 * Destructor, releases all memory allocated.
 */
DKCOMMCTX::~DKCOMMCTX()
{
   if (command){
      free(command);
   }
   if (ini){
      delete ini;
   }

   release_all_dkinfo_list(&all_containers);
   release_all_dkinfo_list(&all_images);
   release_all_dkinfo_list(&all_volumes);
   if (objs_to_backup){
      delete objs_to_backup;
   }
   release_all_pm_list(&param_include_container);
   release_all_pm_list(&param_exclude_container);
   release_all_pm_list(&param_include_image);
   release_all_pm_list(&param_exclude_image);
   release_all_pm_list(&param_container);
   release_all_pm_list(&param_image);
   release_all_pm_list(&param_volume);
   free_and_null_pool_memory(param_docker_host);
   free_and_null_pool_memory(workingvolume);
   free_and_null_pool_memory(workingdir);
};

/*
 * sets runtime workingdir variable used in working volume creation.
 *
 * in:
 *    workdir - the file daemon working directory parameter
 * out:
 *    none
 */
void DKCOMMCTX::setworkingdir(char* workdir)
{
   if (workingdir == NULL){
      /* not allocated yet */
      workingdir = get_pool_memory(PM_FNAME);
   }
   pm_strcpy(&workingdir, workdir);
   DMSG1(NULL, DVDEBUG, "workingdir: %s\n", workingdir);
};

/*
 * Releases the memory allocated by all_* list.
 *
 * in:
 *    alist - a list to release
 * out:
 *    none
 */
void DKCOMMCTX::release_all_dkinfo_list(alist **list)
{
   DKINFO *dkinfo;

   if (*list){
      foreach_alist(dkinfo, *list){
         if (dkinfo){
            delete dkinfo;
         }
      }
      delete *list;
   }
   *list = NULL;
}

/*
 * Releases the memory allocated by param_* list.
 *
 * in:
 *    alist - a list to release
 * out:
 *    none
 */
void DKCOMMCTX::release_all_pm_list(alist **list)
{
   POOLMEM *pm;

   if (*list){
      foreach_alist(pm, *list){
         free_and_null_pool_memory(pm);
      }
      delete *list;
   }
   *list = NULL;
}

/*
 * Prepare a docker volume directory for container execution.
 *
 * in:
 *    bpContext - required for debug/job messages
 *    jobid - for volume directory distinguish and jobid tracking
 * out:
 *    bRC_OK - on success
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::prepare_working_volume(bpContext *ctx, int jobid)
{
   char *dir;
   struct stat statp;
   pid_t pid = getpid();

   DMSG0(ctx, DINFO, "prepare_working_volume called\n");
   if (workingvolume == NULL){
      workingvolume = get_pool_memory(PM_FNAME);
      /* create dir template for mkdtemp function */
      Mmsg(workingvolume, "%s/docker-%d-%d-XXXXXX",
            workingdir != NULL ? workingdir : WORKDIR, jobid, pid);
      dir = mkdtemp(workingvolume);
      if (dir == NULL){
         /* failback to standard method */
         Mmsg(workingvolume, "%s/docker-%d-%d",
               workingdir != NULL ? workingdir : WORKDIR, jobid, pid);
         if (stat(workingvolume, &statp) != 0){
            berrno be;
            /* if the path does not exist then create one */
            if (be.code() != ENOENT || mkdir(workingvolume, 0700) != 0){
               /* creation or other error, set new errno and proceed to inform user */
               be.set_errno(errno);
               DMSG2(ctx, DERROR, "working volume path (%s) creation Err=%s\n", workingvolume, be.bstrerror());
               JMSG2(ctx, abort_on_error ? M_FATAL : M_ERROR,
                     "Working volume path (%s) creation Err=%s!\n", workingvolume, be.bstrerror());
               return bRC_Error;
            }
         } else
            if (!S_ISDIR(statp.st_mode)){
               /* the expected working dir/volume is already available and it is not a directory, strange */
               DMSG2(ctx, DERROR, "working volume path (%s) is not directory Mode=%o\n", workingvolume, statp.st_mode);
               JMSG2(ctx, abort_on_error ? M_FATAL : M_ERROR,
                     "Working volume path (%s) is not directory Mode=%o\n", workingvolume, statp.st_mode);
               return bRC_Error;
            }
      }
   }
   DMSG1(ctx, DINFO, "prepare_working_volume finish: %s\n", workingvolume);
   return bRC_OK;
};

/*
 * It removes files and a volume directory after a successful backup/restore
 *
 * in:
 *    bpContext - required for debug/job messages
 * out
 *    none
 */
void DKCOMMCTX::clean_working_volume(bpContext* ctx)
{
   POOL_MEM fname(PM_FNAME);
   int status;
   int a;
   bool ferr = false;
   const char *ftab[] = {
      BACULACONTAINERERRLOG,
      BACULACONTAINERARCHLOG,
      BACULACONTAINERFIN,
      BACULACONTAINERFOUT,
      NULL,
   };

   DMSG0(ctx, DDEBUG, "clean_working_volume called\n");
   for (a = 0; ftab[a] != NULL; a++) {
      render_working_volume_filename(fname, ftab[a]);
      status = unlink(fname.c_str());
      if (status < 0){
         /* unlink error - report to user */
         berrno be;
         if (be.code() == ENOENT){
            continue;
         }
         ferr = true;
         DMSG2(ctx, DERROR, "unlink error: %s Err=%s\n", fname.c_str(), be.bstrerror());
         JMSG2(ctx, M_ERROR, "Cannot unlink a file: %s Err=%s\n", fname.c_str(), be.bstrerror());
      }
      DMSG1(ctx, DDEBUG, "removing: %s\n", fname.c_str())
   }
   if (!ferr){
      status = rmdir(workingvolume);
      if (status < 0){
         /* unlink error - report to user */
         berrno be;
         DMSG2(ctx, DERROR, "rmdir error: %s Err=%s\n", workingvolume, be.bstrerror());
         JMSG2(ctx, M_ERROR, "Cannot remove directory: %s Err=%s\n", workingvolume, be.bstrerror());
      }
   }
   free_and_null_pool_memory(workingvolume);
   DMSG0(ctx, DDEBUG, "clean_working_volume finish.\n");
};

/*
 * Terminate the connection represented by BPIPE object.
 *  it shows a debug and job messages when connection close
 *  is unsuccessful and when ctx is available only.
 *
 * in:
 *    bpContext - Bacula Plugin context required for debug/job
 *                messages to show, it could be NULL in this
 *                case no messages will be shown.
 * out:
 *    none
 */
void DKCOMMCTX::terminate(bpContext *ctx)
{
   int status;

   if (is_closed()){
      return;
   }

   DMSG(ctx, DDEBUG, "Terminating PID=%d\n", bpipe->worker_pid);
   status = close_bpipe(bpipe);
   if (status){
      /* error during close */
      berrno be;
      f_error = true;
      DMSG(ctx, DERROR, "Error closing backend. Err=%s\n", be.bstrerror(status));
      JMSG(ctx, is_fatal() ? M_FATAL : M_ERROR, "Error closing backend. Err=%s\n", be.bstrerror(status));
   }
   if (bpipe->worker_pid){
      /* terminate the backend */
      kill(bpipe->worker_pid, SIGTERM);
   }
   bpipe = NULL;
};

/*
 * Run the command using *_CMD compile variable and prepared
 *  parameters.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    cmd - the command type to execute
 *    args - the command arguments
 * out:
 *    True - when command execute successfully
 *    False - when execution return error
 */
bool DKCOMMCTX::execute_command(bpContext *ctx, POOL_MEM &args)
{
   return execute_command(ctx, args.c_str());
}

/*
 * Run the command using *_CMD compile variable and prepared
 *  parameters.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    cmd - the command type to execute
 *    args - the command arguments
 * out:
 *    True - when command execute successfully
 *    False - when execution return error
 */
bool DKCOMMCTX::execute_command(bpContext *ctx, const char *args)
{
   return execute_command(ctx, (char*)args);
}

/*
 * Run the command using *_CMD compile variable and prepared
 *  parameters.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    cmd - the command type to execute
 *    args - the command arguments
 * out:
 *    True - when command execute successfully
 *    False - when execution return error
 */
bool DKCOMMCTX::execute_command(bpContext *ctx, POOLMEM *args)
{
   POOL_MEM exe_cmd(PM_FNAME);
   POOL_MEM DH(PM_NAME);
   const char *command = DOCKER_CMD;
   char *envp[3];
   int a = 0;

   if (args == NULL){
      /* cannot execute command with args NULL */
      DMSG0(ctx, DERROR, "Logic error: Cannot execute empty command tool!\n");
      JMSG0(ctx, M_FATAL, "Logic error: Cannot execute empty command tool!\n");
      return false;
   }
   /* check if command is still available to Bacula */
   if (access(command, X_OK) < 0){
      berrno be;
      DMSG2(ctx, DERROR, "Unable to access %s command. Err=%s\n", command, be.bstrerror());
      JMSG2(ctx, M_FATAL, "Unable to access %s command. Err=%s\n", command, be.bstrerror());
      return false;
   }
   /* yes, we still have access to it.
    * the format of a command line to execute is: <cmd> <params> */
   Mmsg(exe_cmd, "%s %s", command, args);
   DMSG(ctx, DINFO, "Executing: %s\n", exe_cmd.c_str());
   /* preparing envinroment variables */
   envp[a++] = bstrdup("LANG=C");
   if (param_docker_host != NULL){
      Mmsg(DH, "DOCKER_HOST=%s", param_docker_host);
      envp[a++] = bstrdup(DH.c_str());
   }
   envp[a] = NULL;
   bpipe = open_bpipe(exe_cmd.c_str(), 0, "rw", envp);
   a = 0;
   while (envp[a] != NULL){
      free(envp[a++]);
   }
   if (bpipe == NULL){
      berrno be;
      DMSG(ctx, DERROR, "Unable to execute command. Err=%s\n", be.bstrerror());
      JMSG(ctx, M_FATAL, "Unable to execute command. Err=%s\n", be.bstrerror());
      return false;
   }
   DMSG(ctx, DINFO, "Command executed at PID=%d\n", get_backend_pid());
   return true;
}

/*
 * Read all output from command tool - until eod and save it
 *  in the out buffer.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    out - the POOL_MEM buffer we will read data
 * out:
 *    -1 - when we've got any error; the function will report
 *         it to Bacula when ctx is not NULL
 *    0 - when no more data to read - EOD
 *    <n> - the size of received message
 */
int32_t DKCOMMCTX::read_output(bpContext *ctx, POOL_MEM &out)
{
   int status;
   int rbytes;
   bool ndone;

   if (is_closed()){
      f_error = true;
      DMSG0(ctx, DERROR, "BPIPE to command tool is closed, cannot get data.\n");
      JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE to command tool is closed, cannot get data.\n");
      return -1;
   }

   /* set variables */
   rbytes = 0;
   ndone = true;
   /* wait a bit for a command to execute */
   bmicrosleep(0, 1000);   // sleep 1mS
   /* read all output data */
   while (ndone){
      status = read_data(ctx, out.c_str() + rbytes, out.size() - rbytes);
      if (status < 0){
         /* error */
         return -1;
      }
      rbytes += status;
      if (is_eod()){
         /* we read all data available */
         ndone = false;
         continue;
      }
      /* it seems out buffer is too small for all data */
      out.check_size(rbytes + 1024);
   }
   return rbytes;
}

/*
 * Reads a single data block from command tool.
 *  It reads as more data as is available on the other size and will fit into
 *  a memory buffer - buf. When EOD encountered during reading it will set
 *  f_eod flag, so checking this flag is mandatory!
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    buf - a memory buffer for data
 *    len - the length of the memory buffer - buf
 * out:
 *    -1 - when we've got any error; the function reports it to Bacula when
 *         ctx is not NULL
 *    when no more data to read - EOD
 *    <n> - the size of received data
 */
int32_t DKCOMMCTX::read_data(bpContext *ctx, POOLMEM *buf, int32_t len)
{
   int status;
   int nbytes;
   int rbytes;
   int timeout;

   if (buf == NULL || len < 1){
      /* we have no space to read data */
      f_error = true;
      DMSG0(ctx, DERROR, "No space to read data from command tool.\n");
      JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "No space to read data from command tool.\n");
      return -1;
   }

   if (is_closed()){
      f_error = true;
      DMSG0(ctx, DERROR, "BPIPE to command tool is closed, cannot get data.\n");
      JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE to command tool is closed, cannot get data.\n");
      return -1;
   }

   /* we will read no more then len bytes available in the buf */
   nbytes = len;
   rbytes = 0;
   /* clear flags */
   f_eod = f_error = f_fatal = false;
   timeout = 200;          // timeout of 200ms
   while (nbytes){
      status = fread(buf + rbytes, 1, nbytes, bpipe->rfd);
      if (status == 0){
         berrno be;
         if (ferror(bpipe->rfd) != 0){
            f_error = true;
            DMSG(ctx, DERROR, "BPIPE read error: ERR=%s\n", be.bstrerror());
            JMSG(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE read error: ERR=%s\n", be.bstrerror());
            return -1;
         }
         if (feof(bpipe->rfd) != 0){
            f_eod = true;
            return rbytes;
         }
         bmicrosleep(0, 1000);   // sleep 1mS
         if (!timeout--){
            /* reach timeout*/
            f_error = true;
            DMSG0(ctx, DERROR, "BPIPE read timeout.\n");
            JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE read timeout.\n");
            return -1;
         }
      } else {
         timeout = 200;          // reset timeout
      }
      nbytes -= status;
      rbytes += status;
   }
   return rbytes;
}

/*
 * Sends a raw data block to command tool.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    buf - a message buffer contains data to send
 *    len - the length of the data to send
 * out:
 *    -1 - when encountered any error
 *    <n> - the number of bytes sent, success
 */
int32_t DKCOMMCTX::write_data(bpContext *ctx, POOLMEM *buf, int32_t len)
{
   int status;
   int nbytes;
   int wbytes;
   int timeout;

   if (buf == NULL){
      /* we have no data to write */
      f_error = true;
      DMSG0(ctx, DERROR, "No data to send to command tool.\n");
      JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "No data to send to command tool.\n");
      return -1;
   }

   if (is_closed()){
      f_error = true;
      DMSG0(ctx, DERROR, "BPIPE to command tool is closed, cannot send data.\n");
      JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE to command tool is closed, cannot send data.\n");
      return -1;
   }

   /* we will write len bytes available in the buf */
   nbytes = len;
   wbytes = 0;
   /* clear flags */
   f_eod = f_error = f_fatal = false;
   timeout = 200;          // timeout of 200ms
   while (nbytes){
      status = fwrite(buf + wbytes, 1, nbytes, bpipe->wfd);
      if (status == 0){
         berrno be;
         if (ferror(bpipe->wfd) != 0){
            f_error = true;
            DMSG(ctx, DERROR, "BPIPE write error: ERR=%s\n", be.bstrerror());
            JMSG(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE write error: ERR=%s\n", be.bstrerror());
            return -1;
         }
         bmicrosleep(0, 1000);   // sleep 1mS
         if (!timeout--){
            /* reached timeout*/
            f_error = true;
            DMSG0(ctx, DERROR, "BPIPE write timeout.\n");
            JMSG0(ctx, is_fatal() ? M_FATAL : M_ERROR, "BPIPE write timeout.\n");
            return -1;
         }
      } else {
         timeout = 200;          // reset timeout
      }
      nbytes -= status;
      wbytes += status;
   }
   return wbytes;
}

/*
 * Render a command tool parameter for string value.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    fmt - a low-level parameter name
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::render_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *fmt, const char *name, char *value)
{
   if (bstrcasecmp(name, pname)){
      if (!*param){
         *param = get_pool_memory(PM_NAME);
         Mmsg(*param, " -%s '%s' ", fmt, value);
         DMSG(ctx, DDEBUG, "render param:%s\n", *param);
      }
      return true;
   }
   return false;
}

/*
 * Render a command tool parameter for integer value.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    fmt - a low-level parameter name
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::render_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *fmt, const char *name, int value)
{
   if (bstrcasecmp(name, pname)){
      if (!*param){
         *param = get_pool_memory(PM_NAME);
         Mmsg(*param, " -%s %d ", value);
         DMSG(ctx, DDEBUG, "render param:%s\n", *param);
      }
      return true;
   }
   return false;
}

/*
 * Render a command tool parameter for string value.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::render_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *name, char *value)
{
   if (bstrcasecmp(name, pname)){
      if (!*param){
         *param = get_pool_memory(PM_NAME);
         Mmsg(*param, "%s", value);
         DMSG(ctx, DDEBUG, "render param:%s\n", *param);
      }
      return true;
   }
   return false;
}

/*
 * Setup DKCOMMCTX parameter for boolean value.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::render_param(bpContext *ctx, bool *param, const char *pname, const char *name, bool value)
{
   if (bstrcasecmp(name, pname)){
      if (param){
         *param = value;
         DMSG2(ctx, DDEBUG, "render param: %s=%s\n", pname, *param ? "True" : "False");
      }
      return true;
   }
   return false;
}

/*
 * Setup DKCOMMCTX parameter for int32_t value.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::render_param(bpContext *ctx, int32_t *param, const char *pname, const char *name, int32_t value)
{
   if (bstrcasecmp(name, pname)){
      if (param){
         *param = value;
         DMSG2(ctx, DDEBUG, "render param: %s=%d\n", pname, *param);
      }
      return true;
   }
   return false;
}

/*
 * Setup DKCOMMCTX parameter for boolean from string value.
 *  The parameter value will be false if value start with '0' character and
 *  will be true in any other case. So, when a plugin will have a following:
 *    param
 *    param=...
 *    param=1
 *  then a param will be set to true.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::parse_param(bpContext *ctx, bool *param, const char *pname, const char *name, char *value)
{
   if (bstrcasecmp(name, pname)){
      if (value && *value == '0'){
         *param = false;
      } else {
         *param = true;
      }
      DMSG2(ctx, DINFO, "%s parameter: %s\n", name, *param ? "True" : "False");
      return true;
   }
   return false;
}

/*
 * Setup DKCOMMCTX parameter for integer from string value.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::parse_param(bpContext *ctx, int32_t *param, const char *pname, const char *name, char *value)
{
   if (value && bstrcasecmp(name, pname)){
      /* convert str to integer */
      *param = atoi(value);
      if (*param == 0){
         /* error in conversion */
         f_error = true;
         DMSG2(ctx, DERROR, "Invalid %s parameter: %s\n", name, value);
         JMSG2(ctx, M_ERROR, "Invalid %s parameter: %s\n", name, value);
         return false;
      }
      DMSG2(ctx, DINFO, "%s parameter: %d\n", name, *param);
      return true;
   }
   return false;
}

/*
 * Setup DKCOMMCTX parameter for DOCKER_BACKUP_MODE_T from string value.
 *  supported values are: pause, nopause
 *  any other will be ignored.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    param - a pointer to the param variable where we will render a parameter
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::parse_param(bpContext *ctx, DOCKER_BACKUP_MODE_T *param, const char *pname, const char *name, char *value)
{
   if (bstrcasecmp(name, pname)){
      if (value){
         if (strcasecmp(value, "pause") == 0){
            *param = DKPAUSE;
         } else
         if (strcasecmp(value, "nopause") == 0){
            *param = DKNOPAUSE;
         }
      }
      switch (*param){
         case DKPAUSE:
            DMSG(ctx, DINFO, "%s parameter: DKPAUSE\n", name);
            break;
         case DKNOPAUSE:
            DMSG(ctx, DINFO, "%s parameter: DKNOPAUSE\n", name);
            break;
      }
      return true;
   }
   return false;
}

/*
 * Render and add a parameter for string value to alist.
 *  When alist is NULL (uninitialized) then it creates a new list to use.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    list - pointer to alist class to use
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::add_param_str(bpContext *ctx, alist **list, const char *pname, const char *name, char *value)
{
   POOLMEM *param;

   if (bstrcasecmp(name, pname)){
      if (!*list){
         *list = New(alist(8, not_owned_by_alist));
      }
      param = get_pool_memory(PM_NAME);
      Mmsg(param, "%s", value);
      (*list)->append(param);
      DMSG2(ctx, DDEBUG, "add param: %s=%s\n", name, value);
      return true;
   }
   return false;
}

/*
 * Render and set a parameter for string value.
 *  When param is NULL (uninitialized) then it allocates a new string.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    list - pointer to alist class to use
 *    pname - a name of the parameter to compare
 *    name - a name of the parameter from parameter list
 *    value - a value to render
 * out:
 *    True if parameter was rendered
 *    False if it was not the parameter required
 */
bool DKCOMMCTX::parse_param(bpContext *ctx, POOLMEM **param, const char *pname, const char *name, char *value)
{
   if (bstrcasecmp(name, pname)){
      if (!*param){
         *param = get_pool_memory(PM_NAME);
         pm_strcpy(param, value);
         DMSG2(ctx, DDEBUG, "add param: %s=%s\n", name, value);
      }
      return true;
   }
   return false;
}

/*
 * Parse a restore plugin parameters for DKCOMMCTX class (single at a time).
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    item - restore plugin parameter to parse
 * out:
 *    if parameter found it will be rendered in class variable
 */
void DKCOMMCTX::parse_parameters(bpContext *ctx, ini_items &item)
{
   /* container_create variable */
   if (render_param(ctx, &param_container_create, "container_create", item.name, item.val.boolval)){
      return;
   }
   /* container_create variable */
   if (render_param(ctx, &param_container_run, "container_run", item.name, item.val.boolval)){
      return;
   }
   /* container_create variable */
   if (render_param(ctx, &param_container_imageid, "container_imageid", item.name, item.val.boolval)){
      return;
   }
   /* container_create variable */
   if (render_param(ctx, &param_container_defaultnames, "container_defaultnames", item.name, item.val.boolval)){
      return;
   }
   /* docker_host variable */
   if (render_param(ctx, &param_docker_host, "docker_host", item.name, item.val.strval)){
      return;
   }
   /* timeout variable */
   if (render_param(ctx, &param_timeout, "timeout", item.name, item.val.int32val)){
      return;
   }
   f_error = true;
   DMSG(ctx, DERROR, "INI: Unknown parameter: %s\n", item.name);
   JMSG(ctx, M_ERROR, "INI: Unknown parameter: %s\n", item.name);
}
/*
 * Check and render DKCOMMCTX required parameters (single at a time).
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    argk - the parameter name
 *    argv - the parameter value (it could be null)
 * out:
 *    bRC_OK - when parameter found and rendered successfully
 *    bRC_Error - on any error
 *    bRC_Max - when parameter not found and should be checked elsewhere
 */
bRC DKCOMMCTX::parse_parameters(bpContext *ctx, char *argk, char *argv)
{
   /* check abort_on_error parameter */
   if (parse_param(ctx, &abort_on_error, "abort_on_error", argk, argv)){
      return bRC_OK;
   }
   /* check allvolumes parameter */
   if (parse_param(ctx, &all_vols_to_backup, "allvolumes", argk, argv)){
      return bRC_OK;
   }
   /* check and handle container list */
   if (add_param_str(ctx, &param_container, "container", argk, argv)){
      return bRC_OK;
   }
   /* check and handle include_container list */
   if (add_param_str(ctx, &param_include_container, "include_container", argk, argv)){
      return bRC_OK;
   }
   /* check and handle exclude_container list */
   if (add_param_str(ctx, &param_exclude_container, "exclude_container", argk, argv)){
      return bRC_OK;
   }
   /* check and handle image list */
   if (add_param_str(ctx, &param_image, "image", argk, argv)){
      return bRC_OK;
   }
   /* check and handle include_image list */
   if (add_param_str(ctx, &param_include_image, "include_image", argk, argv)){
      return bRC_OK;
   }
   /* check and handle exclude_image list */
   if (add_param_str(ctx, &param_exclude_image, "exclude_image", argk, argv)){
      return bRC_OK;
   }
   /* check and handle volume list */
   if (add_param_str(ctx, &param_volume, "volume", argk, argv)){
      return bRC_OK;
   }
   /* check and handle timeout parameter */
   if (parse_param(ctx, &param_timeout, "timeout", argk, argv)){
      return bRC_OK;
   }
   /* check mode parameter */
   if (parse_param(ctx, &param_mode, "mode", argk, argv)){
      return bRC_OK;
   }
   /* check docker_host parameter */
   if (parse_param(ctx, &param_docker_host, "docker_host", argk, argv)){
      return bRC_OK;
   }

   /* parameter unknown */
   return bRC_Max;
}

/*
 * Used for dumping current restore object contents for debugging.
 */
void DKCOMMCTX::dump_robjdebug(bpContext* ctx, restore_object_pkt* rop)
{
   POOL_MEM out(PM_FNAME);

   if (rop){
      out.check_size(rop->object_len + 1);
      pm_memcpy(out, rop->object, rop->object_len);
      DMSG1(ctx, DERROR, "failed restore object:\n%s\n", out.c_str());
   }
}

/*
 * Parse a Restore Object saved during backup and modified by user during restore.
 *    Every RO received will allocate a dedicated command context which is used
 *    by bEventRestoreCommand to handle default parameters for restore.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    rop - a restore object structure to parse
 * out:
 *    bRC_OK - on success
 *    bRC_Error - on error
 */
bRC DKCOMMCTX::parse_restoreobj(bpContext *ctx, restore_object_pkt *rop)
{
   int i;

   DMSG(ctx, DINFO, "INIcmd: %s\n", command);
   if (!ini){
      ini = new ConfigFile();
   }
   if (!ini->dump_string(rop->object, rop->object_len)){
      DMSG0(ctx, DERROR, "ini->dump_string failed.\n");
      dump_robjdebug(ctx, rop);
      return bRC_OK;
   }
   ini->register_items(plugin_items_dump, sizeof(struct ini_items));
   if (!ini->parse(ini->out_fname)){
      DMSG0(ctx, DERROR, "ini->parse failed.\n");
      dump_robjdebug(ctx, rop);
      return bRC_OK;
   }
   for (i=0; ini->items[i].name; i++){
      if (ini->items[i].found){
         parse_parameters(ctx, ini->items[i]);
      }
   }
   return bRC_OK;
}

/*
 * Sets all to backup variables.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 * out:
 *    none, internal variables set
 */
void DKCOMMCTX::set_all_to_backup(bpContext* ctx)
{
   set_all_containers_to_backup(ctx);
   set_all_images_to_backup(ctx);
   set_all_volumes_to_backup(ctx);
   all_to_backup = true;
}

/*
 * Sets objs_to_backup list for all containers backup.
 */
void DKCOMMCTX::set_all_containers_to_backup(bpContext *ctx)
{
   DKINFO *container;

   if (all_containers){
      foreach_alist(container, all_containers){
         objs_to_backup->append(container);
      };
   }
   all_to_backup = true;
};

/*
 * Sets objs_to_backup list for all images backup.
 */
void DKCOMMCTX::set_all_images_to_backup(bpContext *ctx)
{
   DKINFO *image;

   if (all_images){
      foreach_alist(image, all_images){
         objs_to_backup->append(image);
      };
   }
   all_to_backup = true;
};

/*
 * Sets objs_to_backup list for all volumes backup.
 */
void DKCOMMCTX::set_all_volumes_to_backup(bpContext *ctx)
{
   DKINFO *volume;

   if (all_volumes){
      foreach_alist(volume, all_volumes){
         objs_to_backup->append(volume);
      };
   }
   all_to_backup = true;
};

/*
 * Sets objs_to_backup list for all containers or images which match the
 *    container/image id or container/image names parameters from plugin
 *    command parameters.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    params - a list of parameters to compare
 *    dklist - a list of containers/images available
 *    estimate - set to true when doing estimate
 * out:
 *    this->objs_to_backup updated if required
 */
void DKCOMMCTX::filter_param_to_backup(bpContext *ctx, alist *params, alist *dklist, bool estimate)
{
   DKID dkid;
   DKINFO *dkinfo;
   POOLMEM *pobj;
   bool found;

   if (params){
      /* container parameter configured */
      foreach_alist(pobj, params){
         found = false;
         foreach_alist(dkinfo, dklist){
            DMSG3(ctx, DDEBUG, "compare: %s/%s vs %s\n",
                  (char*)dkinfo->id(), dkinfo->name(), pobj);
            /* we have to check container id or container names */
            dkid = pobj;
            if (bstrcmp(pobj, dkinfo->name()) || dkid == *(dkinfo->id())
                  || bstrcmp(pobj, dkinfo->get_image_repository())){
               /* container or image to backup found */
               objs_to_backup->append(dkinfo);
               found = true;
               DMSG3(ctx, DINFO, "adding %s to backup (1): %s (%s)\n",
                     dkinfo->type_str(),
                     dkinfo->name(), (char*)dkinfo->id());
               break;
            };
         };
         if (!found){
            /* docker object not found */
            f_error = true;
            if (!estimate){
               DMSG1(ctx, DERROR, "Not found to backup: %s!\n", pobj);
               JMSG1(ctx, is_fatal() ? M_FATAL : M_ERROR, "Not found to backup: %s!\n", pobj);
            } else {
               DMSG1(ctx, DERROR, "Not found to estimate: %s!\n", pobj);
               JMSG1(ctx, is_fatal() ? M_FATAL : M_ERROR, "Not found to estimate: %s!\n", pobj);
            };
         };
      };
   };
};

/*
 * It is called when 'allvolumes' parameter is set for backup and add
 *  a volumes mounted in containers selected to backup by reading
 *  a Mounts parameter list from docker container.
 *
 * in:
 *    bpContext - required for debug/job messages
 * out:
 *    none
 */
void DKCOMMCTX::add_container_volumes_to_backup(bpContext* ctx)
{
   DKINFO *container;
   DKINFO *volume;
   DKINFO *obj;
   char *p, *q;
   POOL_MEM buf(PM_MESSAGE);
   int len;
   bool found;
   alist containerlist(16, not_owned_by_alist);

   DMSG0(ctx, DDEBUG, "add_container_volumes_to_backup called\n");
   /* prepare containers to backup list */
   foreach_alist(container, objs_to_backup){
      if (container->type() == DOCKER_CONTAINER){
         containerlist.append(container);
      }
   }
   /* proceed if any container to backup */
   if (!containerlist.empty()){
      foreach_alist(container, &containerlist){
         DMSG1(ctx, DDEBUG, "processing container: %s\n", container->get_container_id());
         p = container->get_container_mounts();
         if (p != NULL && *p != 0){
            /* the container has mounts, so iterate on them and check volumes to backup */
            len = strlen(p);
            pm_strcpy(buf, p);
            p = buf.c_str();
            while (*p != 0){
               if ((q = strchr(p, ',')) != NULL){
                  *q = 0;           // terminate comma as a string separator
               } else {
                  q = buf.c_str() + len - 1;
               }
               DMSG1(ctx, DDEBUG, "volmount: %s\n", p);
               /* at 'p' we have mounted docker volume name as string
                * check if volume already selected for backup */
               found = false;
               foreach_alist(obj, objs_to_backup){
                  if (obj->type() == DOCKER_VOLUME && bstrcmp(obj->get_volume_name(), p)){
                     found = true;
                     DMSG0(ctx, DDEBUG, "volume found in objs_to_backup, good!\n");
                     break;
                  }
               };
               /* not? simple check in volume list and add it to backup */
               if (!found){
                  foreach_alist(volume, all_volumes){
                     if (bstrcmp(volume->get_volume_name(), p)){
                        /* this volume we should add for backup */
                        objs_to_backup->append(volume);
                        DMSG0(ctx, DDEBUG, "adding volume to backup!\n");
                        break;
                     }
                  };
               }
               /* next in list */
               p = q + 1;
            }
         }
      }
   }

   DMSG0(ctx, DDEBUG, "add_container_volumes_to_backup finish.\n");
};

/*
 * It creates a list of volumes to backup for a particular container
 *  which are selected manually to backup and should be reflected in
 *  catalog database as a volumes links. It is called after 'allvolumes'
 *  parameter verification.
 *
 * in:
 *    bpContext - required for debug/job messages
 * out:
 *    none
 */
void DKCOMMCTX::select_container_vols(bpContext* ctx)
{
   DKINFO *container;
   DKINFO *volume;
   DKVOLS *vols;
   char *p, *q;
   alist vollist(16, not_owned_by_alist);
   int len;
   POOL_MEM buf(PM_MESSAGE);

   DMSG0(ctx, DDEBUG, "select_container_vols called\n");
   /* prepare volume to backup list */
   foreach_alist(volume, objs_to_backup){
      if (volume->type() == DOCKER_VOLUME){
         vollist.append(volume);
      }
   }
   /* proceed if any volume to backup */
   if (!vollist.empty()){
      foreach_alist(container, objs_to_backup){
         if (container->type() == DOCKER_CONTAINER){
            DMSG1(ctx, DDEBUG, "processing container: %s\n", container->get_container_id());
            p = container->get_container_mounts();
            if (p != NULL && *p != 0){
               /* the container has mounts, so iterate on them and check volumes to backup */
               len = strlen(p);
               pm_strcpy(buf, p);
               p = buf.c_str();
               while (*p != 0){
                  if ((q = strchr(p, ',')) != NULL){
                     *q = 0;           // terminate comma as a string separator
                  } else {
                     q = buf.c_str() + len - 1;
                  }
                  DMSG1(ctx, DDEBUG, "volmount: %s\n", p);
                  if (*p != '/'){
                     foreach_alist(volume, &vollist){
                        if (bstrcmp(volume->get_volume_name(), p)){
                           volume->inc_volume_linknr();
                           vols = New(DKVOLS(volume));
                           update_vols_mounts(ctx, container, vols);
                           container->container_append_vols(vols);
                           DMSG0(ctx, DDEBUG, "adding to vols\n");
                           break;
                        }
                     };
                  }
                  /* next param */
                  p = q + 1;
               }
            }
         }
      };
   }
   DMSG0(ctx, DDEBUG, "select_container_vols finish.\n");
};

/*
 * Checks for common Docker error strings.
 *
 * in:
 *    bpContext - required for debug/job messages
 *    buf - the string to scan
 * out:
 *    true - when a common error found
 *    false - no common errors found
 */
bool DKCOMMCTX::check_for_docker_errors(bpContext* ctx, char* buf)
{
   const char *err1 = "Cannot connect to the Docker daemon";
   const char *err2 = "Unable to find image '" BACULATARIMAGE "' locally";
   int len;

   /* no docker running error */
   len = strlen(err1);
   if (strncmp(buf, err1, len) == 0){
      DMSG1(ctx, DERROR, "no docker running error! Err=%s\n", buf);
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "No Docker is running. Cannot continue!\n");
      return true;
   }

   /* cannot find baculatar image */
   len = strlen(err2);
   if (strncmp(buf, err2, len) == 0){
      DMSG1(ctx, DERROR, "cannot find baculatar image! Err=%s\n", buf);
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR,
            "Docker is unable to find required Bacula container backup image. Cannot continue!\n");
      return true;
   }

   return false;
};

/*
 * Sets objs_to_backup list for all containers or images which match the
 *    container/image names based on include_* / exclude_* regex parameters from plugin
 *    command parameters.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    params_include - a list of include regex parameters to match
 *    params_exclude - a list of exclude regex parameters to not match
 *    dklist - a list of containers/images available
 * out:
 *    this->objs_to_backup updated if required
 */
void DKCOMMCTX::filter_incex_to_backup(bpContext* ctx, alist* params_include, alist *params_exclude, alist* dklist)
{
   alist inex_list(16, not_owned_by_alist);
   POOLMEM *expr;
   bool found;
   int options = REG_EXTENDED | REG_ICASE;
   int rc, indx;
   char prbuf[500];
   DKINFO *dkinfo;

   /* prepare a list of objects from include regex */
   if (params_include){
      foreach_alist(expr, params_include){
         DMSG(ctx, DDEBUG, "processing include: %s\n", expr);
         rc = regcomp(&preg, expr, options);
         if (rc != 0) {
            f_error = true;
            regerror(rc, &preg, prbuf, sizeof(prbuf));
            DMSG(ctx, DERROR, "include regex compilation error: %s\n", prbuf);
            JMSG(ctx, is_fatal() ? M_FATAL : M_ERROR, "include_container regex compilation error: %s\n", prbuf);
            continue;
         }
         /* include regex compiled, so iterate through all_containers */
         foreach_alist(dkinfo, dklist){
            rc = regexec(&preg, dkinfo->name(), 0, NULL, 0);
            if (rc == 0){
               /* found */
               inex_list.append(dkinfo);
               DMSG2(ctx, DDEBUG, "include %s found: %s\n", dkinfo->type_str(), dkinfo->name());
            }
         };
         regfree(&preg);
      };
   }

   /* exclude objects from include list using exclude regex */
   if (params_exclude){
      foreach_alist(expr, params_exclude){
         DMSG(ctx, DDEBUG, "processing exclude: %s\n", expr);
         rc = regcomp(&preg, expr, options);
         if (rc != 0) {
            f_error = true;
            regerror(rc, &preg, prbuf, sizeof(prbuf));
            DMSG(ctx, DERROR, "exclude regex compilation error: %s\n", prbuf);
            JMSG(ctx, is_fatal() ? M_FATAL : M_ERROR, "exclude regex compilation error: %s\n", prbuf);
            continue;
         }
         /* iterate through objects list found used params_include */
         found = true;
         while (found){
            foreach_alist(dkinfo, &inex_list){
               DMSG2(ctx, DDEBUG, "exclude processing %s: %s\n", dkinfo->type_str(), dkinfo->name());
               rc = regexec(&preg, dkinfo->name(), 0, NULL, 0);
               if (rc == 0){
                  /* found */
                  indx = inex_list.current() - 1;
                  DMSG(ctx, DVDEBUG, "inex_list_indx: %d\n", indx);
                  inex_list.remove(indx);
                  /* we have to start again as inex_list->cur_item points to the wrong position */
                  DMSG2(ctx, DDEBUG, "exclude %s found: %s\n", dkinfo->type_str(), dkinfo->name());
                  break;
               }
            };
            if (!dkinfo){
               DMSG0(ctx, DDEBUG, "exclude no more objects to check\n");
               found = false;
            }
         }
         regfree(&preg);
      };
   }
   if (inex_list.size()){
      /* move dkinfos to objs_to_backup list */
      foreach_alist(dkinfo, &inex_list){
         objs_to_backup->append(dkinfo);
         DMSG3(ctx, DINFO, "adding %s to backup (2): %s (%s)\n",
               dkinfo->type_str(),
               dkinfo->name(), (char*)dkinfo->id());
      };
   }
};

/*
 * Prepares a DKCOMMCTX class for a single Plugin parameters for backup and estimate jobs.
 *  The main purpose is to set a objs_to_backup list for a list of vms to backup.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    estimate - if the preparation for estimate (true) or backup (false) job
 * out:
 *    bRC_OK - when preparation was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::prepare_bejob(bpContext *ctx, bool estimate)
{
   /* get list of all objects */
   if (!get_all_containers(ctx) || !get_all_images(ctx)){
      return bRC_Error;
   }
   /* when docker_host defined then skip all volumes */
   if (param_docker_host == NULL && !get_all_volumes(ctx)){
      return bRC_Error;
   }

   /* when no volume/container/image/include/exclude parameters found that all objects should be saved */
   if (!param_container && !param_image && !param_include_container && !param_exclude_container
         && !param_include_image && !param_exclude_image && !param_volume){
      set_all_to_backup(ctx);
   } else {
      all_to_backup = false;
      /* find all objects on param_* lists */
      filter_param_to_backup(ctx, param_container, all_containers, estimate);
      filter_param_to_backup(ctx, param_image, all_images, estimate);
      if (param_volume && param_docker_host == NULL){
         filter_param_to_backup(ctx, param_volume, all_volumes, estimate);
      }

      /* handle include/exclude regex for containers and images only */
      filter_incex_to_backup(ctx, param_include_container, param_exclude_container, all_containers);
      filter_incex_to_backup(ctx, param_include_image, param_exclude_image, all_images);

      /* handle allvolumes for containers backup */
      if (all_vols_to_backup && param_docker_host == NULL){
         add_container_volumes_to_backup(ctx);
      }

      /* generate a warning message if required */
      if ((param_volume || all_vols_to_backup) && param_docker_host){
         DMSG0(ctx, DINFO, "Docker Volume backup with docker_host is unsupported!\n");
         JMSG0(ctx, M_WARNING, "Docker Volume backup with docker_host is unsupported!\n");
      }
   }

   select_container_vols(ctx);

   return bRC_OK;
}

/*
 * Prepares a DKCOMMCTX class for a single Plugin parameters for restore job.
 *  The main purpose is to handle storage_res restore parameter.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 * out:
 *    bRC_OK - when preparation was successful
 *    bRC_Error - on any error (unimplemented)
 */
bRC DKCOMMCTX::prepare_restore(bpContext* ctx)
{
   DMSG0(ctx, DDEBUG, "prepare_restore called\n");
   return bRC_OK;
}

/*
 * Setup DKINFO class values based on object type and string parameters from
 *    docker command output.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    type - a docker object type
 *    paramtab - a table of docker command output values
 *    dkinfo - a class to setup
 * out:
 *    dkinfo updated
 */
void DKCOMMCTX::setup_dkinfo(bpContext* ctx, DKINFO_OBJ_t type, char *paramtab[], DKINFO *dkinfo)
{
   switch (type){
      case DOCKER_CONTAINER:
         setup_container_dkinfo(ctx, paramtab, dkinfo);
         break;
      case DOCKER_IMAGE:
         setup_image_dkinfo(ctx, paramtab, dkinfo);
         break;
      case DOCKER_VOLUME:
         setup_volume_dkinfo(ctx, paramtab, dkinfo);
         break;
   }
};

/*
 * Setup DKINFO container class values based on string parameters from docker command output.
 *    It is required to setup a following parameters in paramtab array
 *       [0] - container id
 *       [1] - container name
 *       [2] - container size
 *    other parameters will be ignored.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    paramtab - a table of docker command output values
 *    dkinfo - a class to setup
 * out:
 *    dkinfo updated
 */
void DKCOMMCTX::setup_container_dkinfo(bpContext* ctx, char *paramtab[], DKINFO *dkinfo)
{
   dkinfo->set_container_id(paramtab[0]);
   dkinfo->set_container_names(paramtab[1]);
   dkinfo->scan_container_size(paramtab[2]);
   dkinfo->set_container_mounts(paramtab[3]);
   DMSG3(ctx, DINFO, "setup_container_dkinfo: %s %s %d\n",
         (char*)dkinfo->get_container_id(), dkinfo->get_container_names(), dkinfo->get_container_size());
   DMSG1(ctx, DINFO, "setup_container_dkinfo: %s\n", dkinfo->get_container_mounts());
};

/*
 * Setup DKINFO image class values based on string parameters from docker command output.
 *    It is required to setup a following parameters in paramtab array
 *       [0] - image id
 *       [1] - image repository
 *       [2] - image tag
 *       [3] - image size
 *       [4] - image creation date
 *    other parameters will be ignored.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    paramtab - a table of docker command output values
 *    dkinfo - a class to setup
 * out:
 *    dkinfo updated
 */
void DKCOMMCTX::setup_image_dkinfo(bpContext* ctx, char *paramtab[], DKINFO *dkinfo)
{
   dkinfo->set_image_id(paramtab[0]);
   dkinfo->set_image_repository(paramtab[1]);
   dkinfo->set_image_tag(paramtab[2]);
   dkinfo->scan_image_size(paramtab[3]);
   dkinfo->set_image_created(str_to_utime(paramtab[4]));
   DMSG3(ctx, DINFO, "setup_image_dkinfo: %s %s : %s\n",
         (char*)dkinfo->get_image_id(), dkinfo->get_image_repository(), dkinfo->get_image_tag());
   DMSG2(ctx, DINFO, "setup_image_dkinfo: %d %ld\n", dkinfo->get_image_size(), dkinfo->get_image_created());
};

/*
 * Setup DKINFO volume class values based on string parameters from docker command output.
 *    It is required to setup a following parameters in paramtab array
 *       [0] - volume name
 *       [1] - volume size
 *    other parameters will be ignored.
 *
 * To get a volume created time you have to inspect details of the particular volume:
 * $ docker volume inspect --format "{{.CreatedAt}}" 0a5f9bf16602f2de6c9e701e6fb6d4ce1292ee336348c7c2624f4a08aaacebc4
 * 2019-06-21T17:11:33+02:00
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    paramtab - a table of docker command output values
 *    dkinfo - a class to setup
 * out:
 *    dkinfo updated
 */
void DKCOMMCTX::setup_volume_dkinfo(bpContext* ctx, char *paramtab[], DKINFO *dkinfo)
{
   dkinfo->set_volume_name(paramtab[0]);
   dkinfo->scan_volume_size(paramtab[1]);
   DMSG2(ctx, DINFO, "setup_volume_dkinfo: %s %ld\n",
         dkinfo->get_volume_name(), dkinfo->get_volume_size());
};

/*
 * Setup a list of objects based on docker command execution.
 *    The docker command should format its output into a number of columns separated by a tab character - '/t'
 *    The function support no more then 10 columns to scan. In current implementation we are using no more then 6.
 *    It will setup a list of all docker objects at 'dklist' list based on 'type' of docker objects.
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    cmd - a docker command to execute
 *    cols - a number of columns to scan
 *    dklist - a pointer to dklist variable to update
 *    type - a docker object type to setup
 * out:
 *    dklist - a new list allocated and populated
 */
alist *DKCOMMCTX::get_all_list_from_docker(bpContext* ctx, const char *cmd, int cols, alist **dklist, DKINFO_OBJ_t type)
{
   POOL_MEM out(PM_MESSAGE);
   int a;
   char *paramtab[10];
   int status;
   DKINFO *dkinfo;
   char *p, *q, *t;

   if (cols > 10){
      DMSG1(ctx, DERROR, "BUG! unsupported number of parameter columns: %d\n", cols);
      JMSG1(ctx, M_FATAL, "Unsupported number of parameter columns: %d You should call a support!\n", cols);
      return NULL;
   }
   /* invalid pointer to list*/
   if (!dklist){
      DMSG0(ctx, DERROR, "BUG! invalid pointer to dklist\n");
      return NULL;
   }
   if (!*dklist){
      DMSG0(ctx, DINFO, "get_all_list_from_docker called\n");
      /* first get all containers list */
      if (!execute_command(ctx, cmd)){
         /* some error executing command */
         DMSG0(ctx, DERROR, "get_all_list_from_docker execution error\n");
         return NULL;
      }
      /* allocate a new list */
      *dklist = New(alist(32, not_owned_by_alist));
      memset(out.c_str(), 0, out.size());
      if ((status = read_output(ctx, out)) > 0){
         /* we read a string, so terminate it with nul char */
         p = out.c_str();
         p[status] = 0;
         while (*p != 0 && (q = strchr(p, '\n')) != NULL){
            /* p is the start of the string and q is the end of line */
            *q = 0;     // q+1 will be the next line
            DMSG(ctx, DVDEBUG, "get_all_list_from_docker scanning: %s\n", p);
            if (check_for_docker_errors(ctx, p)){
               goto bailout;        // TODO: remove this goto
            }
            t = p;
            /* expect 5 tabs-separators for 6 parameters handled in-place */
            for (a = 0; a < cols; a++){
               paramtab[a] = t;
               t = strchr(t, '\t');
               if (t != NULL){
                  *t = 0;
                  t++;     // next param
               } else {
                  break;   // finish scanning
               }
            }
            for (a = 0; a < cols; a++){
               DMSG2(ctx, DDEBUG, "get_all_list_from_docker paramtab[%d]: %s\n", a, paramtab[a]);
            }
            /* so, the single line is between ( p ; q ) and consist of 6 nul terminated string parameters */
            dkinfo = New(DKINFO(type));
            setup_dkinfo(ctx, type, paramtab, dkinfo);
            (*dklist)->append(dkinfo);
            if (dkinfo->type() != DOCKER_VOLUME){
               DMSG3(ctx, DDEBUG, "found %s: %s -> %s\n", dkinfo->type_str(), (char*)dkinfo->id(), dkinfo->name());
            } else {
               DMSG2(ctx, DDEBUG, "found %s: %s\n", dkinfo->type_str(), dkinfo->name());
            }
            /* next line */
            DMSG0(ctx, DVDEBUG, "get_all_list_from_docker next line\n");
            p = q + 1;
         }
      } else {
         DMSG0(ctx, DINFO, "get_all_list_from_docker no container found.\n");
      }
      terminate(ctx);
   } else {
      DMSG1(ctx, DINFO, "get_all_list_from_docker used cached data: %p\n", *dklist);
   }

bailout:
   DMSG0(ctx, DINFO, "get_all_list_from_docker finish.\n");
   return *dklist;
}

/*
 * Updates a docker volume mount point in docker list of mounted vols
 *  for proper volume support file (link) name rendering as: <volname> -> </mount/dir>
 *
 * in:
 *    bpContext - for Bacula debug and jobinfo messages
 *    dkinfo - the container to scan
 *    dkvols - the volume mount information to update
 * out:
 *    none
 */
void DKCOMMCTX::update_vols_mounts(bpContext* ctx, DKINFO *container, DKVOLS *volume)
{
   POOL_MEM out(PM_MESSAGE);
   POOL_MEM cmd(PM_MESSAGE);
   int status;
   char *p, *q, *t;

   DMSG0(ctx, DINFO, "update_volume_mounts called\n");
   if (container && volume){
      /* get details about container mounts */
      Mmsg(cmd, "container inspect --format '{{range .Mounts}}{{.Name}}{{print \"\\t\"}}{{println .Destination}}{{end}}' %s", container->get_container_id());
      if (!execute_command(ctx, cmd)){
         /* some error executing command */
         DMSG0(ctx, DERROR, "update_volume_mounts execution error\n");
         return;
      }
      /* process data:
       * aa9d3074f8c65a5afafddc6eaaf9827e99bb51f676aafaacc05cfca0188e65bf	/var/log\n
       * 9194f415cafcf8d234673478f3358728d43e0203e58d0338b4ee18a4dca6646b	/etc/logrotate.d\n
       * (...)
       */
      if ((status = read_output(ctx, out)) > 0){
         /* we read a string, so terminate it with nul char */
         p = out.c_str();
         p[status] = 0;
         while (*p != 0 && (q = strchr(p, '\n')) != NULL){
            /* p is the start of the string and q is the end of line */
            *q = 0;     // q+1 will be the next line
            DMSG(ctx, DVDEBUG, "update_volume_mounts scanning: %s\n", p);
            if (check_for_docker_errors(ctx, p)){
               return;
            }
            /* expect 1 tab-separator for 2 parameters handled in-place */
            t = strchr(p, '\t');
            if (t != NULL){
               *t++ = 0;
            } else {
               /* scan error */
               return;
            }
            DMSG2(ctx, DDEBUG, "update_volume_mounts volname: %s dest: %s\n", p, t);
            if (bstrcmp(volume->vol->get_volume_name(), p)){
               /* this is the volume we are looking for */
               pm_strcpy(volume->destination, t);
               return;
            }
            /* next line */
            DMSG0(ctx, DVDEBUG, "get_all_list_from_docker next line\n");
            p = q + 1;
         }
      } else {
         DMSG0(ctx, DINFO, "get_all_list_from_docker no container found.\n");
      }
      terminate(ctx);
   } else {
      DMSG2(ctx, DERROR, "invalid parameters: c:%p v:%p\n", container, volume);
      return;
   }
   DMSG0(ctx, DINFO, "update_volume_mounts finish.\n");
}

/*
 * Return a list of all containers available on Docker.
 *
 * the container list is described as the following text block:
# docker ps -a --no-trunc=true --format "{{.ID}}\t{{.Names}}\t{{.Size}}\t{{.Mounts}}\t{{.Labels}}\t{{.Image}}"
66f45d8601bae26a6b2ffeb46922318534d3b3905377b3a224693bd78601cb3b	brave_edison	0B (virtual 228MB)	c0a478d317195ba27dda1370b73e5cb94a7773f2a611142d7dff690abdcfdcbf		postgres\n

 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 * out:
 *    NULL - on any error
 *    empty alist - when no docker comntainers found
 *    alist - a list of DKINFO class which describe a single docker container
 */
alist *DKCOMMCTX::get_all_containers(bpContext* ctx)
{
   return get_all_list_from_docker(ctx,
         "ps -a --no-trunc=true --format \"{{.ID}}\\t{{.Names}}\\t{{.Size}}\\t{{.Mounts}}\\t{{.Labels}}\\t{{.Image}}\"",
         6, &all_containers, DOCKER_CONTAINER);
};

/*
 * Return a list of all images available on Docker.
 *
 * the images list is described as the following text block:
# # docker image ls --no-trunc=true --format "{{.Repository}}\t{{.Tag}}\t{{.ID}}\t{{.Size}}"
restore/a6ba1cb597d5	latest	sha256:44c34a8a510dc08b7b0a8f6961257e89120152739e61611f564353e8feb95e68	319MB\n

 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 * out:
 *    NULL - on any error
 *    empty alist - when no vms found
 *    alist - a list of DKINFO class which describe a single VM
 */
alist *DKCOMMCTX::get_all_images(bpContext* ctx)
{
   return get_all_list_from_docker(ctx,
         "image ls --no-trunc=true --format \"{{.ID}}\\t{{.Repository}}\\t{{.Tag}}\\t{{.Size}}\\t{{.CreatedAt}}\"",
         5, &all_images, DOCKER_IMAGE);
}

/*
 * Return a list of all volumes available on Docker.
 *
 * the volumes list is described as the following text block:
# # docker volume ls --format "{{.Name}}\t{{.Size}}"
0a5f9bf16602f2de6c9e701e6fb6d4ce1292ee336348c7c2624f4a08aaacebc4	N/A\n

 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 * out:
 *    NULL - on any error
 *    empty alist - when no vms found
 *    alist - a list of DKINFO class which describe a single VM
 */
alist *DKCOMMCTX::get_all_volumes(bpContext* ctx)
{
   return get_all_list_from_docker(ctx,
         "volume ls --format \"{{.Name}}\\t{{.Size}}\"", 2, &all_volumes, DOCKER_VOLUME);
}

/*
 * Rename/set the docker image tag on selected image.
 */
bRC DKCOMMCTX::docker_tag(bpContext* ctx, DKID &dkid, POOLMEM *tag)
{
   bRC rc = bRC_OK;
   POOL_MEM cmd(PM_FNAME);
   POOL_MEM out(PM_BSOCK);
   int status;

   DMSG0(ctx, DINFO, "docker_tag called.\n");
   if (!tag){
      DMSG0(ctx, DERROR, "docker_tag tag is NULL!\n");
      return bRC_Error;
   }
   Mmsg(cmd, "image tag %s %s", (char*)dkid, tag);
   DMSG1(ctx, DDEBUG, "%s\n", cmd.c_str());
   if (!execute_command(ctx, cmd)){
      /* some error executing command */
      DMSG0(ctx, DERROR, "docker_tag execution error\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "docker_tag execution error\n");
      return bRC_Error;
   }

   memset(out.c_str(), 0, out.size());
   status = read_output(ctx, out);
   if (status < 0){
      /* error reading data from docker command */
      DMSG0(ctx, DERROR, "docker_tag error reading data from docker command\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "docker_tag error reading data from docker command\n");
      rc = bRC_Error;
   } else
   if (status > 0 && check_for_docker_errors(ctx, out.c_str())){
      rc = bRC_Error;
   }

   /* we do not expect any output here */
   terminate(ctx);
   DMSG0(ctx, DINFO, "docker_tag finish.\n");
   return rc;
};

/*
 * Waits for restore commands to finish.
 *  Closes a BPIPE write descriptor which means EOF to command tool. Then try
 *  to read from tools output and checks if restore was successful or not.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 * out:
 *    bRC_OK - restore was successful and vmuuid is filled with VM UUID restored.
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::wait_for_restore(bpContext *ctx, DKID &dkid)
{
   POOL_MEM out(PM_BSOCK);
   POOL_MEM buf(PM_BSOCK);
   int status;
   char *p;
   bRC rc = bRC_OK;

   DMSG0(ctx, DINFO, "wait_for_restore called.\n");
   /* first flush any outstanding restore data and close write descriptor */
   close_wpipe(bpipe);
   /* now read the status from command */
   while ((status = read_output(ctx, out)) != 0){
      if (status < 0){
         /* error reading data from command tool */
         DMSG0(ctx, DERROR, "error reading data from command tool\n");
         rc = bRC_Error;
         goto bailout;
      }
      pm_strcat(buf, out);
      p = buf.c_str();
      p[status] = 0;
   }

   /* check for errors */
   DMSG1(ctx, DVDEBUG, "bufout: %s\n", buf.c_str());
   p = buf.c_str();
   if (strstr(p, "Loaded image ID: ") == NULL){
      /* error, missing confirmation*/
      DMSG0(ctx, DERROR, "wait_for_restore confirmation error!\n");
      JMSG1(ctx, abort_on_error ? M_FATAL : M_ERROR, "Image restore commit error: %s\n", p);
      rc = bRC_Error;
   } else {
      dkid = (char*)(p+17);
      DMSG1(ctx, DDEBUG, "scanned dkid: %s\n", (char*)dkid);
   }

bailout:
   terminate(ctx);
   DMSG0(ctx, DINFO, "wait_for_restore finish.\n");
   return rc;
}

/*
 * Executes docker command tool to make a container image commit.
 *    Setup dkinfo->data.container.imagesave with image id of newly created image
 *    which should be used at image save method.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    dkinfo - container for commit
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::container_commit(bpContext* ctx, DKINFO *dkinfo, int jobid)
{
   POOL_MEM cmd(PM_FNAME);
   POOL_MEM imagename(PM_FNAME);
   POOL_MEM out(PM_MESSAGE);
   const char *mode = "";
   const char *PAUSE = "-p";
   DKID imagesave;
   bRC rc = bRC_OK;
   int status;
   char *p;

   DMSG0(ctx, DINFO, "container_commit called.\n");
   if (dkinfo->type() != DOCKER_CONTAINER){
      /* commit works on containers only */
      return bRC_Error;
   }
   if (param_mode == DKPAUSE){
      mode = PAUSE;
   }
   // commit -p 66f45d8601bae26a6b2ffeb46922318534d3b3905377b3a224693bd78601cb3b mcache1/66f45d8601ba:backup
   render_imagesave_name(imagename, dkinfo, jobid);
   Mmsg(cmd, "commit %s %s %s", mode, (char*)dkinfo->get_container_id(), imagename.c_str());
   if (!execute_command(ctx, cmd)){
      /* some error executing command */
      DMSG0(ctx, DERROR, "container_commit execution error\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "container_commit execution error\n");
      return bRC_Error;
   }

   memset(out.c_str(), 0, out.size());
   status = read_output(ctx, out);
   if (status < 0){
      /* error reading data from docker command */
      DMSG0(ctx, DERROR, "container_commit error reading data from docker command\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "container_commit error reading data from docker command\n");
      rc = bRC_Error;
   } else {
      /* terminate committed image id string */
      p = out.c_str();
      p[status] = 0;
      strip_trailing_junk(out.c_str());

      /* check a known output error */
      if (status > 0 && check_for_docker_errors(ctx, out.c_str())){
         rc = bRC_Error;
      } else {
         // should return a string: sha256:290835d692069c376072061362cb11b1e99efd555f6fb83b7be3e524ba6067fc
         imagesave = p;
         if (imagesave.id() < 0){
            /* error scanning image id */
            DMSG1(ctx, DERROR, "container_commit cannot scan commit image id. Err=%s\n", p);
            JMSG1(ctx, abort_on_error ? M_FATAL : M_ERROR, "container_commit cannot scan commit image id. Err=%s\n", p);
            rc = bRC_Error;
         } else {
            dkinfo->set_container_imagesave(imagesave);
            dkinfo->set_container_imagesave_tag(imagename);
            DMSG(ctx, DINFO, "Commit created: %s\n", dkinfo->get_container_imagesave_tag());
            JMSG(ctx, M_INFO, "Commit created: %s\n", dkinfo->get_container_imagesave_tag());
         }
      }
   }

   terminate(ctx);
   DMSG0(ctx, DINFO, "container_commit finish.\n");
   return rc;
}

/*
 * Executes docker command tool to make a container image commit.
 *    Setup dkinfo->data.container.imagesave with image id of newly created image
 *    which should be used at image save method.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    dkinfo - container for commit
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::delete_container_commit(bpContext* ctx, DKINFO *dkinfo, int jobid)
{
   POOL_MEM cmd(PM_FNAME);
   POOL_MEM imagename(PM_FNAME);
   POOL_MEM out(PM_MESSAGE);
   DKID imagesave;
   bRC rc = bRC_OK;
   int status;
   char *p, *q;
   int noerror = 0;

   DMSG0(ctx, DINFO, "delete_container_commit called.\n");
   if (dkinfo->type() != DOCKER_CONTAINER){
      /* commit works on containers only, silently ignore images */
      return bRC_OK;
   }

   if (dkinfo->get_container_imagesave()->id() > 0){
      /* container has commit image */
      /*
       # docker rmi e7cd2a7f1c52a1fa8d88ab812abdcd814064e4884a12bd1f9acde16133023a69
       Untagged: mcache1/66f45d8601ba/123:backup
       Deleted: sha256:e7cd2a7f1c52a1fa8d88ab812abdcd814064e4884a12bd1f9acde16133023a69
       */

      Mmsg(cmd, "rmi %s", (char*)dkinfo->get_container_imagesave());
      if (!execute_command(ctx, cmd)){
         /* some error executing command */
         DMSG0(ctx, DERROR, "delete_container_commit execution error\n");
         JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "delete_container_commit execution error\n");
         return bRC_Error;
      }

      memset(out.c_str(), 0, out.size());
      status = read_output(ctx, out);
      if (status < 0){
         /* error reading data from docker command */
         DMSG0(ctx, DERROR, "delete_container_commit error reading data from docker command\n");
         JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR,
               "delete_container_commit error reading data from docker command\n");
         rc = bRC_Error;
         goto bailout;
      }

      /* terminate output string */
      p = out.c_str();
      p[status] = 0;

      /* check a known output error */
      if (status > 0 && (strncmp(out.c_str(), "Cannot connect to the Docker daemon", 35) == 0)){
         DMSG1(ctx, DERROR, "No Docker is running. Cannot continue! Err=%s\n", out.c_str());
         JMSG1(ctx, abort_on_error ? M_FATAL : M_ERROR, "No Docker is running. Err=%s\n", out.c_str());
         rc = bRC_Error;
         goto bailout;
      }

      render_imagesave_name(imagename, dkinfo, jobid);

      /* it should return the following string:
       Untagged: mcache1/66f45d8601ba:backup\n
       Deleted: sha256:e7cd2a7f1c52a1fa8d88ab812abdcd814064e4884a12bd1f9acde16133023a69\n
       */
      while (*p != 0 && (q = strchr(p, '\n')) != NULL){
         /* p is the start of the string and q is the end of line */
         *q = 0;
         DMSG(ctx, DVDEBUG, "delete_container_commit scanning: %s\n", p);
         if (strstr(p, "Untagged: ") == p && strstr(p, imagename.c_str()) != NULL){
            /* above message means 1/3 of success */
            noerror++;
         }
         if (strstr(p, "Deleted: ") == p){
            /* check if it deleted what we requested for */
            noerror++;
            imagesave = (char*)(p + 9);
            if (imagesave == *dkinfo->get_container_imagesave()){
               /* yes it deleted exactly what we are requesting for */
               noerror++;
            }
         }
         /* next line */
         DMSG0(ctx, DVDEBUG, "delete_snapshot next line\n");
         p = q + 1;
      }

      if (noerror < 3){
         /* error deleting snapshot */
         strip_trailing_junk(out.c_str());
         DMSG(ctx, DERROR, "Error deleting commit image. Err=%s\n", out.c_str());
         JMSG(ctx, abort_on_error ? M_FATAL : M_ERROR, "Error deleting commit image. Err=%s\n", out.c_str());
         rc = bRC_Error;
         goto bailout;
      }

      DMSG(ctx, DINFO, "Commit removed: %s\n", dkinfo->get_container_imagesave_tag());
      JMSG(ctx, M_INFO, "Commit removed: %s\n", dkinfo->get_container_imagesave_tag());

bailout:
      terminate(ctx);
   } else {
      DMSG0(ctx, DINFO, "container_commit no imagesave available.\n");
   }

   DMSG0(ctx, DINFO, "container_commit finish.\n");
   return rc;
}

/*
 * Executes docker command tool to save docker image.
 *    The data to backup is generated on docker stdout channel and will be saved
 *    on pluginIO calls.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    dkid - docker image to save information
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::image_save(bpContext* ctx, DKID *dkid)
{
   POOL_MEM cmd(PM_FNAME);

   DMSG0(ctx, DINFO, "image_save called.\n");
   Mmsg(cmd, "save %s", (char*)dkid);
   if (!execute_command(ctx, cmd)){
      /* some error executing command */
      DMSG0(ctx, DERROR, "image_save execution error\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "image_save execution error\n");
      return bRC_Error;
   }
   DMSG0(ctx, DINFO, "image_save finish, now we can read all the data.\n");

   return bRC_OK;
}

/*
 * It runs a Bacula Archive container for Docker Volume files data backup.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    cmd - the command for Bacula Archive container execution ('backup' and 'restore' are supported)
 *    volname - a volume name to backup from or restore to
 *    jobid - required for proper support volume creation
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::run_container_volume_cmd(bpContext* ctx, const char *cmd, POOLMEM *volname, int jobid)
{
   POOL_MEM bactarcmd(PM_FNAME);
   POOL_MEM out(PM_MESSAGE);
   int status;
   char *p;

   DMSG1(ctx, DINFO, "run_container_volume_cmd called: %s.\n", cmd);
   if (workingvolume == NULL && prepare_working_volume(ctx, jobid) != bRC_OK){
      return bRC_Error;
   }
   /* Here we will run archive container for volume backup */
   Mmsg(bactarcmd, "run -d --rm -v %s:/%s -v %s:/logs %s %s", volname, cmd, workingvolume, BACULATARIMAGE, cmd);
   if (!execute_command(ctx, bactarcmd)){
      /* some error executing command */
      DMSG0(ctx, DERROR, "run_container_volume_cmd execution error\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "run_container_volume_cmd execution error\n");
      return bRC_Error;
   }

   /* setup output buffer */
   memset(out.c_str(), 0, out.size());
   status = read_output(ctx, out);
   if (status < 0){
      /* error reading data from docker command */
      DMSG0(ctx, DERROR, "run_container_volume_cmd error reading data from docker command\n");
      JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR, "run_container_volume_cmd error reading data from docker command\n");
      return bRC_Error;
   }

   /* terminate container id string */
   p = out.c_str();
   p[status] = 0;
   strip_trailing_junk(out.c_str());

   /* check a known output error */
   if (status > 0 && check_for_docker_errors(ctx, out.c_str())){
      return bRC_Error;
   }

   DMSG2(ctx, DINFO, "run_container_volume_cmd finish - acc: %s, now we can %s all the data.\n", out.c_str(), cmd);

   return bRC_OK;
}

/*
 * Execute a Bacula Archive Container for volume backup.
 *
 * in:
 *    bpContext - required for debug/job messages
 *    volname - a volume name to backup from
 *    jobid - required for proper support volume creation
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::run_container_volume_save(bpContext* ctx, POOLMEM* volname, int jobid)
{
   return run_container_volume_cmd(ctx, "backup", volname, jobid);
};

/*
 * Execute a Bacula Archive Container for volume restore.
 *
 * in:
 *    bpContext - required for debug/job messages
 *    volname - a volume name to restore to
 *    jobid - required for proper support volume creation
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::run_container_volume_load(bpContext* ctx, POOLMEM* volname, int jobid)
{
   return run_container_volume_cmd(ctx, "restore", volname, jobid);
};

/*
 * Execute Docker commands to perform backup procedure.
 *    Commit container then save committed image for container backup
 *    or simply save docker image for image backup.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    dkinfo - the docker object to backup
 *    jobid - bacula jobid number
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::backup_docker(bpContext *ctx, DKINFO *dkinfo, int jobid)
{
   DMSG0(ctx, DINFO, "backup_docker called.\n");
   switch (dkinfo->type()){
      case DOCKER_CONTAINER:
         /* create container commit */
         if (container_commit(ctx, dkinfo, jobid) == bRC_OK){
            if (dkinfo->get_container_imagesave()->id() > 0){
               return image_save(ctx, dkinfo->get_container_imagesave());
            }
         }
         break;
      case DOCKER_IMAGE:
         return image_save(ctx, dkinfo->get_image_id());
      case DOCKER_VOLUME:
         return run_container_volume_save(ctx, dkinfo->get_volume_name(), jobid);
      default:
         break;
   }
   DMSG0(ctx, DINFO, "backup_docker finish with error.\n");
   return bRC_Error;
};

/*
 * Executes Docker commands to perform restore.
 *  The data to restore is gathered on command stdin channel and will be sent
 *  on pluginIO calls.
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 * out:
 *    bRC_OK - when command execution was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::restore_docker(bpContext *ctx, DKINFO *dkinfo, int jobid)
{
   DMSG0(ctx, DINFO, "restore_docker called.\n");
   if (dkinfo != NULL && dkinfo->type() == DOCKER_VOLUME){
      return run_container_volume_load(ctx, dkinfo->get_volume_name(), jobid);
   } else {
      if (!execute_command(ctx, "load")){
         /* some error executing command */
         DMSG0(ctx, DERROR, "restore_docker execution error\n");
         return bRC_Error;
      }
   }
   DMSG0(ctx, DINFO, "restore_docker finish, now we can write the data.\n");
   return bRC_OK;
};

/*
 * Create or run docker container based on restored image.
 *
 * # docker container create --name mcache1_59 mcache1/b97d4dd88063/59:restore
 *
 * in:
 *    bpContext - for Bacula debug jobinfo messages
 *    dkinfo - restored image for container creation or run
 * out:
 *    bRC_OK - when creation/run was successful
 *    bRC_Error - on any error
 */
bRC DKCOMMCTX::docker_create_run_container(bpContext* ctx, DKINFO *dkinfo)
{
   POOL_MEM cmd(PM_FNAME);
   POOL_MEM out(PM_BSOCK);
   bRC rc = bRC_OK;
   int status;
   char *p;
   char *imagelabel;
   const char *namepar;
   const char *nameval;
   DKID containerid;

   if (!param_container_create && !param_container_run){
      DMSG0(ctx, DINFO, "docker_create_container skipped on request.\n");
      return bRC_OK;
   }
   DMSG0(ctx, DINFO, "docker_create_container called.\n");
   if (dkinfo){
      imagelabel = param_container_imageid ? (char*)dkinfo->get_container_imagesave() : dkinfo->get_container_imagesave_tag();
      namepar = param_container_defaultnames ? "" : "--name ";
      nameval = param_container_defaultnames ? "" : dkinfo->get_container_names();
      if (param_container_run){
         // create and run the container
         Mmsg(cmd, "run -d %s%s %s", namepar, nameval, imagelabel);
      } else {
         // create only
         Mmsg(cmd, "container create %s%s %s", namepar, nameval, imagelabel);
      }
      if (!execute_command(ctx, cmd.c_str())){
         /* some error executing command */
         DMSG0(ctx, DERROR, "docker_create_container execution error\n");
         return bRC_Error;
      }

      memset(out.c_str(), 0, out.size());
      status = read_output(ctx, out);
      if (status < 0){
         /* error reading data from docker command */
         DMSG0(ctx, DERROR, "docker_create_container error reading data from docker command\n");
         JMSG0(ctx, abort_on_error ? M_FATAL : M_ERROR,
               "docker_create_container error reading data from docker command\n");
         rc = bRC_Error;
         goto bailout;
      }

      /* terminate committed image id string */
      p = out.c_str();
      p[status] = 0;
      strip_trailing_junk(out.c_str());

      /* check a known output error */
      if (status > 0 && (strncmp(out.c_str(), "Cannot connect to the Docker daemon", 35) == 0)){
         DMSG1(ctx, DERROR, "No Docker is running. Cannot continue! Err=%s\n", out.c_str());
         JMSG1(ctx, abort_on_error ? M_FATAL : M_ERROR, "No Docker is running. Err=%s\n", out.c_str());
         rc = bRC_Error;
         goto bailout;
      }

      // should return a string like: 5dd2e72fd9981184ddb8b04aaea06003617fd3f09ad0764921694e20be680c54
      containerid = p;
      if (containerid.id() < 0){
         /* error scanning container id */
         DMSG1(ctx, DERROR, "docker_create_container cannot scan commit image id. Err=%s\n", p);
         JMSG1(ctx, abort_on_error ? M_FATAL : M_ERROR,
               "docker_create_container cannot scan commit image id. Err=%s\n", p);
         rc = bRC_Error;
         goto bailout;
      } else {
         dkinfo->set_container_id(containerid);
         if (param_container_run){
            DMSG1(ctx, DINFO, "docker_create_container successfully run container as: %s\n", (char*)containerid);
            JMSG1(ctx, M_INFO, "Successfully run container as: (%s)\n", containerid.digest_short());
         }
      }
   }

bailout:
   terminate(ctx);
   DMSG0(ctx, DINFO, "docker_create_container finish.\n");
   return rc;
};
