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

#include "task.h"
#include "jcr.h"
#define dbglvl 10
int authenticate_daemon(JCR *jcr, MONITOR *monitor, RESMON *res);

static void *handle_task(void *data)
{
   task   *t;
   worker *wrk = (worker *)data;
   lmgr_init_thread();
   
   wrk->set_running();
   Dmsg0(dbglvl, "Worker started\n");

   while (!wrk->is_quit_state()) {
      if (wrk->is_wait_state()) {
         wrk->wait();
         continue;
      }   
      t = (task *)wrk->dequeue();
      if (!t) {
         continue;
      }
      /* Do the work */
      switch(t->type) {
      case TASK_STATUS:
         t->do_status();
         break;
      case TASK_RESOURCES:
         t->get_resources();
         break;
      case TASK_DEFAULTS:
         t->get_job_defaults();
         break;
      case TASK_RUN:
         t->run_job();
         break;
      case TASK_BWLIMIT:
         t->set_bandwidth();
         break;
      case TASK_INFO:
         t->get_job_info(t->arg2);
         break;
      case TASK_DISCONNECT:
         t->disconnect_bacula();
         t->mark_as_done();
         break;
      default:
         Mmsg(t->errmsg, "Unknown task");
         t->mark_as_failed();
         break;
      }
   }
   Dmsg0(dbglvl, "Worker stoped\n");
   lmgr_cleanup_thread();
   return NULL;
}

bool task::set_bandwidth()
{
   bool ret = false;
   btimer_t *tid = NULL;
   if (res->type != R_CLIENT) {
      mark_as_failed();
      Mmsg(errmsg, _("Bandwidth can set only set on Client"));
      return false;
   }
   if (!arg || !*arg) {
      mark_as_failed();
      Mmsg(errmsg, _("Bandwidth parameter is invalid"));
      return false;
   }

   if (res->proxy_sent) {
      free_bsock(res->bs);
   }

   if (!res->bs || !res->bs->is_open() || res->bs->is_error()) {
      if (!connect_bacula()) { 
         mark_as_failed();
         return false;
      }
   }

   tid = start_thread_timer(NULL, pthread_self(), (uint32_t)120);
   res->bs->fsend("setbandwidth limit=%s\n", NPRTB(arg));
   while (get_next_line(res)) {
      Dmsg1(dbglvl, "-> %s\n", curline);
   }

   if (tid) {
      stop_thread_timer(tid);
   }

   /* Do not reuse the same socket */
   disconnect_bacula();

   if (ret) {
      mark_as_done();
   } else {
      mark_as_failed();
   }
   return ret;
}

RESMON *task::get_res()
{
   return res;
}

void task::lock_res()
{
   res->mutex->lock();
}

void task::unlock_res()
{
   res->mutex->unlock();
}

bool task::disconnect_bacula()
{
   free_bsock(res->bs);
   return true;
}

