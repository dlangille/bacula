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

#ifndef TASK_H
#define TASK_H

#include "common.h"
#include <QtCore/QObject>
#include "tray_conf.h"

enum {
   TASK_NONE,
   TASK_STATUS,
   TASK_RESOURCES,
   TASK_QUERY,
   TASK_RUN,
   TASK_RESTORE,
   TASK_DEFAULTS,
   TASK_CLOSE,
   TASK_INFO,
   TASK_BWLIMIT,
   TASK_DISCONNECT
};

/* The task should emit a signal when done */
class task: public QObject
{
   Q_OBJECT

public:
   RESMON   *res;
   POOLMEM  *errmsg;
   int       type;
   bool      status;
   char     *curline;
   char     *curend;
   char     *arg;               /* Argument that can be used by some tasks */
   char     *arg2;

   union {
      bool  b;
      int   i;
      char  c[256];
   } result;                    /* The task might return something */
   
   task(): QObject(), res(NULL), type(TASK_NONE), status(false), curline(NULL),
      curend(NULL), arg(NULL), arg2(NULL)
   {
      errmsg = get_pool_memory(PM_FNAME);
      *errmsg = 0;
      memset(result.c, 0, sizeof(result.c));
   };
   ~task() {
      Enter();
      disconnect();             /* disconnect all signals */
      free_pool_memory(errmsg);
   };
   void init(int t) {
      res = NULL;
      type = t;
      status = false;
   };
   void init(RESMON *s, int t) {
      init(t);
      res = s;
   };

   RESMON *get_res();
   void lock_res();
   void unlock_res();
   bool connect_bacula();
   bool do_status();
   bool read_status_terminated(RESMON *res);
   bool read_status_header(RESMON *res);
   bool read_status_running(RESMON *res);
   bool set_bandwidth();
   bool disconnect_bacula();
   void mark_as_done() {
      status = true;
      emit done(this);
   };
   void mark_as_failed() {
      status = false;
      emit done(this);
   };
   bool get_resources();
   bool get_next_line(RESMON *res);
   bool get_job_defaults(); /* Look r->defaults.job */
   bool run_job();
   bool get_job_info(const char *level);     /* look r->info */

signals:
   void done(task *t);
};

worker *worker_start();
void worker_stop(worker *);

#endif
