/*
 *   bpipe.c bi-directional pipe
 * 
 *    Kern Sibbald, November MMII
 *
 *   Version $Id$
 */

/*
   Copyright (C) 2000, 2001, 2002 Kern Sibbald and John Walker

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

 */

#include "bacula.h"
#include "jcr.h"


#define MAX_ARGV 100
static void build_argc_argv(char *cmd, int *bargc, char *bargv[], int max_arg);



/*
 * Run an external program. Optionally wait a specified number
 *   of seconds. Program killed if wait exceeded. We open
 *   a bi-directional pipe so that the user can read from and
 *   write to the program. 
 */
BPIPE *open_bpipe(char *prog, int wait, char *mode)
{
   char *bargv[MAX_ARGV];
   int bargc;
   int readp[2], writep[2];
   POOLMEM *tprog;
   int mode_read, mode_write;
   BPIPE *bpipe;

   bpipe = (BPIPE *)malloc(sizeof(BPIPE));
   memset(bpipe, 0, sizeof(BPIPE));
   mode_read = (mode[0] == 'r');
   mode_write = (mode[0] == 'w' || mode[1] == 'w');
   /* Build arguments for running program. */
   tprog = get_pool_memory(PM_FNAME);
   pm_strcpy(&tprog, prog);
   build_argc_argv(tprog, &bargc, bargv, MAX_ARGV);
#ifdef	xxxxxx
   printf("argc=%d\n", bargc);
   int i;
   for (i=0; i<bargc; i++) {
      printf("argc=%d argv=%s:\n", i, bargv[i]);
   }
#endif
   free_pool_memory(tprog);

   /* Each pipe is one way, write one end, read the other, so we need two */
   if (mode_write && pipe(writep) == -1) {
      free(bpipe);
      return NULL;
   }
   if (mode_read && pipe(readp) == -1) {
      free(bpipe);
      return NULL;
   }
   /* Start worker process */
   switch (bpipe->worker_pid = fork()) {
   case -1:			      /* error */
      free(bpipe);
      return NULL;

   case 0:			      /* child */
      if (mode_write) {
	 close(writep[1]);
	 dup2(writep[0], 0);	      /* Dup our write to his stdin */
      }
      if (mode_read) {
	 close(readp[0]);	      /* Close unused child fds */
	 dup2(readp[1], 1);	      /* dup our read to his stdout */
	 dup2(readp[1], 2);	      /*   and his stderr */
      }
      execvp(bargv[0], bargv);	      /* call the program */
      exit(errno);                    /* shouldn't get here */

   default:			      /* parent */
      break;
   }
   if (mode_read) {
      close(readp[1]);		      /* close unused parent fds */
      bpipe->rfd = fdopen(readp[0], "r"); /* open file descriptor */
   }
   if (mode_write) {
      close(writep[0]);
      bpipe->wfd = fdopen(writep[1], "w");
   }
   bpipe->worker_stime = time(NULL);
   bpipe->wait = wait;
   if (wait > 0) {
      bpipe->timer_id = start_child_timer(bpipe->worker_pid, wait);
   }
   return bpipe;
}

/* Close the write pipe only */
int close_wpipe(BPIPE *bpipe)
{
   int stat = 1;

   if (bpipe->wfd) {
      fflush(bpipe->wfd);
      if (fclose(bpipe->wfd) != 0) {
	 stat = 0;
      }
      bpipe->wfd = NULL;
   }
   return stat;
}

/* Close both pipes and free resources */
int close_bpipe(BPIPE *bpipe) 
{
   int chldstatus = 0;
   int stat = 0;    
   int wait_option;
   int remaining_wait;
   pid_t wpid = 0;


   /* Close pipes */
   if (bpipe->rfd) {
      fclose(bpipe->rfd);
      bpipe->rfd = NULL;
   }
   if (bpipe->wfd) {
      fclose(bpipe->wfd);
      bpipe->wfd = NULL;
   }

   if (bpipe->wait == 0) {
      wait_option = 0;		      /* wait indefinitely */
   } else {
      wait_option = WNOHANG;          /* don't hang */
   }
   remaining_wait = bpipe->wait;

   /* wait for worker child to exit */
   for ( ;; ) {
      wpid = waitpid(bpipe->worker_pid, &chldstatus, wait_option);
      if (wpid == bpipe->worker_pid || (wpid == -1 && errno != EINTR)) {
	 break;
      }
      if (remaining_wait > 0) {
	 bmicrosleep(1, 0);	       /* wait one second */
	 remaining_wait--;
      } else {
	 stat = 1;		      /* set error status */
	 errno = ETIME; 	      /* set timed out */
	 wpid = -1;
         break;                       /* don't wait any longer */
      }
   }
   if (wpid > 0) {
      if (WIFEXITED(chldstatus)) {	     /* process exit()ed */
	 stat = WEXITSTATUS(chldstatus);
      } else if (WIFSIGNALED(chldstatus)) {  /* process died */
	 stat = 1;
      }
      if (stat != 0) {
	 errno = ECHILD;	      /* set child errno */
      }
   }  
   if (bpipe->timer_id) {
      stop_child_timer(bpipe->timer_id);
   }
   free(bpipe);
#ifdef HAVE_FREEBSD_OS
   stat = 0;  /* kludge because FreeBSD doesn't seem to return valid status */
#endif
   return stat;
}


/*
 * Run an external program. Optionally wait a specified number
 *   of seconds. Program killed if wait exceeded. Optionally
 *   return the output from the program (normally a single line).
 *
 * Contrary to my normal calling conventions, this program 
 *
 *  Returns: 0 on success
 *	     non-zero on error
 */
int run_program(char *prog, int wait, POOLMEM *results)
{
   BPIPE *bpipe;
   int stat1, stat2;
   char *mode;

   mode = (char *)(results != NULL ? "r" : "");
   bpipe = open_bpipe(prog, wait, mode);
   if (!bpipe) {
      return 0;
   }
   if (results) {
      results[0] = 0;
      stat1 = fgets(results, sizeof_pool_memory(results), bpipe->rfd) == NULL;
   } else {
      stat1 = 0;
   }
   stat2 = close_bpipe(bpipe);
   return stat2 != 0 ? stat2 : stat1; 
}


/*
 * Build argc and argv from a string
 */
static void build_argc_argv(char *cmd, int *bargc, char *bargv[], int max_argv)
{
   int i;	
   char *p, *q, quote;
   int argc = 0;

   argc = 0;
   for (i=0; i<max_argv; i++)
      bargv[i] = NULL;

   p = cmd;
   quote = 0;
   while  (*p && (*p == ' ' || *p == '\t'))
      p++;
   if (*p == '\"' || *p == '\'') {
      quote = *p;
      p++;
   }
   if (*p) {
      while (*p && argc < MAX_ARGV) {
	 q = p;
	 if (quote) {
	    while (*q && *q != quote)
	    q++;
	    quote = 0;
	 } else {
            while (*q && *q != ' ')
	    q++;
	 }
	 if (*q)
            *(q++) = '\0';
	 bargv[argc++] = p;
	 p = q;
         while (*p && (*p == ' ' || *p == '\t'))
	    p++;
         if (*p == '\"' || *p == '\'') {
	    quote = *p;
	    p++;
	 }
      }
   }
   *bargc = argc;
}