bool task::connect_bacula()
{
   JCR jcr;
   bool ret = false;
   memset(&jcr, 0, sizeof(jcr));
   curend = curline = NULL;

   RESMON *r = get_res();
   MONITOR *monitor = (MONITOR*)GetNextRes(R_MONITOR, NULL);

   if (r->type == R_CLIENT) {
      r->proxy_sent = false;
      if (r->bs && (r->bs->is_error() || !r->bs->is_open())) {
         free_bsock(r->bs);
      }
      if (!r->bs) {
         r->bs = new_bsock();
         Dmsg0(dbglvl, "Trying to connect to FD\n");
         if (r->bs->connect(NULL, r->connect_timeout, 0, 0, _("Client daemon"),
                            r->address, NULL, r->port, 0))
         {
            Dmsg0(dbglvl, "Connect done!\n");
            jcr.file_bsock = r->bs;
            if (!authenticate_daemon(&jcr, monitor, r)) {
               Dmsg0(dbglvl, "Unable to authenticate\n");
               Mmsg(errmsg, "Unable to authenticate with the FileDaemon");
               free_bsock(r->bs);
               return false;
            }
            Dmsg0(dbglvl, "Authenticate OK\n");
            ret = true;
         } else {
            Mmsg(errmsg, "Unable to connect to the FileDaemon");
            Dmsg0(dbglvl, "Connect error!\n");
         }
      } else {
         ret = true;
      }
   }
   if (r->type == R_STORAGE) {
      if (r->bs && (r->bs->is_error() || !r->bs->is_open())) {
         free_bsock(r->bs);
      }
      if (!r->bs) {
         r->bs = new_bsock();
         Dmsg0(dbglvl, "Trying to connect to FD\n");
         if (r->bs->connect(NULL, r->connect_timeout, 0, 0, _("Storage daemon"),
                            r->address, NULL, r->port, 0))
         {
            Dmsg0(dbglvl, "Connect done!\n");
            jcr.store_bsock = r->bs;
            if (!authenticate_daemon(&jcr, monitor, r)) {
               Dmsg0(dbglvl, "Unable to authenticate\n");
               Mmsg(errmsg, "Unable to authenticate with the Storage Daemon");
               free_bsock(r->bs);
               return false;
            }
            Dmsg0(dbglvl, "Authenticate OK\n");
            ret = true;
         } else {
            Mmsg(errmsg, "Unable to connect to the Storage Daemon");
            Dmsg0(dbglvl, "Connect error!\n");
         }
      } else {
         ret = true;
      }
   }
   if (r->type == R_DIRECTOR) {
      if (r->bs && (r->bs->is_error() || !r->bs->is_open())) {
         free_bsock(r->bs);
      }
      if (!r->bs) {
         r->bs = new_bsock();
         Dmsg0(dbglvl, "Trying to connect to DIR\n");
         if (r->bs->connect(NULL, r->connect_timeout, 0, 0, _("Director daemon"),
                            r->address, NULL, r->port, 0))
         {
            Dmsg0(dbglvl, "Connect done!\n");
            jcr.dir_bsock = r->bs;
            if (!authenticate_daemon(&jcr, monitor, r)) {
               Dmsg0(dbglvl, "Unable to authenticate\n");
               Mmsg(errmsg, "Unable to authenticate with the Director");
               free_bsock(r->bs);
               return false;
            }
            Dmsg0(dbglvl, "Authenticate OK\n");
            ret = true;
         } else {
            Mmsg(errmsg, "Unable to connect to the Director");
            Dmsg0(dbglvl, "Connect error!\n");
         }
      } else {
         ret = true;
      }
   }
   return ret;
}

