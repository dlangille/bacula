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

#ifdef SMARTALLOC
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

/* Looks to be broken on scientific linux */
#ifdef xxxx
inline void *operator new(size_t size, char const * file, int line)
{
   void *pnew = sm_malloc(file,line, size);
   memset((char *)pnew, 0, size);
   return pnew;
}

inline void *operator new[](size_t size, char const * file, int line)
{
   void *pnew = sm_malloc(file, line, size);
   memset((char *)pnew, 0, size);
   return pnew;
}

inline void *operator new(size_t size)
{
   void *pnew = sm_malloc(__FILE__, __LINE__, size);
   memset((char *)pnew, 0, size);
   return pnew;
}

inline void *operator new[](size_t size)
{
   void *pnew = sm_malloc(__FILE__, __LINE__, size);
   memset((char *)pnew, 0, size);
   return pnew;
}

#define new   new(__FILE__, __LINE__)

inline void operator delete(void *buf)
{
   sm_free( __FILE__, __LINE__, buf);
}

inline void operator delete[] (void *buf)
{
  sm_free(__FILE__, __LINE__, buf);
}

inline void operator delete[] (void *buf, char const * file, int line)
{
  sm_free(file, line, buf);
}

inline void operator delete(void *buf, char const * file, int line)
{
   sm_free(file, line, buf);
}

#endif
#endif  /* !SMARTALLOC */

#define Dmsg(context, level,  ...) bfuncs->DebugMessage(context, __FILE__, __LINE__, level, __VA_ARGS__ )
#define Jmsg(context, type,  ...) bfuncs->JobMessage(context, __FILE__, __LINE__, type, 0, __VA_ARGS__ )


#ifdef USE_CMD_PARSER
#include "lib/cmd_parser.h"
#endif /* USE_CMD_PARSER */

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

/* Check if the bacula version is enterprise */
#ifndef check_beef
# define check_beef(ctx, ret) \
  do {                       \
     const char *v;          \
     bfuncs->getBaculaValue(ctx, bVarVersion, (void *)&v);  \
     if (v[0] == '6' || v[0] == '8') {                      \
        *(ret) = true;       \
     } else {                \
        *(ret) = false;      \
     }                       \
  } while (0)

#endif  /* check_beef */

#ifdef USE_JOB_LIST

/* This class is used to store locally the job history, you can attach data
 * to it such as snapshot names
 * !!! Don't forget that this file may be deleted by the user. !!!
 */

class joblist: public SMARTALLOC
{
private:
   bpContext *ctx;

public:
   char level;                  /* level of the job */

   char base[MAX_NAME_LENGTH];  /* base name */
   char key[MAX_NAME_LENGTH];   /* group of backup */
   char name[MAX_NAME_LENGTH];  /* job name */
   char prev[MAX_NAME_LENGTH];  /* based on jobname */
   char root[MAX_NAME_LENGTH];  /* root of this branch */
   char rootdiff[MAX_NAME_LENGTH];  /* root of diff if any */

   btime_t job_time;           /* job time */

   void init() {
      level = 0;
      job_time = 0;
      *key = *name = *prev = *root = *rootdiff = 0;
      set_base("jobs.dat");
      ctx = NULL;
   }

   void set_base(const char *b) {
      strncpy(base, b, sizeof(base));
   }

   joblist(bpContext *actx) { init(); ctx = actx; }

   joblist(bpContext *actx, 
        const char *akey, 
        const char *jobname, 
        const char *prevjobname, 
        char joblevel) 
   {
      init();
      ctx = actx;
      if (jobname) {
         strncpy(name, jobname, MAX_NAME_LENGTH);
      }

      if (prevjobname) {
         strncpy(prev, prevjobname, MAX_NAME_LENGTH);
      }

      level = joblevel;

      if (akey) {
         strncpy(key, akey, MAX_NAME_LENGTH);

      } else {
         get_key_from_name();
      }
   }

   ~joblist() { }

   /* Will extract the name from the full job name */
   bool get_key_from_name() {
      // pluginTest.2012-07-19_16.59.21_11
      int l = strlen(name);
      int dlen = 23; // strlen(".2012-07-19_16.59.21_11");

      if (l > dlen) {           /* we probably have a key */
         int start = l - dlen;
         if (name[start] == '.' &&
             B_ISDIGIT(name[start + 1]) &&   // 2
             B_ISDIGIT(name[start + 2]) &&   // 0
             B_ISDIGIT(name[start + 3]) &&   // 1
             B_ISDIGIT(name[start + 4]) &&   // 2
             name[start + 5] == '-' &&       // -
             B_ISDIGIT(name[start + 6]) &&   // 0
             B_ISDIGIT(name[start + 7]))     // 7
         {
            bstrncpy(key, name, start + 1);
            Dmsg(ctx, dbglvl+100, "key is %s from jobname %s\n", key, name);
            return true;
         }
      }
      Dmsg(ctx, dbglvl+100, "Unable to get key from jobname %s\n", name);
      return false;
   }

