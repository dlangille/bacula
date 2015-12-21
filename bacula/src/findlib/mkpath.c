/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2015 Kern Sibbald

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
 *  Kern Sibbald, September MMVII
 *
 *  This is tricky code, especially when writing from scratch. Fortunately,
 *    a non-copyrighted version of mkdir was available to consult.
 *
 * ***FIXME*** the mkpath code could be significantly optimized by
 *   walking up the path chain from the bottom until it either gets
 *   to the top or finds an existing directory then walk back down
 *   creating the path components.  Currently, it always starts at
 *   the top, which can be rather inefficient for long path names.
 *
 */
#include "bacula.h"
#include "jcr.h"

#define dbglvl 50

/*
 * For old systems that don't have lchown() or lchmod()
 */
#ifndef HAVE_LCHOWN
#define lchown chown
#endif
#ifndef HAVE_LCHMOD
#define lchmod chmod
#endif


typedef struct PrivateCurDir {
   hlink link;
   char fname[1];
} CurDir;

/* Initialize the path hash table */
static bool path_list_init(JCR *jcr)
{
   CurDir *elt = NULL;
   jcr->path_list = (htable *)malloc(sizeof(htable));

   /* Hard to know in advance how many directories will
    * be stored in this hash
    */
   jcr->path_list->init(elt, &elt->link, 10000);
   return true;
}

/* Add a path to the hash when we create a directory
 * with the replace=NEVER option
 */
bool path_list_add(JCR *jcr, uint32_t len, char *fname)
{
   bool ret = true;
   CurDir *item;

   if (!jcr->path_list) {
      path_list_init(jcr);
   }

   /* we store CurDir, fname in the same chunk */
   item = (CurDir *)jcr->path_list->hash_malloc(sizeof(CurDir)+len+1);

   memset(item, 0, sizeof(CurDir));
   memcpy(item->fname, fname, len+1);

   jcr->path_list->insert(item->fname, item);

   Dmsg1(dbglvl, "add fname=<%s>\n", fname);
   return ret;
}

void free_path_list(JCR *jcr)
{
   if (jcr->path_list) {
      jcr->path_list->destroy();
      free(jcr->path_list);
      jcr->path_list = NULL;
   }
}

bool path_list_lookup(JCR *jcr, char *fname)
{
   bool found=false;
   char bkp;

   if (!jcr->path_list) {
      return false;
   }

   /* Strip trailing / */
   int len = strlen(fname);
   if (len == 0) {
      return false;
   }
   len--;
   bkp = fname[len];
   if (fname[len] == '/') {       /* strip any trailing slash */
      fname[len] = 0;
   }

   CurDir *temp = (CurDir *)jcr->path_list->lookup(fname);
   if (temp) {
      found=true;
   }

   Dmsg2(dbglvl, "lookup <%s> %s\n", fname, found?"ok":"not ok");

   fname[len] = bkp;            /* restore last / */
   return found;
}

static bool makedir(JCR *jcr, char *path, mode_t mode, int *created)
{
   struct stat statp;

   if (mkdir(path, mode) != 0) {
      berrno be;
      *created = false;
      if (lstat(path, &statp) != 0) {
         Jmsg2(jcr, M_ERROR, 0, _("Cannot create directory %s: ERR=%s\n"),
              path, be.bstrerror());
         return false;
      } else if (!S_ISDIR(statp.st_mode)) {
         Jmsg1(jcr, M_ERROR, 0, _("%s exists but is not a directory.\n"), path);
         return false;
      }
      return true;                 /* directory exists */
   }
#if 0
   /* TODO: This code rely on statp that is not initialized, we need to do a stat() */
   if (S_ISLNK(statp.st_mode)) {
      /*
       * Note, we created a directory, not a link, so if we find a
       *  link, there is a security problem here.
       */
      Jmsg1(jcr, M_FATAL, 0, _("Security problem!! We created directory %s, but it is a link.\n"),
         path);
      return false;
   }
#endif
   if (jcr->keep_path_list) {
      /* When replace=NEVER, we keep track of all directories newly created */
      path_list_add(jcr, strlen(path), path);
   }

   *created = true;
   return true;
}