bool task::read_status_running(RESMON *r)
{
   bool ret = false;
   char *start, *end;
   struct s_running_job *item = NULL;
   alist *running_jobs = New(alist(10, owned_by_alist));
   
   while (r->bs->recv() >= -1) {
      if (r->bs->msglen < 0 &&
          r->bs->msglen != BNET_CMD_BEGIN &&
          r->bs->msglen != BNET_CMD_OK)
      {
         Dmsg1(dbglvl, "Got Signal %s\n", bnet_sig_to_ascii(r->bs->msglen));
         break;
      }
      Dmsg2(dbglvl, "RECV -> %s:%d\n", r->bs->msg, r->bs->msglen);
      start = r->bs->msg;

      while ((end = strchr(start, '\n')) != NULL) {
         *end = 0;
         Dmsg1(dbglvl, "line=[%s]\n", start);
         if (strncasecmp(start, "jobid=", 6) == 0) {
            if (item) {
               Dmsg1(dbglvl, "Append item %ld\n", item->JobId);
               running_jobs->append(item);
            }
            item = (struct s_running_job *)malloc(sizeof(struct s_running_job));
            memset(item, 0, sizeof(struct s_running_job));
            item->JobId = str_to_uint64(start + 6);

         } else if (!item) {
            Dmsg0(dbglvl, "discard line\n");

         } else if (strncasecmp(start, "level=", 6) == 0) {
            item->JobLevel = start[6];

         } else if (strncasecmp(start, "type=", 5) == 0) {
            item->JobType = start[5];

         } else if (strncasecmp(start, "status=", 7) == 0) {
            item->JobStatus = start[7];

         } else if (strncasecmp(start, "jobbytes=", 9) == 0) {
            item->JobBytes = str_to_uint64(start + 9);

         } else if (strncasecmp(start, "jobfiles=", 9) == 0) {
            item->JobFiles = str_to_uint64(start + 9);

         } else if (strncasecmp(start, "job=", 4) == 0) {
            bstrncpy(item->Job, start + 4, sizeof(item->Job));

         } else if (strncasecmp(start, "starttime_epoch=", 16) == 0) {
            item->start_time = str_to_uint64(start + 16);

         } else if (strncasecmp(start, "schedtime_epoch=", 16) == 0) {
            item->sched_time = str_to_uint64(start + 16);

         } else if (strncasecmp(start, "bytes/sec=", 10) == 0) {
            item->bytespersec = str_to_uint64(start + 10);

         } else if (strncasecmp(start, "avebytes_sec=", 13) == 0) {
            item->bytespersec = str_to_uint64(start + 13);

         } else if (strncasecmp(start, "errors=", 7) == 0) {
            item->Errors = str_to_uint64(start + 7);

         } else if (strncasecmp(start, "readbytes=", 10) == 0) {
            item->ReadBytes = str_to_uint64(start + 10);

         } else if (strncasecmp(start, "processing file=", 16) == 0) {
            bstrncpy(item->CurrentFile, start + 16, sizeof(item->CurrentFile));

         } else if (strncasecmp(start, "clientname=", 11) == 0) {
            bstrncpy(item->Client, start + 11, sizeof(item->Client));

         } else if (strncasecmp(start, "fileset=", 8) == 0) {
            bstrncpy(item->FileSet, start + 8, sizeof(item->FileSet));

         } else if (strncasecmp(start, "storage=", 8) == 0) {
            bstrncpy(item->Storage, start + 8, sizeof(item->Storage));

         } else if (strncasecmp(start, "rstorage=", 8) == 0) {
            bstrncpy(item->RStorage, start + 8, sizeof(item->Storage));

         } else if (strncasecmp(start, "sdtls=", 6) == 0) {
            item->SDtls = str_to_uint64(start + 6);
         }
         start = end+1;
      }
      r->last_update = time(NULL);

      if (r->bs->is_error()) {
         Mmsg(errmsg, "Got error on the socket communication line");
         goto bail_out;
      } 
   }
   if (item) {
      Dmsg1(dbglvl, "Append item %ld\n", item->JobId);
      running_jobs->append(item);
   }
   ret = true;

bail_out:
   r->mutex->lock();
   if (r->running_jobs) {
      delete r->running_jobs;
   }
   r->running_jobs = running_jobs;
   r->mutex->unlock();

   return ret;
}