   bool find_job(const char *name, POOLMEM **data=NULL);   /* set root, job_time */
   bool find_root_job();
   void store_job(char *data);
   void prune_jobs(char *build_cmd(void *arg, const char *data, const char *job), 
                   void *arg, alist *jobs);
};

static pthread_mutex_t joblist_mutex = PTHREAD_MUTEX_INITIALIZER;

bool joblist::find_job(const char *name, POOLMEM **data)
{
   BFILE fp;
   FILE *f;
   POOLMEM *tmp;
   char buf[1024];
   char curkey[MAX_NAME_LENGTH];  /* key */
   char curjobname[MAX_NAME_LENGTH]; /* jobname */
   char prevjob[MAX_NAME_LENGTH]; /* last jobname */
   char rootjob[MAX_NAME_LENGTH]; /* root jobname */
   char t[MAX_NAME_LENGTH];
   char curlevel;
   bool ok=false;

   *root = 0;
   job_time = 0;
   *rootdiff = 0;

   binit(&fp);
   set_portable_backup(&fp);

   tmp = get_pool_memory(PM_FNAME);
   Mmsg(tmp, "%s/%s", working, base);

   P(joblist_mutex);
   if (bopen(&fp, tmp, O_RDONLY, 0) < 0) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to open job database. "
           "Can't open %s for reading ERR=%s\n",
           tmp, be.bstrerror(errno));
      goto bail_out;
   }

   f = fdopen(fp.fid, "r");
   if (!f) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to open job database. ERR=%s\n",
           be.bstrerror(errno));
      goto bail_out;
   }

   while (!ok && fgets(buf, sizeof(buf), f) != NULL) {
      *curkey = *curjobname = *rootjob = *prevjob = 0;

      Dmsg(ctx, dbglvl+100, "line = [%s]\n", buf);

      if (sscanf(buf, "time=%60s level=%c key=%127s name=%127s root=%127s prev=%127s", 
                 t, &curlevel, curkey, curjobname, rootjob, prevjob) != 6) {

         if (sscanf(buf, "time=%60s level=F key=%127s name=%127s", 
                        t, curkey, curjobname) != 3) {
            Dmsg(ctx, dbglvl+100, "Bad line l=[%s]\n", buf);
            continue;
         }
      }
      
      if (strcmp(name, curjobname) == 0 &&
          strcmp(key, curkey) == 0)
      {
         job_time = str_to_uint64(t);
         bstrncpy(root, rootjob, MAX_NAME_LENGTH);
         if (curlevel == 'D') {
            bstrncpy(rootdiff, curjobname, MAX_NAME_LENGTH);
         }

         if (data) {
            pm_strcpy(data, strstr(buf, " vol=") +  5);
            strip_trailing_newline(*data);
            unbash_spaces(*data);
         }

         ok = true;
         Dmsg(ctx, dbglvl+100, "Found job root %s -> %s -> %s\n",
              rootdiff, root, curjobname);
      } 
   }

   fclose(f);

bail_out:
   V(joblist_mutex);
   free_pool_memory(tmp);
   return ok;

}

