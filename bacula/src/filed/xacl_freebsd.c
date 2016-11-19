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
#include "xacl_freebsd.h"

#if defined(HAVE_FREEBSD_OS)
/*
 * Define the supported ACL streams for this OS
 */
static const int os_acl_streams[] = {
   STREAM_XACL_FREEBSD_ACCESS,
   STREAM_XACL_FREEBSD_NFS4,
   0
};

static const int os_default_acl_streams[] = {
   STREAM_XACL_FREEBSD_DEFAULT,
   0
};

/*
 * Define the supported XATTR streams for this OS
 */
static const int os_xattr_streams[] = {
   STREAM_XACL_FREEBSD_XATTR,
   0
};

static const int os_xattr_namespaces[] = {
   EXTATTR_NAMESPACE_USER,
   EXTATTR_NAMESPACE_SYSTEM,
   -1
};

static const char *os_xattr_acl_skiplist[] = {
   "system.posix1e.acl_access",
   "system.posix1e.acl_default",
   "system.nfs4.acl",
   NULL
};

static const char *os_xattr_skiplist[] = {
   NULL
};

/*
 * OS Specyfic constructor
 */
XACL_FreeBSD::XACL_FreeBSD(){

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
acl_type_t XACL_FreeBSD::get_acltype(XACL_type xacltype){

   acl_type_t acltype;

   switch (xacltype){
#ifdef HAVE_ACL_TYPE_NFS4
      case XACL_TYPE_NFS4:
         acltype = ACL_TYPE_NFS4;
         break;
#endif
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
int XACL_FreeBSD::acl_nrentries(acl_t acl){

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
bool XACL_FreeBSD::acl_issimple(acl_t acl){

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
 * Checks if ACL's are available for a specified file
 *
 * in:
 *    jcr - Job Control Record
 *    name - specifies the system variable to be queried
 * out:
 *    bRC_XACL_ok - check successful, lets setup xacltype variable
 *    bRC_XACL_error -  in case of error
 *    bRC_XACL_skip - you should skip all other routine
 *    bRC_XACL_cont - you need to continue your routine
 */
bRC_XACL XACL_FreeBSD::check_xacltype (JCR *jcr, int name){

   int aclrc = 0;

   aclrc = pathconf(jcr->last_fname, name);
   switch (aclrc){
      case -1: {
         /* some error check why */
         berrno be;
         if (errno == ENOENT){
            /* file does not exist skip it */
            return bRC_XACL_skip;
         } else {
            Mmsg2(jcr->errmsg, _("pathconf error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
            Dmsg2(100, "pathconf error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
            return bRC_XACL_error;
         }
      }
      case 0:
         /* continue the routine */
         return bRC_XACL_cont;
      default:
         break;
   }
   return bRC_XACL_ok;
};

/*
 * Perform OS specyfic ACL backup
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_FreeBSD::os_backup_acl (JCR *jcr, FF_PKT *ff_pkt){

   bRC_XACL rc;
   XACL_type xacltype = XACL_TYPE_NONE;

#if defined(_PC_ACL_NFS4)
   /*
    * Check if filesystem supports NFS4 acls.
    */
   rc = check_xacltype(jcr, _PC_ACL_NFS4);
   switch (rc){
      case bRC_XACL_ok:
         xacltype = XACL_TYPE_NFS4;
         break;
      case bRC_XACL_skip:
         return bRC_XACL_ok;
      case bRC_XACL_cont:
         break;
      default:
         /* errors */
         return rc;
   }
#endif
   if (xacltype == XACL_TYPE_NONE){
      /*
       * Check if filesystem supports POSIX acls.
       */
      rc = check_xacltype(jcr, _PC_ACL_EXTENDED);
      switch (rc){
         case bRC_XACL_ok:
            xacltype = XACL_TYPE_ACCESS;
            break;
         case bRC_XACL_skip:
            return bRC_XACL_ok;
         case bRC_XACL_cont:
            break;
         default:
            /* errors */
            return rc;
      }
   }

   /* no ACL's available for file, so skip this filesystem */
   if (xacltype == XACL_TYPE_NONE){
      clear_flag(XACL_FLAG_NATIVE);
      /*
       * it is a bit of hardcore to clear a poolmemory with a NULL pointer,
       * but it is working, hehe :)
       * you may ask why it is working? it is simple, a pm_strcpy function is handling
       * a null pointer with a substitiution of empty string.
       */
      set_content(NULL);
      return bRC_XACL_ok;
   }

   switch (xacltype){
      case XACL_TYPE_NFS4:
         /*
          * Read NFS4 ACLs
          */
         if (os_get_acl(jcr, XACL_TYPE_NFS4) == bRC_XACL_fatal)
            return bRC_XACL_fatal;

         if (get_content_len() > 0){
            if (send_acl_stream(jcr, STREAM_XACL_FREEBSD_NFS4) == bRC_XACL_fatal)
               return bRC_XACL_fatal;
         }
         break;
      case XACL_TYPE_ACCESS:
         /*
          * Read access ACLs
          */
         if (os_get_acl(jcr, XACL_TYPE_ACCESS) == bRC_XACL_fatal)
            return bRC_XACL_fatal;

         if (get_content_len() > 0){
            if (send_acl_stream(jcr, STREAM_XACL_FREEBSD_ACCESS) == bRC_XACL_fatal)
               return bRC_XACL_fatal;
         }

         /*
          * Directories can have default ACLs too
          */
         if (ff_pkt->type == FT_DIREND){
            if (os_get_acl(jcr, XACL_TYPE_DEFAULT) == bRC_XACL_fatal)
               return bRC_XACL_fatal;
            if (get_content_len() > 0){
               if (send_acl_stream(jcr, STREAM_XACL_FREEBSD_DEFAULT) == bRC_XACL_fatal)
                  return bRC_XACL_fatal;
            }
         }
         break;
      default:
         break;
   }

   return bRC_XACL_ok;
};

/*
 * Perform OS specyfic ACL restore
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_FreeBSD::os_restore_acl (JCR *jcr, int stream, char *content, uint32_t length){

   int aclrc = 0;
   const char *acl_type_name;

   switch (stream){
      case STREAM_UNIX_ACCESS_ACL:
      case STREAM_XACL_FREEBSD_ACCESS:
      case STREAM_UNIX_DEFAULT_ACL:
      case STREAM_XACL_FREEBSD_DEFAULT:
         aclrc = pathconf(jcr->last_fname, _PC_ACL_EXTENDED);
         acl_type_name = "POSIX";
         break;
      case STREAM_XACL_FREEBSD_NFS4:
#if defined(_PC_ACL_NFS4)
         aclrc = pathconf(jcr->last_fname, _PC_ACL_NFS4);
#endif
         acl_type_name = "NFS4";
         break;
      default:
         acl_type_name = "unknown";
         break;
   }

   switch (aclrc){
      case -1: {
         berrno be;

         switch (errno){
            case ENOENT:
               return bRC_XACL_ok;
            default:
               Mmsg2(jcr->errmsg, _("pathconf error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg3(100, "pathconf error acl=%s file=%s ERR=%s\n", content, jcr->last_fname, be.bstrerror());
               return bRC_XACL_error;
         }
      }
      case 0:
         clear_flag(XACL_FLAG_NATIVE);
         Mmsg2(jcr->errmsg, _("Trying to restore acl on file \"%s\" on filesystem without %s acl support\n"), jcr->last_fname, acl_type_name);
         return bRC_XACL_error;
      default:
         break;
   }

   switch (stream){
      case STREAM_UNIX_ACCESS_ACL:
      case STREAM_XACL_FREEBSD_ACCESS:
         return os_set_acl(jcr, XACL_TYPE_ACCESS, content, length);
      case STREAM_UNIX_DEFAULT_ACL:
      case STREAM_XACL_FREEBSD_DEFAULT:
         return os_set_acl(jcr, XACL_TYPE_DEFAULT, content, length);
      case STREAM_XACL_FREEBSD_NFS4:
         return os_set_acl(jcr, XACL_TYPE_NFS4, content, length);
      default:
         break;
   }
   return bRC_XACL_error;
};

/*
 * Perform OS specyfic extended attribute backup
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_FreeBSD::os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt){

   bRC_XACL rc;
   POOLMEM *xlist;
   uint32_t xlen;
   char *name;
   uint32_t name_len;
   POOLMEM *value;
   uint32_t value_len;
   POOLMEM *name_gen;
   uint32_t name_gen_len;
   char * namespace_str;
   int namespace_len;
   bool skip;
   alist *xattr_list = NULL;
   int xattr_count = 0;
   uint32_t len = 0;
   XACL_xattr *xattr;
   int a;

   for (a = 0; os_xattr_namespaces[a] != -1; a++){ // loop through all available namespaces
      /* xlist is allocated as POOLMEM by os_get_xattr_names */
      rc = os_get_xattr_names(jcr, os_xattr_namespaces[a], &xlist, &xlen);
      switch (rc){
         case bRC_XACL_ok:
            /* it's ok, so go further */
            break;
         case bRC_XACL_skip:
         case bRC_XACL_cont:
            /* no xattr available, so skip rest of it */
            return bRC_XACL_ok;
         default:
            return rc;
      }

      /* get a string representation of the namespace */
      if (extattr_namespace_to_string(os_xattr_namespaces[a], &namespace_str) != 0){
         Mmsg2(jcr->errmsg, _("Failed to convert %d into namespace on file \"%s\"\n"), os_xattr_namespaces[a], jcr->last_fname);
         Dmsg2(100, "Failed to convert %d into namespace on file \"%s\"\n", os_xattr_namespaces[a], jcr->last_fname);
         goto bail_out;
      }
      namespace_len = strlen(namespace_str);

      /* follow the list of xattr names and get the values */
      for (name = xlist; (name - xlist) + 1 < xlen; name = strchr(name, '\0') + 1){
         name_len = strlen(name);
         name_gen = get_pool_memory(PM_FNAME);
         name_gen = check_pool_memory_size(name_gen, name_len + namespace_len + 2);
         bsnprintf(name_gen, name_len + namespace_len + 2, "%s.%s", namespace_str, name);
         name_gen_len = strlen(name_gen);

         skip =  check_xattr_skiplists(jcr, ff_pkt, name_gen);
         if (skip || name_len == 0){
            Dmsg1(100, "Skipping xattr named %s\n", name_gen);
            continue;
         }

         /* value is allocated as POOLMEM by os_get_xattr_value */
         rc = os_get_xattr_value(jcr, os_xattr_namespaces[a], name, &value, &value_len);
         switch (rc){
            case bRC_XACL_ok:
               /* it's ok, so go further */
               break;
            case bRC_XACL_skip:
               /* no xattr available, so skip rest of it */
               rc = bRC_XACL_ok;
               goto bail_out;
            default:
               /* error / fatal */
               goto bail_out;
         }

         /*
          * we have a name of the extended attribute in the name variable
          * and value of the extended attribute in the value variable
          * so we need to build a list
          */
         xattr = (XACL_xattr*)malloc(sizeof(XACL_xattr));
         xattr->name_len = name_gen_len;
         xattr->name = name_gen;
         xattr->value_len = value_len;
         xattr->value = value;
         /*       magic              name_len          name        value_len       value */
         len += sizeof(uint32_t) + sizeof(uint32_t) + name_gen_len + sizeof(uint32_t) + value_len;

         if (xattr_list == NULL){
            xattr_list = New(alist(10, not_owned_by_alist));
         }
         xattr_list->append(xattr);
         xattr_count++;
      }
      if (xattr_count > 0){
         /* serialize the stream */
         rc = serialize_xattr_stream(jcr, len, xattr_list);
         if (rc != bRC_XACL_ok){
            Mmsg(jcr->errmsg, _("Failed to serialize extended attributes on file \"%s\"\n"), jcr->last_fname);
            Dmsg1(100, "Failed to serialize extended attributes on file \"%s\"\n", jcr->last_fname);
            goto bail_out;
         } else {
            /* send data to SD */
            rc = send_xattr_stream(jcr, STREAM_XACL_FREEBSD_XATTR);
         }
      } else {
         rc = bRC_XACL_ok;
      }
   }
bail_out:
   /* free allocated data */
   if (xattr_list != NULL){
      foreach_alist(xattr, xattr_list){
         if (xattr == NULL){
            break;
         }
         if (xattr->name){
            free_pool_memory(name_gen);
         }
         if (xattr->value){
            free(xattr->value);
         }
         free(xattr);
      }
      delete xattr_list;
   }
   if (xlist != NULL){
      free(xlist);
   }

   return rc;
};

/*
 * Perform OS specyfic XATTR restore. Runtime is called only when stream is supported by OS.
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_FreeBSD::os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length){
   return generic_restore_xattr(jcr, stream);
};

/*
 * Low level OS specyfic runtime to get ACL data from file. The ACL data is set in internal content buffer
 *
 * in/out - check API at xacl.h
 */
bRC_XACL XACL_FreeBSD::os_get_acl(JCR *jcr, XACL_type xacltype){

   acl_t acl;
   acl_type_t acltype;
   char *acltext;
   bRC_XACL rc = bRC_XACL_ok;

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

#if defined(_PC_ACL_NFS4)
      if (xacltype == XACL_TYPE_NFS4){
         int trivial;
         if (acl_is_trivial_np(acl, &trivial) == 0){
            if (trivial == 1){
               goto bail_out;
            }
         }
      }
#endif

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
bRC_XACL XACL_FreeBSD::os_set_acl(JCR *jcr, XACL_type xacltype, char *content, uint32_t length){

   acl_t acl;
   acl_type_t acltype;

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

   acl = acl_from_text(content);
   if (acl == NULL){
      berrno be;

      Mmsg2(jcr->errmsg, _("acl_from_text error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
      Dmsg3(100, "acl_from_text error acl=%s file=%s ERR=%s\n", content, jcr->last_fname, be.bstrerror());
      return bRC_XACL_error;
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
 *
 * As a FreeBSD uses a different attributes name schema/format then this method is a very different
 * from a standard generic method because it uses a namespace (ns) value for os dependent optimization.
 */
bRC_XACL XACL_FreeBSD::os_get_xattr_names (JCR *jcr, int ns, POOLMEM ** pxlist, uint32_t * xlen){

   int len;
   POOLMEM * list;
   int a;
   int stra;
   POOLMEM * genlist;

   /* check input data */
   if (jcr == NULL || xlen == NULL || pxlist == NULL){
      return bRC_XACL_inval;
   }
   /* get the length of the extended attributes */
   len = extattr_list_link(jcr->last_fname, ns, NULL, 0);
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
            case EPERM:
               if (ns == EXTATTR_NAMESPACE_SYSTEM){
                  return bRC_XACL_cont;
               } /* else show error */
            default:
               Mmsg2(jcr->errmsg, _("extattr_list_link error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg2(100, "extattr_list_link error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
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
   len = extattr_list_link(jcr->last_fname, ns, list, len);
   switch (len){
   case -1: {
      berrno be;

      switch (errno){
      case ENOENT:
         /* no file available, skip it, first release allocated memory */
         free_pool_memory(list);
         return bRC_XACL_skip;
         case EPERM:
            if (ns == EXTATTR_NAMESPACE_SYSTEM){
               return bRC_XACL_cont;
            } /* else show error */
      default:
         Mmsg2(jcr->errmsg, _("extattr_list_link error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "extattr_list_link error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         free_pool_memory(list);
         return bRC_XACL_error;
      }
      break;
   }
   default:
      break;
   }
   /* convert FreeBSD list type to the generic one */
   genlist = get_pool_memory(PM_BSOCK);
   genlist = check_pool_memory_size(genlist, len + 1);
   memset(genlist, 0, len + 1);
   for (a = 0; a < len; a += list[a] + 1){
      stra = list[a];
      memcpy(genlist + a, list + a + 1, stra);
      genlist[a + stra] = '\0';
   }
   free_pool_memory(list);
   /* setup return data */
   *pxlist = genlist;
   *xlen = len;
   return bRC_XACL_ok;
};

/*
 * Return a value of the requested attribute name and a length of the allocated buffer.
 * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
 * when not needed.
 *
 * in/out - check API at xacl.h
 *
 * As a FreeBSD uses a different attributes name schema/format then this method is a very different
 * from a standard generic method because it uses a namespace (ns) value for os dependent optimization.
 */
bRC_XACL XACL_FreeBSD::os_get_xattr_value (JCR *jcr, int ns, char * name, char ** pvalue, uint32_t * plen){

   int len;
   POOLMEM * value;

   /* check input data */
   if (jcr == NULL || name == NULL || plen == NULL || pvalue == NULL){
      return bRC_XACL_inval;
   }
   /* get the length of the value for extended attribute */
   len = extattr_get_link(jcr->last_fname, ns, name, NULL, 0);
   switch (len){
      case -1: {
         berrno be;

         switch (errno){
            case ENOENT:
               /* no file available, skip it */
               return bRC_XACL_skip;
            default:
               /* XXX: what about ENOATTR error value? */
               Mmsg2(jcr->errmsg, _("extattr_get_link error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
               Dmsg2(100, "extattr_get_link error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
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
      len = extattr_get_link(jcr->last_fname, ns, name, value, len);
      switch (len){
      case -1: {
         berrno be;

         switch (errno){
         case ENOENT:
            /* no file available, skip it, first release allocated memory */
            free_pool_memory(value);
            return bRC_XACL_skip;
         default:
            Mmsg2(jcr->errmsg, _("extattr_get_link error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
            Dmsg2(100, "extattr_get_link error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
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
 *
 * xattr->name should be in '<namespace>.<name>' format which
 * function handle without problem, otherwise it returns an error
 * TODO: it is possible to handle a different attributes name format
 * for os portability where default namespace 'user' can be used
 */
bRC_XACL XACL_FreeBSD::os_set_xattr (JCR *jcr, XACL_xattr *xattr){

   char * name;
   char * nspace;
   int ns;
   int rc;

   /* check input data */
   if (jcr == NULL || xattr == NULL){
      return bRC_XACL_inval;
   }

   /* search for attribute namespace which is distinguished from attribute name by a dot '.' character */
   if ((name = strchr(xattr->name, '.')) == (char *)NULL){
      Mmsg2(jcr->errmsg, _("Failed to split %s into namespace and name part on file \"%s\"\n"), xattr->name, jcr->last_fname);
      Dmsg2(100, "Failed to split %s into namespace and name part on file \"%s\"\n", xattr->name, jcr->last_fname);
      return bRC_XACL_error;
   }

   /* split namespace and name of the attribute */
   nspace = xattr->name;
   *name++ = '\0';

   /* check if namespace is valid on this system */
   if (extattr_string_to_namespace(nspace, &ns) != 0){
      Mmsg2(jcr->errmsg, _("Failed to convert %s into namespace on file \"%s\"\n"), nspace, jcr->last_fname);
      Dmsg2(100, "Failed to convert %s into namespace on file \"%s\"\n", nspace, jcr->last_fname);
      return bRC_XACL_error;
   }

   /* set extattr on file */
   rc = extattr_set_link(jcr->last_fname, ns, name, xattr->value, xattr->value_len);
   if (rc < 0 || rc != (int)xattr->value_len){
      berrno be;

      switch (errno){
      case ENOENT:
         break;
      default:
         Mmsg2(jcr->errmsg, _("extattr_set_link error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
         Dmsg2(100, "extattr_set_link error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
         return bRC_XACL_error;
      }
   }
   return bRC_XACL_ok;
};

#endif /* HAVE_FREEBSD_OS */