bool task::read_status_terminated(RESMON *r)
{
   bool ret = false;
   char *start, *end;
   struct s_last_job *item = NULL;

   r->mutex->lock();
   if (r->terminated_jobs) {
      delete r->terminated_jobs;
   }
   r->terminated_jobs = New(dlist(item, &item->link));
   r->mutex->unlock();

   while (r->bs->recv() >= -1) {
      if (r->bs->msglen < 0 &&
          r->bs->msglen != BNET_CMD_BEGIN &&
          r->bs->msglen != BNET_CMD_OK)
      {
         Dmsg1(dbglvl, "Got Signal %s\n", bnet_sig_to_ascii(r->bs->msglen));
         break;
      }

      Dmsg2(dbglvl, "RECV -> %s:%d\n", r->bs->msg, r->bs->msglen);
      r->mutex->lock();
      start = r->bs->msg;

      while ((end = strchr(start, '\n')) != NULL) {
         *end = 0;
         Dmsg1(dbglvl, "line=[%s]\n", start);
         if (strncasecmp(start, "jobid=", 6) == 0) {
            if (item) {
               Dmsg1(dbglvl, "Append item %ld\n", item->JobId);
               r->terminated_jobs->append(item);
            }
            item = (struct s_last_job *)malloc(sizeof(struct s_last_job));
            memset(item, 0, sizeof(struct s_last_job));
            item->JobId = str_to_uint64(start + 6);

         } else if (!item) {
            Dmsg0(dbglvl, "discard line\n");

         } else if (strncasecmp(start, "level=", 6) == 0) {
            item->JobLevel = start[6];

         } else if (strncasecmp(start, "type=", 5) == 0) {
            item->JobType = start[5];

         } else if (strncasecmp(start, "status=", 7) == 0) {
            item->JobStatus = start[7];

         } else if (strncasecmp(start, "jobbytes=", 9) == 0) {
            item->JobBytes = str_to_uint64(start + 9);

         } else if (strncasecmp(start, "jobfiles=", 9) == 0) {
            item->JobFiles = str_to_uint64(start + 9);

         } else if (strncasecmp(start, "job=", 4) == 0) {
            bstrncpy(item->Job, start + 4, sizeof(item->Job));

         } else if (strncasecmp(start, "starttime_epoch=", 16) == 0) {
            item->start_time = str_to_uint64(start + 16);

         } else if (strncasecmp(start, "endtime_epoch=", 14) == 0) {
            item->end_time = str_to_uint64(start + 14);

         } else if (strncasecmp(start, "errors=", 7) == 0) {
            item->Errors = str_to_uint64(start + 7);
         }
         start = end+1;
      }
      r->last_update = time(NULL);
      r->mutex->unlock();

      if (r->bs->is_error()) {
         Mmsg(errmsg, "Got error on the socket communication line");
         goto bail_out;
      }
   }
   if (item) {
      r->mutex->lock();
      Dmsg1(dbglvl, "Append item %ld\n", item->JobId);
      r->terminated_jobs->append(item);
      r->mutex->unlock();
   }
   ret = true;

bail_out:
   return ret;
}

bool task::read_status_header(RESMON *r)
{
   bool ret = false;
   char *start, *end;

   while (r->bs->recv() >= -1) {
      if (r->bs->msglen < 0 &&
          r->bs->msglen != BNET_CMD_BEGIN &&
          r->bs->msglen != BNET_CMD_OK)
      {
         Dmsg1(dbglvl, "Got Signal %d\n", r->bs->msglen);
         break;
      }

      Dmsg2(dbglvl, "RECV -> %s:%d\n", r->bs->msg, r->bs->msglen);
      r->mutex->lock();
      start = r->bs->msg;

      while ((end = strchr(start, '\n')) != NULL) {
         *end = 0;
         Dmsg1(dbglvl, "line=[%s]\n", start);
         if (strncasecmp(start, "name=", 5) == 0) {
            bstrncpy(r->name, start + 5, sizeof(r->name));

         } else if (strncasecmp(start, "version=", 8) == 0) {
            bstrncpy(r->version, start + 8, sizeof(r->version));

         } else if (strncasecmp(start, "plugins=", 8) == 0) {
            bstrncpy(r->plugins, start + 8, sizeof(r->plugins));

         } else if (strncasecmp(start, "bwlimit=", 8) == 0) {
            r->bwlimit = str_to_uint64(start + 8);

         } else if (strncasecmp(start, "started=", 8) == 0) {
            bstrncpy(r->started, start + 8, sizeof(r->started));

         } else if (strncasecmp(start, "reloaded=", 9) == 0) {
            bstrncpy(r->reloaded, start + 9, sizeof(r->reloaded));
         }
         start = end+1;
      }

      if (r->bs->is_error()) {
         r->mutex->unlock();
         Mmsg(errmsg, "Got error on the socket communication line");
         goto bail_out;

      } 
      r->last_update = time(NULL);
      r->mutex->unlock();
   }
   ret = true;
bail_out:
   return ret;
}


