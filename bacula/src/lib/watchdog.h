/*
 * Watchdog timer routines
 * 
 *    Kern Sibbald, December MMII
 *
*/
/*
   Copyright (C) 2002 Kern Sibbald and John Walker

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

#define TYPE_CHILD   1
#define TYPE_PTHREAD 2

#define TIMEOUT_SIGNAL SIGUSR2

typedef struct s_btimer_t {
   struct s_btimer_t *next;
   struct s_btimer_t *prev;
   time_t start_time;
   int32_t  wait;
   pid_t pid;			      /* process id if TYPE_CHILD */
   int killed;
   int type;
   pthread_t tid;		      /* thread id if TYPE_PTHREAD */
} btimer_t;

#define btimer_id btimer_t *
