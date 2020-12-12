/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2020 Kern Sibbald

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
/**
 * Major refactoring of XATTR code written by:
 *
 *  Radosław Korzeniewski, MMXVI
 *  radoslaw@korzeniewski.net, radekk@inteos.pl
 *  Inteos Sp. z o.o. http://www.inteos.pl/
 *
 */

#include "bacula.h"
#include "filed.h"
#include "bxattr_linux.h"

#if defined(HAVE_LINUX_OS)

/* check if XATTR support is enabled */
#if defined(HAVE_XATTR)

/*
 * Define the supported XATTR streams for this OS
 */
static const int os_xattr_streams[] = {
   STREAM_XACL_LINUX_XATTR,
   0
};

static const char *os_xattr_acl_skiplist[] = {
   "system.posix_acl_access",
   "system.posix_acl_default",
   NULL
};

static const char *os_xattr_skiplist[] = {
   NULL
};

/*
 * OS specific constructor
 */
BXATTR_Linux::BXATTR_Linux(){
   set_xattr_streams(os_xattr_streams);
   set_xattr_skiplists(os_xattr_skiplist, os_xattr_acl_skiplist);
};

/*
 * Perform OS specific extended attribute backup
 *
 * in/out - check API at bxattr.h
 */
bRC_BXATTR BXATTR_Linux::os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt){
   return generic_backup_xattr(jcr, ff_pkt);
};

/*
 * Perform OS specific XATTR restore. Runtime is called only when stream is supported by OS.
 *
 * in/out - check API at bxattr.h
 */
bRC_BXATTR BXATTR_Linux::os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length){
   return generic_restore_xattr(jcr, stream);
};

#define CIFS_XATTR "system.cifs_acl"
#define CIFS_XATTR_LEN 15

/*
 * Return a list of xattr names in newly allocated pool memory and a length of the allocated buffer.
 * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
 * when not needed.
 *
 * in/out - check API at bxattr.h
 */