bool task::do_status()
{
   bool ret = false;
   btimer_t *tid = NULL;

   /* We don't want to use a proxy session */
   if (res->type == R_CLIENT && res->proxy_sent) {
      free_bsock(res->bs);
   }
   if (!res->bs || !res->bs->is_open() || res->bs->is_error()) {
      if (!connect_bacula()) {
         goto bail_out;
      }
   }
   /* TODO: */
   tid = start_thread_timer(NULL, pthread_self(), (uint32_t)120);
   if (res->type == R_CLIENT || res->type == R_STORAGE) {
      Dmsg0(dbglvl, "Send status command header\n");
      res->bs->fsend(".status header api=2\n");
      // TODO: Update a local set of variables and commit everything when it's done
      ret = read_status_header(res);

      if (ret) {
         res->bs->fsend(".status terminated api=2\n");
         ret = read_status_terminated(res);
      }
      if (ret) {
         res->bs->fsend(".status running api=2\n");
         ret = read_status_running(res);
      }
   }
   if (res->type == R_DIRECTOR) {
      Dmsg0(dbglvl, "-> .api 2\n");
      res->bs->fsend(".api 2\n");
      while (get_next_line(res)) {
         Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
      }
      Dmsg0(dbglvl, "Send status command header\n");
      res->bs->fsend(".status dir header\n");
      // TODO: Update a local set of variables and commit everything when it's done
      ret = read_status_header(res);

      if (ret) {
         Dmsg0(dbglvl, "Send status command terminated\n");
         res->bs->fsend(".status dir terminated\n");
         ret = read_status_terminated(res);
      }
      if (ret) {
         Dmsg0(dbglvl, "Send status command running\n");
         res->bs->fsend(".status dir running\n");
         ret = read_status_running(res);
      }
   }
bail_out:
   if (tid) {
      stop_thread_timer(tid);
   }
   /* Use a new socket the next time */
   disconnect_bacula();
   if (ret) {
      mark_as_done();
   } else {
      mark_as_failed();
   }
   return ret;
}

bool task::get_next_line(RESMON *r)
{
   /* We are currently reading a line */
   if (curline && curend && r->bs->msglen > 0 && curend < (r->bs->msg + r->bs->msglen - 1)) {
      curline = curend + 1;    /* skip \0 */
      if ((curend = strchr(curline, '\n')) != NULL) {
         *curend = '\0';
      }
      return true;
   }
   curline = curend = NULL;
   do {
      r->bs->recv();

      if (r->bs->msglen < 0) {
         Dmsg1(dbglvl, "<- %s\n", bnet_sig_to_ascii(r->bs->msglen));
         switch(r->bs->msglen) {
         case BNET_ERROR_MSG:
            r->bs->recv();
            strip_trailing_junk(r->bs->msg);
            Dmsg1(0, "ERROR: %s\n", r->bs->msg);
            break;
         case BNET_MAIN_PROMPT:       // stop
            return false;
         case BNET_CMD_OK:
         case BNET_CMD_BEGIN:
         case BNET_MSGS_PENDING:
            break;
         case BNET_TERMINATE:
            return false;
         default:                     // error or question?
            return false;
         }

      } else if (r->bs->msglen == 0) { // strange
         return false;

      } else {
         Dmsg1(10, "<- %s\n", r->bs->msg);
         curline = r->bs->msg;
         curend = strchr(curline, '\n');
         if (curend) {
            *curend = 0;
         }
         return true;           // something to read
      }
   } while (!r->bs->is_error());
   return false;
}