/* Find the root job for the current job */
bool joblist::find_root_job()
{
   BFILE fp;
   FILE *f;
   POOLMEM *tmp;
   char buf[1024];
   char curkey[MAX_NAME_LENGTH];  /* key */
   char curjobname[MAX_NAME_LENGTH]; /* jobname */
   char prevjob[MAX_NAME_LENGTH]; /* last jobname */
   char rootjob[MAX_NAME_LENGTH]; /* root jobname */
   char t[MAX_NAME_LENGTH];
   char curlevel;
   bool ok=false;

   *root = 0;
   job_time = 0;

   if (level == 'F') {
      bstrncpy(root, name, MAX_NAME_LENGTH);
      return true;
   }

   binit(&fp);
   set_portable_backup(&fp);

   tmp = get_pool_memory(PM_FNAME);
   Mmsg(tmp, "%s/%s", working, base);

   P(joblist_mutex);
   if (bopen(&fp, tmp, O_RDONLY, 0) < 0) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to prune previous jobs. "
           "Can't open %s for reading ERR=%s\n",
           tmp, be.bstrerror(errno));
      goto bail_out;
   }

   f = fdopen(fp.fid, "r");
   if (!f) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to prune previous jobs. ERR=%s\n",
           be.bstrerror(errno));
      goto bail_out;
   }

   while (!ok && fgets(buf, sizeof(buf), f) != NULL) {
      *curkey = *curjobname = *rootjob = *prevjob = 0;

      Dmsg(ctx, dbglvl+100, "line = [%s]\n", buf);

      if (sscanf(buf, "time=%60s level=%c key=%127s name=%127s root=%127s prev=%127s", 
                 t, &curlevel, curkey, curjobname, rootjob, prevjob) != 6) {

         if (sscanf(buf, "time=%60s level=F key=%127s name=%127s", 
                        t, curkey, curjobname) == 3) {
            bstrncpy(rootjob, curjobname, MAX_NAME_LENGTH);
            *prevjob = 0;
            curlevel = 'F';

         } else {
            Dmsg(ctx, dbglvl+100, "Bad line l=[%s]\n", buf);
            continue;
         }
      }
      
      if (strcmp(key,  curkey)  == 0  &&
          strcmp(prev, curjobname) == 0) 
      {
         bstrncpy(root, rootjob, MAX_NAME_LENGTH);

         if (curlevel == 'D') {
            bstrncpy(rootdiff, curjobname, MAX_NAME_LENGTH);
         }
         ok = true;
         Dmsg(ctx, dbglvl+100, "Found job root %s -> %s -> %s\n",
              rootdiff, root, curjobname);
      } 
   }

   fclose(f);

bail_out:
   V(joblist_mutex);
   free_pool_memory(tmp);
   return true;
}

/* Store the current job in the jobs.dat for a specific data list */
void joblist::store_job(char *data)
{
   BFILE fp;
   int l;
   POOLMEM *tmp = NULL;
   btime_t now;

   /* Not initialized, no need to store jobs */
   if (*name == 0 || !level) {
      Dmsg(ctx, dbglvl+100, "store_job fail name=%s level=%d\n", name, level);
      return;
   }

   find_root_job();

   binit(&fp);
   set_portable_backup(&fp);

   P(joblist_mutex);

   tmp = get_pool_memory(PM_FNAME);
   Mmsg(tmp, "%s/%s", working, base);
   if (bopen(&fp, tmp, O_WRONLY|O_CREAT|O_APPEND, 0600) < 0) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to update the job history. ERR=%s\n",
           be.bstrerror(errno));
      goto bail_out;
   }

   now = time(NULL);
   
   bash_spaces(data);

   if (level == 'F') {
      l = Mmsg(tmp, "time=%lld level=%c key=%s name=%s vollen=%d vol=%s\n", 
               now, level, key, name, strlen(data), data);

   } else {
      l = Mmsg(tmp, "time=%lld level=%c key=%s name=%s root=%s prev=%s vollen=%d vol=%s\n",
               now, level, key, name, root, prev, strlen(data), data);
   }

   if (bwrite(&fp, tmp, l) != l) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to update the job history. ERR=%s\n",
           be.bstrerror(errno));
   }

   bclose(&fp);

bail_out:
   V(joblist_mutex);
   free_pool_memory(tmp);
}

/* Prune jobs at the end of the job, this function can generate commands
 * in order to cleanup something
 */