bRC_BXATTR BXATTR_Linux::os_get_xattr_names (JCR *jcr, POOLMEM ** pxlist, uint32_t * xlen){

   int len, len2;
   POOLMEM * list;
   bool append_cifs=false;

   /* check input data */
   if (jcr == NULL || xlen == NULL || pxlist == NULL){
      return bRC_BXATTR_inval;
   }

   /* With CIFS and CIFS2, the system.cifs_acl attribute holds the BackupRead
    * information, but it is not listed by llistxattr
    */
   if (strncmp(get_current_fs(), "cifs", 4) == 0) {
      append_cifs = true;
   }

   /* get the length of the extended attributes */
   len = llistxattr(jcr->last_fname, NULL, 0);
   switch (len){
      case -1: {
         berrno be;

         switch (errno){
            case ENOENT:
               /* no file available, skip it */
               return bRC_BXATTR_skip;
            case EOPNOTSUPP:
               /* no xattr supported on filesystem, clear a flag and skip it */
               clear_flag(BXATTR_FLAG_NATIVE);
               set_content(NULL);
               return bRC_BXATTR_skip;
            default:
               Mmsg2(jcr->errmsg, _("llistxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg2(100, "llistxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
               return bRC_BXATTR_error;
         }
         break;
      }
      case 0:
         if (append_cifs) {     /* We have nothing, but the cifs attribute can be hidden */
            len += CIFS_XATTR_LEN + 1;
         } else {
            /* xattr available but empty, skip it */
            return bRC_BXATTR_skip;
         }
      default:
         break;
   }

   /*
    * allocate memory for the extented attribute list
    * default size is a 4k for PM_BSOCK, which should be sufficient on almost all
    * Linux system where xattrs a limited in size to single filesystem block ~4kB
    * so we need to check required size
    */
   list = get_pool_memory(PM_BSOCK);
   list = check_pool_memory_size(list, len + 1);
   memset(list, 0, len + 1);

   /* get the list of extended attributes names for a file */
   len2 = llistxattr(jcr->last_fname, list, len);
   switch (len2){
   case -1: {
      berrno be;

      switch (errno){
      case ENOENT:
         /* no file available, skip it, first release allocated memory */
         free_pool_memory(list);
         return bRC_BXATTR_skip;
      default:
         Mmsg2(jcr->errmsg, _("llistxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "llistxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         free_pool_memory(list);
         return bRC_BXATTR_error;
      }
      break;
   }
   default:
      break;
   }
   /* ensure a list is nul terminated */
   list[len2] = '\0';
   if (append_cifs) {
      len2 = xattr_list_append(list, len2, CIFS_XATTR, CIFS_XATTR_LEN);
   }
   /* setup return data */
   *pxlist = list;
   *xlen = len2;
   return bRC_BXATTR_ok;
};

/*
 * Return a value of the requested attribute name and a length of the allocated buffer.
 * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
 * when not needed.
 *
 * in/out - check API at bxattr.h
 */
bRC_BXATTR BXATTR_Linux::os_get_xattr_value (JCR *jcr, char * name, char ** pvalue, uint32_t * plen){

   int len;
   POOLMEM * value;

   /* check input data */
   if (jcr == NULL || name == NULL || plen == NULL || pvalue == NULL){
      return bRC_BXATTR_inval;
   }

   /* get the length of the value for extended attribute */
   len = lgetxattr(jcr->last_fname, name, NULL, 0);
   switch (len){
      case -1: {
         berrno be;

         switch (errno){
            case ENOENT:
               /* no file available, skip it */
               return bRC_BXATTR_skip;
            default:
               /* XXX: what about ENOATTR error value? */
               Mmsg2(jcr->errmsg, _("lgetxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg2(100, "lgetxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
               return bRC_BXATTR_error;
         }
         break;
      }
      default:
         break;
   }

   if (len > 0){
      /*
       * allocate memory for the extented attribute value
       * default size is a 256B for PM_MESSAGE, so we need to check required size
       */
      value = get_pool_memory(PM_MESSAGE);
      value = check_pool_memory_size(value, len + 1);
      memset(value, 0, len + 1);
      /* value is not empty, get a data */
      len = lgetxattr(jcr->last_fname, name, value, len);
      switch (len){
      case -1: {
         berrno be;

         switch (errno){
         case ENOENT:
            /* no file available, skip it, first release allocated memory */
            free_pool_memory(value);
            return bRC_BXATTR_skip;
         default:
            Mmsg2(jcr->errmsg, _("lgetxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
            Dmsg2(100, "lgetxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
            free_pool_memory(value);
            return bRC_BXATTR_error;
         }
         break;
      }
      default:
         break;
      }
      /* ensure a value is nul terminated */
      value[len] = '\0';
   } else {
      /* empty value */
      value = NULL;
      len = 0;
   }
   /* setup return data */
   *pvalue = value;
   *plen = len;
   return bRC_BXATTR_ok;
};

/*
 * Low level OS specific runtime to set extended attribute on file
 *
 * in/out - check API at bxattr.h
 */
bRC_BXATTR BXATTR_Linux::os_set_xattr (JCR *jcr, BXATTR_xattr *xattr){

   /* check input data */
   if (jcr == NULL || xattr == NULL){
      return bRC_BXATTR_inval;
   }

   /* set extattr on file */
   if (lsetxattr(jcr->last_fname, xattr->name, xattr->value, xattr->value_len, 0) != 0){
      berrno be;

      switch (errno){
      case ENOENT:
         break;
      case ENOTSUP:
         /*
          * If the filesystem reports it doesn't support XATTR we clear the
          * BXATTR_FLAG_NATIVE flag so we skip XATTR restores on all other files
          * on the same filesystem. The BXATTR_FLAG_NATIVE flag gets set again
          * when we change from one filesystem to an other.
          */
         clear_flag(BXATTR_FLAG_NATIVE);
         Mmsg(jcr->errmsg, _("setxattr error on file \"%s\": filesystem doesn't support XATTR\n"), jcr->last_fname);
         Dmsg3(100, "setxattr error name=%s value=%s file=%s filesystem doesn't support XATTR\n", xattr->name, xattr->value, jcr->last_fname);
         break;
      default:
         Mmsg2(jcr->errmsg, _("setxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "setxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         return bRC_BXATTR_error;
      }
   }
   return bRC_BXATTR_ok;
};

#endif /* HAVE_XATTR */

#endif /* HAVE_LINUX_OS */
