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

#include "bacula.h"
#include "bwlimit.h"

#define ONE_SEC 1000000L /* number of microseconds in a second */

void bwlimit::control_bwlimit(int bytes)
{
   btime_t now, temp;
   if (bytes == 0 || m_bwlimit == 0) {
      return;
   }

   lock_guard lg(m_bw_mutex);     /* Release the mutex automatically when we quit the function*/
   now = get_current_btime();          /* microseconds */
   temp = now - m_last_tick;           /* microseconds */

   m_nb_bytes += bytes;
   if (temp < 0 || temp > (ONE_SEC*10)) { /* Take care of clock problems (>10s) or back in time */
      m_nb_bytes = bytes;
      m_last_tick = now;
      return;
   }

   /* Less than 0.1ms since the last call, see the next time */
   if (temp < 100) {
      return;
   }

   /* Remove what was authorised to be written in temp us */
   m_nb_bytes -= (int64_t)(temp * ((double)m_bwlimit / ONE_SEC));

   if (m_nb_bytes < 0) {
      m_nb_bytes = 0;
   }

   /* What exceed should be converted in sleep time */
   int64_t usec_sleep = (int64_t)(m_nb_bytes /((double)m_bwlimit / ONE_SEC));
   if (usec_sleep > 100) {
      bmicrosleep(usec_sleep / ONE_SEC, usec_sleep % ONE_SEC);
      m_last_tick = get_current_btime();
      m_nb_bytes = 0;
   } else {
      m_last_tick = now;
   }
}