/*
 * Restore the owner and permissions (mode) of a Directory.
 *  See attribs.c for the equivalent for files.
 */
static void set_own_mod(ATTR *attr, char *path, uid_t owner, gid_t group, mode_t mode)
{
   if (lchown(path, owner, group) != 0 && attr->uid == 0
#ifdef AFS
        && errno != EPERM
#endif
   ) {
      berrno be;
      Jmsg2(attr->jcr, M_WARNING, 0, _("Cannot change owner and/or group of %s: ERR=%s\n"),
           path, be.bstrerror());
   }
   if (lchmod(path, mode) != 0 && attr->uid == 0) {
      berrno be;
      Jmsg2(attr->jcr, M_WARNING, 0, _("Cannot change permissions of %s: ERR=%s\n"),
           path, be.bstrerror());
   }
}

/*
 * mode is the mode bits to use in creating a new directory
 *
 * parent_mode are the parent's modes if we need to create parent
 *    directories.
 *
 * owner and group are to set on any created dirs
 *
 * keep_dir_modes if set means don't change mode bits if dir exists
 */
bool makepath(ATTR *attr, const char *apath, mode_t mode, mode_t parent_mode,
            uid_t owner, gid_t group, int keep_dir_modes)
{
   struct stat statp;
   mode_t omask, tmode;
   char *path = (char *)apath;
   char *p;
   int len;
   bool ok = false;
   int created;
   char new_dir[5000];
   int ndir = 0;
   int i = 0;
   int max_dirs = (int)sizeof(new_dir);
   JCR *jcr = attr->jcr;

   if (stat(path, &statp) == 0) {     /* Does dir exist? */
      if (!S_ISDIR(statp.st_mode)) {
         Jmsg1(jcr, M_ERROR, 0, _("%s exists but is not a directory.\n"), path);
         return false;
      }
      /* Full path exists */
      if (keep_dir_modes) {
         return true;
      }
      set_own_mod(attr, path, owner, group, mode);
      return true;
   }
   omask = umask(0);
   umask(omask);
   len = strlen(apath);
   path = (char *)alloca(len+1);
   bstrncpy(path, apath, len+1);
   strip_trailing_slashes(path);
   /*
    * Now for one of the complexities. If we are not running as root,
    *  then if the parent_mode does not have wx user perms, or we are
    *  setting the userid or group, and the parent_mode has setuid, setgid,
    *  or sticky bits, we must create the dir with open permissions, then
    *  go back and patch all the dirs up with the correct perms.
    * Solution, set everything to 0777, then go back and reset them at the
    *  end.
    */
   tmode = 0777;

   p = path;

   /* Skip leading slash(es) */
   while (IsPathSeparator(*p)) {
      p++;
   }
   while ((p = first_path_separator(p))) {
      char save_p;
      save_p = *p;
      *p = 0;
      if (!makedir(jcr, path, tmode, &created)) {
         goto bail_out;
      }
      if (ndir < max_dirs) {
         new_dir[ndir++] = created;
      }
      *p = save_p;
      while (IsPathSeparator(*p)) {
         p++;
      }
   }
   /* Create final component */
   if (!makedir(jcr, path, tmode, &created)) {
      goto bail_out;
   }
   if (ndir < max_dirs) {
      new_dir[ndir++] = created;
   }
   if (ndir >= max_dirs) {
      Jmsg0(jcr, M_WARNING, 0, _("Too many subdirectories. Some permissions not reset.\n"));
   }

   /* Now set the proper owner and modes */
   p = path;
   /* Skip leading slash(es) */
   while (IsPathSeparator(*p)) {
      p++;
   }
   while ((p = first_path_separator(p))) {
      char save_p;
      save_p = *p;
      *p = 0;
      if (i < ndir && new_dir[i++] && !keep_dir_modes) {
         set_own_mod(attr, path, owner, group, parent_mode);
      }
      *p = save_p;
      while (IsPathSeparator(*p)) {
         p++;
      }
   }

   /* Set for final component */
   if (i < ndir && new_dir[i++]) {
      set_own_mod(attr, path, owner, group, mode);
   }

   ok = true;
bail_out:
   umask(omask);
   return ok;
}