bool task::get_job_defaults()
{
   bool ret = false;
   btimer_t *tid = NULL;
   char *p;

   if (!res->bs || !res->bs->is_open() || res->bs->is_error()) {
      if (!connect_bacula()) { 
         goto bail_out;
      }
   }

   res->mutex->lock();
   bfree_and_null(res->defaults.client);
   bfree_and_null(res->defaults.pool);
   bfree_and_null(res->defaults.storage);
   bfree_and_null(res->defaults.level);
   bfree_and_null(res->defaults.type);
   bfree_and_null(res->defaults.fileset);
   bfree_and_null(res->defaults.catalog);

   tid = start_thread_timer(NULL, pthread_self(), (uint32_t)120);
   if (res->type == R_CLIENT && !res->proxy_sent) {
      res->proxy_sent = true;
      res->bs->fsend("proxy\n");
      while (get_next_line(res)) {
         if (strncmp(curline, "2000", 4) != 0) {
            pm_strcpy(errmsg, curline);
            goto bail_out;
         }
         Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
      }
   }

   res->bs->fsend(".api 2\n");
   while (get_next_line(res)) {
      Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
   }
   res->bs->fsend(".defaults job=\"%s\"\n", res->defaults.job);
   while (get_next_line(res)) {
      Dmsg1(dbglvl, "line = [%s]\n", curline);
      if ((p = strchr(curline, '=')) == NULL) {
         continue;
      }
      *p++ = 0;
      if (strcasecmp(curline, "client") == 0) {
         res->defaults.client = bstrdup(p);

      } else if (strcasecmp(curline, "pool") == 0) {
         res->defaults.pool = bstrdup(p);

      } else if (strcasecmp(curline, "storage") == 0) {
         res->defaults.storage = bstrdup(p);

      } else if (strcasecmp(curline, "level") == 0) {
         res->defaults.level = bstrdup(p);

      } else if (strcasecmp(curline, "type") == 0) {
         res->defaults.type = bstrdup(p);

      } else if (strcasecmp(curline, "fileset") == 0) {
         res->defaults.fileset = bstrdup(p);

      } else if (strcasecmp(curline, "catalog") == 0) {
         res->defaults.catalog = bstrdup(p);

      } else if (strcasecmp(curline, "priority") == 0) {
         res->defaults.priority = str_to_uint64(p);
      }
   }
   ret = true;
bail_out:
   if (tid) {
      stop_thread_timer(tid);
   }
   if (ret) {
      mark_as_done();
   } else {
      mark_as_failed();
   }
   res->mutex->unlock();   
   return ret;
}

bool task::get_job_info(const char *level)
{
   bool ret = false;
   btimer_t *tid = NULL;
   char *p;

   if (!res->bs || !res->bs->is_open() || res->bs->is_error()) {
      if (!connect_bacula()) { 
         goto bail_out;
      }
   }
   res->mutex->lock();
   memset(&res->infos, 0, sizeof(res->infos));

   tid = start_thread_timer(NULL, pthread_self(), (uint32_t)120);
   if (res->type == R_CLIENT && !res->proxy_sent) {
      res->proxy_sent = true;
      res->bs->fsend("proxy\n");
      while (get_next_line(res)) {
         if (strncmp(curline, "2000", 4) != 0) {
            pm_strcpy(errmsg, curline);
            goto bail_out;
         }
         Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
      }
   }

   res->bs->fsend(".api 2\n");
   while (get_next_line(res)) {
      Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
   }
   if (level) {
      res->bs->fsend(".estimate job=\"%s\" level=%s\n", arg, level);
   } else {
      res->bs->fsend(".estimate job=\"%s\"\n", arg);
   }
   while (get_next_line(res)) {
      Dmsg1(dbglvl, "line = [%s]\n", curline);
      if ((p = strchr(curline, '=')) == NULL) {
         continue;
      }
      *p++ = 0;
      if (strcasecmp(curline, "level") == 0) {
         res->infos.JobLevel = p[0];

      } else if (strcasecmp(curline, "jobbytes") == 0) {
         res->infos.JobBytes = str_to_uint64(p);

      } else if (strcasecmp(curline, "jobfiles") == 0) {
         res->infos.JobFiles = str_to_uint64(p);

      } else if (strcasecmp(curline, "corrbytes") == 0) {
         res->infos.CorrJobBytes = str_to_uint64(p);

      } else if (strcasecmp(curline, "corrfiles") == 0) {
         res->infos.CorrJobFiles = str_to_uint64(p);

      } else if (strcasecmp(curline, "nbjob") == 0) {
         res->infos.CorrNbJob = str_to_uint64(p);
      }
   }
   ret = true;
bail_out:
   res->mutex->unlock();
   if (tid) {
      stop_thread_timer(tid);
   }
   if (ret) {
      mark_as_done();
   } else {
      mark_as_failed();
   }
   return ret;
}