void joblist::prune_jobs(char *build_cmd(void *arg, const char *data, const char *job), 
                         void *arg, alist *jobs)
{
   BFILE fp, fpout;
   FILE *f=NULL;
   POOLMEM *tmp;
   POOLMEM *tmpout;
   POOLMEM *data;
   POOLMEM *buf;
   char curkey[MAX_NAME_LENGTH];  /* key */
   char jobname[MAX_NAME_LENGTH]; /* jobname */
   char prevjob[MAX_NAME_LENGTH]; /* last jobname */
   char rootjob[MAX_NAME_LENGTH]; /* root jobname */
   char t[MAX_NAME_LENGTH];
   uint32_t datalen;
   char curlevel;
   bool keep;
   bool ok=false;
   int count=0, len;

   /* In Incremental, it means that the previous Full/Diff is well terminated */
   if (level != 'I') {
      return;
   }

   find_root_job();

   binit(&fp);
   set_portable_backup(&fp);

   binit(&fpout);
   set_portable_backup(&fpout);

   tmp = get_pool_memory(PM_FNAME);
   Mmsg(tmp, "%s/%s", working, base);

   tmpout = get_pool_memory(PM_FNAME);
   Mmsg(tmpout, "%s/%s.swap", working, base);

   buf = get_pool_memory(PM_FNAME);
   data = get_pool_memory(PM_FNAME);
   *buf = *data = 0;

   P(joblist_mutex);
   if (bopen(&fp, tmp, O_RDONLY, 0) < 0) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to prune previous jobs. "
           "Can't open %s for reading ERR=%s\n",
           tmp, be.bstrerror(errno));
      goto bail_out;
   }
   if (bopen(&fpout, tmpout, O_CREAT|O_WRONLY, 0600) < 0) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to prune previous jobs. "
           "Can't open %s for writing ERR=%s\n",
           tmpout, be.bstrerror(errno));
      goto bail_out;
   }

   f = fdopen(fp.fid, "r");     /* we use fgets from open() */
   if (!f) {
      berrno be;
      Jmsg(ctx, M_ERROR, "Unable to prune previous jobs. ERR=%s\n",
           be.bstrerror(errno));
      goto bail_out;
   }

   while (fgets(buf, sizeof_pool_memory(buf), f) != NULL) {
      *data = *curkey = *jobname = *rootjob = *prevjob = 0;
      keep = false;
      datalen = 0;
      
      len = strlen(buf);
      if (len > 0 && buf[len -1] != '\n') {
         /* The line is larger than the buffer, we need to capture the rest */
         bool ok=false;
         while (!ok) {
            Dmsg(ctx, dbglvl+100, "Reading extra 1024 bytes, len=%d\n", len);
            buf = check_pool_memory_size(buf, sizeof_pool_memory(buf) + 1024);
            if (fgets(buf + len, 1023, f) == NULL) {
               ok = true;
            }
            len = strlen(buf);
            if (buf[len - 1] == '\n') {
               ok = true;
            }
            if (len > 32000) {  /* sanity check */
               ok = true;
            }
         }
      }

      /* We don't capture the vol list, because our sscanf is limited to 1000 bytes  */
      if (sscanf(buf, "time=%60s level=%c key=%127s name=%127s root=%127s prev=%127s vollen=%d vol=", 
                 t, &curlevel, curkey, jobname, rootjob, prevjob, &datalen) != 7) {

         if (sscanf(buf, "time=%60s level=F key=%127s name=%127s vollen=%d vol=", 
                    t, curkey, jobname, &datalen) == 4) {
            *rootdiff = *rootjob = *prevjob = 0;
            curlevel = 'F';

         } else {
            Dmsg(ctx, dbglvl+100, "Bad line l=[%s]\n", buf);
            keep = true;
         }
      }
      
      if (!keep) {
         pm_strcpy(data, strstr(buf, " vol=") +  5);
         strip_trailing_newline(data);
         unbash_spaces(data);

         if (datalen != strlen(data)) {
            Dmsg(ctx, dbglvl+100, "Bad data line datalen != strlen(data) %d != %d\n", datalen, strlen(data)); 
            Dmsg(ctx, dbglvl+100, "v=[%s]\n", data);
         }
      }


      if (!keep &&
          (strcmp(key,  curkey)  != 0 ||
           strcmp(name, jobname) == 0 ||
           strcmp(prev, jobname) == 0 ||
           strcmp(root, jobname) == 0 ||
           strcmp(rootdiff, jobname) == 0))
      {
         keep = true;
      } 
      
      if (keep) {
         if (bwrite(&fpout, buf, len) < 0) {
            berrno be;
            Jmsg(ctx, M_ERROR, "Unable to update the job history. ERR=%s\n",
                 be.bstrerror(errno));
            goto bail_out;
         }

      } else if (build_cmd) {
         count++;
         Dmsg(ctx, dbglvl+100, "Can prune jobname %s\n", jobname);

         char *p2 = data;
         for(char *p = data; *p; p++) {
            if (*p == ',') {
               *p = 0;
               jobs->append(bstrdup(build_cmd(arg, p2, jobname)));
               p2 = p + 1 ;
            }
         }
         jobs->append(bstrdup(build_cmd(arg, p2, jobname)));

      } else if (jobs) {
         jobs->append(bstrdup(data));
      }
   }

   ok = true;

bail_out:
   if (f) {
      fclose(f);
   }
   if (is_bopen(&fpout)) {
      bclose(&fpout);
   }

   /* We can switch the file */
   if (ok) {
      unlink(tmp);
      
      if (rename(tmpout, tmp) < 0) {
         berrno be;
         Jmsg(ctx, M_ERROR, "Unable to update the job history. ERR=%s\n",
              be.bstrerror(errno));
      }
   }

   V(joblist_mutex);
   free_pool_memory(tmp);
   free_pool_memory(tmpout);
   free_pool_memory(data);
   free_pool_memory(buf);

   Dmsg(ctx, dbglvl+100, "Pruning %d jobs\n", count);
}


#endif  /* ! USE_JOB_LIST */

