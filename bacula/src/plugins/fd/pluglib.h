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
 * Common definitions and utility functions for Inteos plugins.
 * Functions defines a common framework used in our utilities and plugins.
 *
 * Author: Radosław Korzeniewski, MMXIX
 * radoslaw@korzeniewski.net, radekk@inteos.pl
 * Inteos Sp. z o.o. http://www.inteos.pl/
 */

#ifndef _PLUGLIB_H_
#define _PLUGLIB_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#include "bacula.h"
#include "fd_plugins.h"

/* Pointers to Bacula functions used in plugins */
extern bFuncs *bfuncs;
extern bInfo *binfo;

/* definitions */
/* size of different string or query buffers */
#define BUFLEN       4096
#define BIGBUFLEN    65536

/* debug and messages functions */
#define JMSG0(ctx,type,msg) \
      if (ctx) bfuncs->JobMessage ( ctx, __FILE__, __LINE__, type, 0, PLUGINPREFIX " " msg );

#define JMSG1 JMSG
#define JMSG(ctx,type,msg,var) \
      if (ctx) bfuncs->JobMessage ( ctx, __FILE__, __LINE__, type, 0, PLUGINPREFIX " " msg, var );

#define JMSG2(ctx,type,msg,var1,var2) \
      if (ctx) bfuncs->JobMessage ( ctx, __FILE__, __LINE__, type, 0, PLUGINPREFIX " " msg, var1, var2 );

#define JMSG3(ctx,type,msg,var1,var2,var3) \
      if (ctx) bfuncs->JobMessage ( ctx, __FILE__, __LINE__, type, 0, PLUGINPREFIX " " msg, var1, var2, var3 );

#define JMSG4(ctx,type,msg,var1,var2,var3,var4) \
      if (ctx) bfuncs->JobMessage ( ctx, __FILE__, __LINE__, type, 0, PLUGINPREFIX " " msg, var1, var2, var3, var4 );

#define DMSG0(ctx,level,msg) \
      if (ctx) bfuncs->DebugMessage ( ctx, __FILE__, __LINE__, level, PLUGINPREFIX " " msg );

#define DMSG1 DMSG
#define DMSG(ctx,level,msg,var) \
      if (ctx) bfuncs->DebugMessage ( ctx, __FILE__, __LINE__, level, PLUGINPREFIX " " msg, var );

#define DMSG2(ctx,level,msg,var1,var2) \
      if (ctx) bfuncs->DebugMessage ( ctx, __FILE__, __LINE__, level, PLUGINPREFIX " " msg, var1, var2 );

#define DMSG3(ctx,level,msg,var1,var2,var3) \
      if (ctx) bfuncs->DebugMessage ( ctx, __FILE__, __LINE__, level, PLUGINPREFIX " " msg, var1, var2, var3 );

#define DMSG4(ctx,level,msg,var1,var2,var3,var4) \
      if (ctx) bfuncs->DebugMessage ( ctx, __FILE__, __LINE__, level, PLUGINPREFIX " " msg, var1, var2, var3, var4 );

#define DMSG6(ctx,level,msg,var1,var2,var3,var4,var5,var6) \
      if (ctx) bfuncs->DebugMessage ( ctx, __FILE__, __LINE__, level, PLUGINPREFIX " " msg, var1, var2, var3, var4, var5, var6 );

/* fixed debug level definitions */
#define D1  1                    /* debug for every error */
#define DERROR D1
#define D2  10                   /* debug only important stuff */
#define DINFO  D2
#define D3  200                  /* debug for information only */
#define DDEBUG D3
#define D4  800                  /* debug for detailed information only */
#define DVDEBUG D4

#define getBaculaVar(bvar,val)  bfuncs->getBaculaValue(ctx, bvar, val);

/* used for sanity check in plugin functions */
#define ASSERT_CTX \
    if (!ctx || !ctx->pContext || !bfuncs) \
    { \
        return bRC_Error; \
    }

/* defines for handleEvent */
#define DMSG_EVENT_STR(event,value)       DMSG2(ctx, DINFO, "%s value=%s\n", eventtype2str(event), NPRT((char *)value));
#define DMSG_EVENT_CHAR(event,value)      DMSG2(ctx, DINFO, "%s value='%c'\n", eventtype2str(event), (char)value);
#define DMSG_EVENT_LONG(event,value)      DMSG2(ctx, DINFO, "%s value=%ld\n", eventtype2str(event), (intptr_t)value);
#define DMSG_EVENT_PTR(event,value)       DMSG2(ctx, DINFO, "%s value=%p\n", eventtype2str(event), value);

const char *eventtype2str(bEvent *event);
uint64_t pluglib_size_suffix(int disksize, char suff);
uint64_t pluglib_size_suffix(double disksize, char suff);
bRC pluglib_mkpath(bpContext* ctx, char* path, bool isfatal);

/*
 * Checks if plugin command points to our Plugin
 *
 * in:
 *    command - the plugin command used for backup/restore
 * out:
 *    True - if it is our plugin command
 *    False - the other plugin command
 */
inline bool isourplugincommand(const char *pluginprefix, const char *command)
{
   /* check if it is our Plugin command */
   if (strncmp(pluginprefix, command, strlen(pluginprefix)) == 0){
      /* it is not our plugin prefix */
      return true;
   }
   return false;
}

#endif                           /* _PLUGLIB_H_ */