bool task::run_job()
{
   bool ret = false;
   char *p;
   btimer_t *tid = NULL;

   if (!res->bs || !res->bs->is_open() || res->bs->is_error()) {
      if (!connect_bacula()) { 
         goto bail_out;
      }
   }

   tid = start_thread_timer(NULL, pthread_self(), (uint32_t)120);
   if (res->type == R_CLIENT && !res->proxy_sent) {
      res->proxy_sent = true;
      res->bs->fsend("proxy\n");
      while (get_next_line(res)) {
         if (strncmp(curline, "2000", 4) != 0) {
            pm_strcpy(errmsg, curline);
            goto bail_out;
         }
         Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
      }
   }

   res->bs->fsend(".api 2\n");
   while (get_next_line(res)) {
      Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
   }
   if (res->type == R_DIRECTOR && res->use_setip) {
      res->bs->fsend("setip\n");
      while (get_next_line(res)) {
         Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
      }
   }
   res->bs->fsend("%s\n", arg);
   while (get_next_line(res)) {
      if ((p = strstr(curline, "JobId=")) != NULL && sscanf(p, "JobId=%d\n", &result.i) == 1) {
         ret = true;
      }
   }
   // Close the socket, it's over or we don't want to reuse it
   disconnect_bacula();
   
bail_out:

   if (tid) {
      stop_thread_timer(tid);
   }
   if (ret) {
      mark_as_done();
   } else {
      mark_as_failed();
   }
   return ret;
}

/* Get resources to run a job */
bool task::get_resources()
{
   bool ret = false;
   btimer_t *tid = NULL;

   if (!res->bs || !res->bs->is_open() || res->bs->is_error()) {
      if (!connect_bacula()) { 
         goto bail_out;
      }
   }

   res->mutex->lock();
   if (res->jobs) {
      delete res->jobs;
   }
   res->jobs = New(alist(10, owned_by_alist));
   if (res->clients) {
      delete res->clients;
   }
   res->clients = New(alist(10, owned_by_alist));
   if (res->filesets) {
      delete res->filesets;
   }
   res->filesets = New(alist(10, owned_by_alist));
   if (res->pools) {
      delete res->pools;
   }
   res->pools = New(alist(10, owned_by_alist));
   if (res->storages) {
      delete res->storages;
   }
   res->storages = New(alist(10, owned_by_alist));
   if (res->catalogs) {
      delete res->catalogs;
   }
   res->catalogs = New(alist(10, owned_by_alist));

   tid = start_thread_timer(NULL, pthread_self(), (uint32_t)120);
   if (res->type == R_CLIENT && !res->proxy_sent) {
      res->proxy_sent = true;
      res->bs->fsend("proxy\n");
      while (get_next_line(res)) {
         if (strncmp(curline, "2000", 4) != 0) {
            pm_strcpy(errmsg, curline);
            goto bail_out;
         }
         Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
      }
      Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
   }

   res->bs->fsend(".api 2\n");
   while (get_next_line(res)) {
      Dmsg2(dbglvl, "<- %d %s\n", res->bs->msglen, curline);
   }

   res->bs->fsend(".jobs type=B\n");
   while (get_next_line(res)) {
      res->jobs->append(bstrdup(curline));
   }

   res->bs->fsend(".pools\n");
   while (get_next_line(res)) {
      res->pools->append(bstrdup(curline));
   }

   res->bs->fsend(".clients\n");
   while (get_next_line(res)) {
      res->clients->append(bstrdup(curline));
   }

   res->bs->fsend(".filesets\n");
   while (get_next_line(res)) {
      res->filesets->append(bstrdup(curline));
   }

   res->bs->fsend(".storage\n");
   while (get_next_line(res)) {
      res->storages->append(bstrdup(curline));
   }

   res->bs->fsend(".catalog\n");
   while (get_next_line(res)) {
      res->catalogs->append(bstrdup(curline));
   }

bail_out:
   res->mutex->unlock();

   if (tid) {
      stop_thread_timer(tid);
   }
   if (ret) {
      mark_as_done();
   } else {
      mark_as_failed();
   }
   return ret;
}

worker *worker_start()
{
   worker *w = New(worker());
   w->start(handle_task, w);
   return w;
}

void worker_stop(worker *w)
{
   if (w) {
      w->stop();
      delete w;
   }
}
