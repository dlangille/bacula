/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2016 Kern Sibbald

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
 * Major refactoring of ACL and XATTR code written by:
 *
 *  Radosław Korzeniewski, MMXVI
 *  radoslaw@korzeniewski.net, radekk@inteos.pl
 *  Inteos Sp. z o.o. http://www.inteos.pl/
 *
 */

#include "bacula.h"
#include "filed.h"
#include "xacl_linux.h"

#if defined(HAVE_LINUX_OS)
/*
 * Define the supported ACL streams for this OS
 */
static const int os_acl_streams[] = {
   STREAM_XACL_LINUX_ACCESS,
   0
};

static const int os_default_acl_streams[] = {
   STREAM_XACL_LINUX_DEFAULT,
   0
};

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
 * OS Specyfic constructor
 */
XACL_Linux::XACL_Linux(){
   set_acl_streams(os_acl_streams, os_default_acl_streams);
   set_xattr_streams(os_xattr_streams);
   set_xattr_skiplists(os_xattr_skiplist, os_xattr_acl_skiplist);
};

/*
 * Translates Bacula internal acl representation into
 * acl type
 *
 * in:
 *    xacltype - internal Bacula acl type (XACL_type)
 * out:
 *    acl_type_t - os dependent acl type
 *    when failed - ACL_TYPE_NONE is returned
 */
acl_type_t XACL_Linux::get_acltype(XACL_type xacltype){

   acl_type_t acltype;

   switch (xacltype){
      case XACL_TYPE_ACCESS:
         acltype = ACL_TYPE_ACCESS;
         break;
      case XACL_TYPE_DEFAULT:
         acltype = ACL_TYPE_DEFAULT;
         break;
      default:
         /*
          * sanity check for acl's not supported by OS
          */
         acltype = (acl_type_t)ACL_TYPE_NONE;
         break;
   }
   return acltype;
};

/*
 * Counts a number of acl entries
 *
 * in:
 *    acl - acl object
 * out:
 *    int - number of entries in acl object
 *    when no acl entry available or any error then return zero '0'
 */
int XACL_Linux::acl_nrentries(acl_t acl){

   int nr = 0;
   acl_entry_t aclentry;
   int rc;

   rc = acl_get_entry(acl, ACL_FIRST_ENTRY, &aclentry);
   while (rc == 1){
      nr++;
      rc = acl_get_entry(acl, ACL_NEXT_ENTRY, &aclentry);
   }

   return nr;
};

/*
 * Checks if acl is simple.
 *
 * acl is simple if it has only the following entries:
 * "user::",
 * "group::",
 * "other::"
 *
 * in:
 *    acl - acl object
 * out:
 *    true - when acl object is simple
 *    false - when acl object is not simple
 */
bool XACL_Linux::acl_issimple(acl_t acl){

   acl_entry_t aclentry;
   acl_tag_t acltag;
   int rc;

   rc = acl_get_entry(acl, ACL_FIRST_ENTRY, &aclentry);
   while (rc == 1){
      if (acl_get_tag_type(aclentry, &acltag) < 0){
         return true;
      }
      /*
       * Check for ACL_USER_OBJ, ACL_GROUP_OBJ or ACL_OTHER to find out.
       */
      if (acltag != ACL_USER_OBJ &&
          acltag != ACL_GROUP_OBJ &&
          acltag != ACL_OTHER){
         return false;
      }
      rc = acl_get_entry(acl, ACL_NEXT_ENTRY, &aclentry);
   }
   return true;
};

