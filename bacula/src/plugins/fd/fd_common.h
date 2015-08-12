/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2015 Kern Sibbald
   Copyright (C) 2010-2014 Free Software Foundation Europe e.V.

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

/* You can include this file to your plugin to have
 * access to some common tools and utilities provided by Bacula
 */

#ifndef PCOMMON_H
#define PCOMMON_H

#define JT_BACKUP                'B'  /* Backup Job */
#define JT_RESTORE               'R'  /* Restore Job */

#define L_FULL                   'F'  /* Full backup */
#define L_INCREMENTAL            'I'  /* since last backup */
#define L_DIFFERENTIAL           'D'  /* since last full backup */

#ifndef DLL_IMP_EXP
# if defined(BUILDING_DLL)
#   define DLL_IMP_EXP   __declspec(dllexport)
# elif defined(USING_DLL)
#   define DLL_IMP_EXP   __declspec(dllimport)
# else
#   define DLL_IMP_EXP
# endif
#endif

DLL_IMP_EXP void *sm_malloc(const char *fname, int lineno, unsigned int nbytes);
DLL_IMP_EXP void sm_free(const char *file, int line, void *fp);
DLL_IMP_EXP void *reallymalloc(const char *fname, int lineno, unsigned int nbytes);
DLL_IMP_EXP void reallyfree(const char *file, int line, void *fp);

#ifndef bmalloc
# define bmalloc(s)      sm_malloc(__FILE__, __LINE__, (s))
# define bfree(o)        sm_free(__FILE__, __LINE__, (o))
#endif

#define SM_CHECK sm_check(__FILE__, __LINE__, false)

#ifdef malloc
#undef malloc
#undef free
#endif

#define malloc(s)    sm_malloc(__FILE__, __LINE__, (s))
#define free(o)      sm_free(__FILE__, __LINE__, (o))

#define Dmsg(context, level,  ...) bfuncs->DebugMessage(context, __FILE__, __LINE__, level, __VA_ARGS__ )
#define Jmsg(context, type,  ...) bfuncs->JobMessage(context, __FILE__, __LINE__, type, 0, __VA_ARGS__ )


#ifdef USE_CMD_PARSER
/* from lib/scan.c */
extern int parse_args(POOLMEM *cmd, POOLMEM **args, int *argc,
                      char **argk, char **argv, int max_args);
extern int parse_args_only(POOLMEM *cmd, POOLMEM **args, int *argc,
                           char **argk, char **argv, int max_args);

class cmd_parser {
public:
   POOLMEM *args;
   POOLMEM *cmd;                /* plugin command line */
   POOLMEM *org;                /* original command line */

   char **argk;                  /* Argument keywords */
   char **argv;                  /* Argument values */
   int argc;                    /* Number of arguments */
   int max_cmd;                 /* Max number of arguments */
   bool  use_name;              /* Search for : */

   cmd_parser() {
      org = get_pool_memory(PM_FNAME);
      args = get_pool_memory(PM_FNAME);
      cmd = get_pool_memory(PM_FNAME);
      *args = *org  = *cmd = 0;
      argc = 0; 
      use_name = true;
      max_cmd = MAX_CMD_ARGS;
      argk = argv = NULL;
   };

   virtual ~cmd_parser() {
      free_pool_memory(org);
      free_pool_memory(cmd);
      free_pool_memory(args);
      if (argk) {
         free(argk);
      }
      if (argv) {
         free(argv);
      }
   }

   /*
    * Given a single keyword, find it in the argument list, but
    *   it must have a value
    * Returns: -1 if not found or no value
    *           list index (base 0) on success
    */
   int find_arg_with_value(const char *keyword)
   {
      for (int i=1; i<argc; i++) {
         if (strcasecmp(keyword, argk[i]) == 0) {
            if (argv[i]) {
               return i;
            } else {
               return -1;
            }
         }
      }
      return -1;
   }

   /*
    * Given a single keyword, find it in the argument list
    * Returns: -1 if not found
    *           list index (base 0) on success
    */
   int find_arg(const char *keyword)
   {
      for (int i=1; i<argc; i++) {
         if (strcasecmp(keyword, argk[i]) == 0) {
            return i;
         }
      }
      return -1;
   }

   /* 
    * Build args, argc, argk from Plugin Restore|Backup command
    */

   bRC parse_cmd(const char *line)
   {
      char *a;
      int nbequal = 0;
      if (!line || *line == '\0') {
         return bRC_Error;
      }

      /* Same command line that before */
      if (!strcmp(line, org)) {
         return bRC_OK;
      }
      
      /* 
       * line = delta:minsize=10002 param1=xxx
       *             |     backup command
       */
      pm_strcpy(org, line);
      pm_strcpy(cmd, line);

      if (use_name) {
         if ((a = strchr(cmd, ':')) != NULL) {
            *a = ' ';              /* replace : by ' ' for command line processing */
         } else {
            return bRC_Error;
         }
      }

      for (char *p = cmd; *p ; p++) {
         if (*p == '=') {
            nbequal++;
         }
      }

      if (argk) {
         free(argk);
      }
      if (argv) {
         free(argv);
      }

      max_cmd = MAX(nbequal, MAX_CMD_ARGS) + 1;

      argk = (char **) malloc(sizeof(char **) * max_cmd);
      argv = (char **) malloc(sizeof(char **) * max_cmd);
      
      parse_args(cmd, &args, &argc, argk, argv, max_cmd);
      
      return bRC_OK;
   }

};
#endif  /* USE_CMD_PARSER */

#ifdef USE_ADD_DRIVE
/* Keep drive letters for windows vss snapshot */
static void add_drive(char *drives, int *nCount, char *fname) {
   if (strlen(fname) >= 2 && B_ISALPHA(fname[0]) && fname[1] == ':') {
      /* always add in uppercase */
      char ch = toupper(fname[0]);
      /* if not found in string, add drive letter */
      if (!strchr(drives,ch)) {
         drives[*nCount] = ch;
         drives[*nCount+1] = 0;
         (*nCount)++;
      }                                
   }
}

/* Copy our drive list to Bacula core list */
static void copy_drives(char *drives, char *dest) {
   int last = strlen(dest);     /* dest is 27 bytes long */
   for (char *p = drives; *p && last < 26; p++) {
      if (!strchr(dest, *p)) {
         dest[last++] = *p;
         dest[last] = 0;
      }
   }
}
#endif  /* USE_ADD_DRIVE */

#endif  /* ! PCOMMON_H */