/*
 * Perform OS specyfic ACL backup
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_backup_acl (JCR *jcr, FF_PKT *ff_pkt){
   return generic_backup_acl(jcr, ff_pkt);
};

/*
 * Perform OS specyfic ACL restore
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_restore_acl (JCR *jcr, int stream, char *content, uint32_t length){
   return generic_restore_acl(jcr, stream);
};

/*
 * Perform OS specyfic extended attribute backup
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt){
   return generic_backup_xattr(jcr, ff_pkt);
};

/*
 * Perform OS specyfic XATTR restore. Runtime is called only when stream is supported by OS.
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length){
   return generic_restore_xattr(jcr, stream);
};

/*
 * Low level OS specyfic runtime to get ACL data from file. The ACL data is set in internal content buffer.
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_get_acl(JCR *jcr, XACL_type xacltype){

   acl_t acl;
   acl_type_t acltype;
   char *acltext;
   bRC_XACL rc = bRC_XACL_ok;

   /* check input data */
   if (jcr == NULL){
      return bRC_XACL_inval;
   }

   acltype = get_acltype(xacltype);
   acl = acl_get_file(jcr->last_fname, acltype);

   if (acl){
      Dmsg1(400, "OS_ACL read from file: %s\n",jcr->last_fname);
      if (acl_nrentries(acl) == 0){
         goto bail_out;
      }

      /* check for fimple ACL which correspond to standard permissions only */
      if (xacltype == XACL_TYPE_ACCESS && acl_issimple(acl)){
         goto bail_out;
      }

      if ((acltext = acl_to_text(acl, NULL)) != NULL){
         set_content(acltext);
         acl_free(acl);
         acl_free(acltext);
         return bRC_XACL_ok;
      }

      berrno be;

      Mmsg2(jcr->errmsg, _("acl_to_text error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
      Dmsg2(100, "acl_to_text error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());

      rc = bRC_XACL_error;
   } else {
      berrno be;

      switch (errno){
      case EOPNOTSUPP:
         /* fs does not support acl, skip it */
         Dmsg0(400, "Wow, ACL is not supported on this filesystem\n");
         clear_flag(XACL_FLAG_NATIVE);
         break;
      case ENOENT:
         break;
      default:
         /* Some real error */
         Mmsg2(jcr->errmsg, _("acl_get_file error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "acl_get_file error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         rc = bRC_XACL_error;
         break;
      }
   }

bail_out:
   if (acl){
      acl_free(acl);
   }
   /*
    * it is a bit of hardcore to clear a poolmemory with a NULL pointer,
    * but it is working, hehe :)
    * you may ask why it is working? it is simple, a pm_strcpy function is handling
    * a null pointer with a substitiution of empty string.
    */
   set_content(NULL);
   return rc;
};

/*
 * Low level OS specyfic runtime to set ACL data on file
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_set_acl(JCR *jcr, XACL_type xacltype, char *content, uint32_t length){

   acl_t acl;
   acl_type_t acltype;

   /* check input data */
   if (jcr == NULL || content == NULL){
      return bRC_XACL_inval;
   }

   acl = acl_from_text(content);
   if (acl == NULL){
      berrno be;

      Mmsg2(jcr->errmsg, _("acl_from_text error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
      Dmsg3(100, "acl_from_text error acl=%s file=%s ERR=%s\n", content, jcr->last_fname, be.bstrerror());
      return bRC_XACL_error;
   }

   if (acl_valid(acl) != 0){
      berrno be;

      Mmsg2(jcr->errmsg, _("acl_valid error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
      Dmsg3(100, "acl_valid error acl=%s file=%s ERR=%s\n", content, jcr->last_fname, be.bstrerror());
      acl_free(acl);
      return bRC_XACL_error;
   }

   /* handle different acl types for Linux */
   acltype = get_acltype(xacltype);
   if (acltype == ACL_TYPE_DEFAULT && length == 0){
      /* delete ACl from file when no acl data available for default acl's */
      if (acl_delete_def_file(jcr->last_fname) == 0){
         return bRC_XACL_ok;
      }

      berrno be;
      switch (errno){
         case ENOENT:
            return bRC_XACL_ok;
         case ENOTSUP:
            /*
             * If the filesystem reports it doesn't support acl's we clear the
             * XACL_FLAG_NATIVE flag so we skip ACL restores on all other files
             * on the same filesystem. The XACL_FLAG_NATIVE flag gets set again
             * when we change from one filesystem to an other.
             */
            clear_flag(XACL_FLAG_NATIVE);
            Mmsg(jcr->errmsg, _("acl_delete_def_file error on file \"%s\": filesystem doesn't support ACLs\n"), jcr->last_fname);
            return bRC_XACL_error;
         default:
            Mmsg2(jcr->errmsg, _("acl_delete_def_file error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
            return bRC_XACL_error;
      }
   }

   /*
    * Restore the ACLs, but don't complain about links which really should
    * not have attributes, and the file it is linked to may not yet be restored.
    * This is only true for the old acl streams as in the new implementation we
    * don't save acls of symlinks (which cannot have acls anyhow)
    */
   if (acl_set_file(jcr->last_fname, acltype, acl) != 0 && jcr->last_type != FT_LNK){
      berrno be;
      switch (errno){
      case ENOENT:
         acl_free(acl);
         return bRC_XACL_ok;
      case ENOTSUP:
         /*
          * If the filesystem reports it doesn't support ACLs we clear the
          * XACL_FLAG_NATIVE flag so we skip ACL restores on all other files
          * on the same filesystem. The XACL_FLAG_NATIVE flag gets set again
          * when we change from one filesystem to an other.
          */
         clear_flag(XACL_FLAG_NATIVE);
         Mmsg(jcr->errmsg, _("acl_set_file error on file \"%s\": filesystem doesn't support ACLs\n"), jcr->last_fname);
         Dmsg2(100, "acl_set_file error acl=%s file=%s filesystem doesn't support ACLs\n", content, jcr->last_fname);
         acl_free(acl);
         return bRC_XACL_error;
      default:
         Mmsg2(jcr->errmsg, _("acl_set_file error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg3(100, "acl_set_file error acl=%s file=%s ERR=%s\n", content, jcr->last_fname, be.bstrerror());
         acl_free(acl);
         return bRC_XACL_error;
      }
   }
   acl_free(acl);
   return bRC_XACL_ok;
};

/*
 * Return a list of xattr names in newly allocated pool memory and a length of the allocated buffer.
 * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
 * when not needed.
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_get_xattr_names (JCR *jcr, POOLMEM ** pxlist, uint32_t * xlen){

   int len;
   POOLMEM * list;

   /* check input data */
   if (jcr == NULL || xlen == NULL || pxlist == NULL){
      return bRC_XACL_inval;
   }

   /* get the length of the extended attributes */
   len = llistxattr(jcr->last_fname, NULL, 0);
   switch (len){
      case -1: {
         berrno be;

         switch (errno){
            case ENOENT:
               /* no file available, skip it */
               return bRC_XACL_skip;
            case EOPNOTSUPP:
               /* no xattr supported on filesystem, clear a flag and skip it */
               clear_flag(XACL_FLAG_NATIVE);
               set_content(NULL);
               return bRC_XACL_skip;
            default:
               Mmsg2(jcr->errmsg, _("llistxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg2(100, "llistxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
               return bRC_XACL_error;
         }
         break;
      }
      case 0:
         /* xattr available but empty, skip it */
         return bRC_XACL_skip;
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
   len = llistxattr(jcr->last_fname, list, len);
   switch (len){
   case -1: {
      berrno be;

      switch (errno){
      case ENOENT:
         /* no file available, skip it, first release allocated memory */
         free_pool_memory(list);
         return bRC_XACL_skip;
      default:
         Mmsg2(jcr->errmsg, _("llistxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "llistxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         free_pool_memory(list);
         return bRC_XACL_error;
      }
      break;
   }
   default:
      break;
   }
   /* ensure a list is nul terminated */
   list[len] = '\0';
   /* setup return data */
   *pxlist = list;
   *xlen = len;
   return bRC_XACL_ok;
};

/*
 * Return a value of the requested attribute name and a length of the allocated buffer.
 * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
 * when not needed.
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_get_xattr_value (JCR *jcr, char * name, char ** pvalue, uint32_t * plen){

   int len;
   POOLMEM * value;

   /* check input data */
   if (jcr == NULL || name == NULL || plen == NULL || pvalue == NULL){
      return bRC_XACL_inval;
   }

   /* get the length of the value for extended attribute */
   len = lgetxattr(jcr->last_fname, name, NULL, 0);
   switch (len){
      case -1: {
         berrno be;

         switch (errno){
            case ENOENT:
               /* no file available, skip it */
               return bRC_XACL_skip;
            default:
               /* XXX: what about ENOATTR error value? */
               Mmsg2(jcr->errmsg, _("lgetxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg2(100, "lgetxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
               return bRC_XACL_error;
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
            return bRC_XACL_skip;
         default:
            Mmsg2(jcr->errmsg, _("lgetxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
            Dmsg2(100, "lgetxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
            free_pool_memory(value);
            return bRC_XACL_error;
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
   return bRC_XACL_ok;
};

/*
 * Low level OS specyfic runtime to set extended attribute on file
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_Linux::os_set_xattr (JCR *jcr, XACL_xattr *xattr){

   /* check input data */
   if (jcr == NULL || xattr == NULL){
      return bRC_XACL_inval;
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
          * XACL_FLAG_NATIVE flag so we skip XATTR restores on all other files
          * on the same filesystem. The XACL_FLAG_NATIVE flag gets set again
          * when we change from one filesystem to an other.
          */
         clear_flag(XACL_FLAG_NATIVE);
         Mmsg(jcr->errmsg, _("setxattr error on file \"%s\": filesystem doesn't support XATTR\n"), jcr->last_fname);
         Dmsg3(100, "setxattr error name=%s value=%s file=%s filesystem doesn't support XATTR\n", xattr->name, xattr->value, jcr->last_fname);
         break;
      default:
         Mmsg2(jcr->errmsg, _("setxattr error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "setxattr error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         return bRC_XACL_error;
      }
   }
   return bRC_XACL_ok;
};

#endif /* HAVE_LINUX_OS */
