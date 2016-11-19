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
 *
 * A specialized class to handle ACL and XATTR in Bacula Enterprise.
 * The runtime consist of two parts:
 * 1. OS independent class: XACL
 * 2. OS dependent subclass: XACL_*
 *
 * OS dependent subclasses are available for the following OS:
 *   - Darwin (OSX)
 *   - FreeBSD (POSIX and NFSv4/ZFS acls)
 *   - Linux
 *   - Solaris (POSIX and NFSv4/ZFS acls)
 *
 * OS dependend subclasses in progress:
 *   - AIX (pre-5.3 and post 5.3 acls, acl_get and aclx_get interface)
 *   - HPUX
 *   - IRIX
 *   - Tru64
 *
 * OS independent class support AFS acls using the pioctl interface.
 *
 * ACLs are saved in OS native text format provided by acl(3) API and uses
 * different streams for all different platforms.
 * XATTRs are saved in OS independent format (Bacula own) and uses different streams
 * for all different platforms. In theory it is possible to restore XATTRs from
 * particular OS on different OS platform. But this functionality is not available.
 * Above behavior is a backward compatibility with previous Bacula implementation
 * we need to maintain.
 *
 * During OS specyfic implementation of XACL you need to implement a following methods:
 *
 * [xacl] - indicates xacl function/method to call
 * [os] - indicates OS specyfic function, which could be different on specyfic OS
 *        (we use a Linux api calls as an example)
 *
 * ::os_get_acl(JCR *jcr, XACL_type xacltype)
 *
 *   1. get binary form of the acl - acl_get_file[os]
 *   2. check if acl is trivial if required - call acl_issimple[xacl]
 *   3. translate binary form into text representation - acl_to_text[os]
 *   4. save acl text into content - set_content[xacl]
 *   5. if acl not supported on filesystem - call clear_flag(XACL_FLAG_NATIVE)[xacl]
 *
 * ::os_backup_acl (JCR *jcr, FF_PKT *ff_pkt)
 *
 *   1. call os_get_acl[xacl] for all supported ACL_TYPES
 *   2. call send_acl_stream[xacl] for all supported ACL_STREAMS
 *
 * ::os_set_acl(JCR *jcr, XACL_type xacltype, char *content, uint32_t length)
 *
 *   1. prepare acl binary form from text representation stored in content - acl_from_text[os]
 *   2. set acl on file - acl_set_file[os]
 *   3. if acl not supported on filesystem, clear_flag(XACL_FLAG_NATIVE)
 *
 * ::os_restore_acl (JCR *jcr, int stream, char *content, uint32_t length)
 *
 *   1. call os_set_acl for all supported ACL_TYPES
 *
 * ::os_get_xattr_names (JCR *jcr, int namespace, POOLMEM ** pxlist, uint32_t * xlen)
 *
 *    1. get a size of the extended attibutes list for the file - llistxattr[os]
 *       in most os'es it is required to have a sufficient space for attibutes list
 *       and we wont allocate too much and too low space
 *    2. allocate the buffer of required space
 *    3. get an extended attibutes list for file - llistxattr[os]
 *    4. return allocated space buffer in pxlist and length of the buffer in xlen
 *
 * ::os_get_xattr_value (JCR *jcr, char * name, char ** pvalue, uint32_t * plen)
 *
 *    1. get a size of the extended attibute value for the file - lgetxattr[os]
 *       in most os'es it is required to have a sufficient space for attibute value
 *       and we wont allocate too much and too low space
 *    2. allocate the buffer of required space
 *    3. get an extended attibute value for file - lgetxattr[os]
 *    4. return allocated space buffer in pvalue and length of the buffer in plen
 *
 * ::os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt)
 *
 *    1. get a list of extended attributes (name and value) for a file; in most implementations
 *       it require to get a separate list of attributes names and separate values for every name,
 *       so it is:
 *       1A. get a list of xattr attribute names available on file - os_get_xattr_names[xacl]
 *       1B. for every attribute name get a value - os_get_xattr_value[xacl]
 *          You should skip some OS specyfic attributes like ACL attributes or NFS4; you can use
 *          check_xattr_skiplists[xacl] for this
 *       1C. build a list [type alist] of name/value pairs stored in XACL_xattr struct
 *    2. if the xattr list is not empty then serialize the list using serialize_xattr_stream[xacl]
 *    3. call send_xattr_stream[xacl]
 *
 * ::os_set_xattr (JCR *jcr, XACL_xattr *xattr)
 *
 *    1. set xattr on file using name/value in xattr - lsetxattr[os]
 *    2. if xattr not supported on filesystem - call clear_flag(XACL_FLAG_NATIVE)[xacl]
 *
 * ::os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length)
 *
 *    1. unserialize backup stream
 *    2. for every extended attribute restored call os_set_xattr[xacl] to set this attribute on file
 */

#include "bacula.h"
#include "filed.h"

/*
 * This is a constructor of the base XACL class which is OS independent
 *
 * - for initialization it uses ::init()
 *
 */
XACL::XACL (){
   init();
};

/*
 * This is a destructor of the XACL class
 */
XACL::~XACL (){
   free_pool_memory(content);
};

/*
 * Initialization routine
 * - initializes all variables to required status
 * - allocates required memory
 */
void XACL::init(){
#if defined(HAVE_ACL)
   acl_ena = TRUE;
#else
   acl_ena = FALSE;
#endif

#if defined(HAVE_XATTR)
   xattr_ena = TRUE;
#else
   xattr_ena = FALSE;
#endif

   /* generic variables */
   flags = XACL_FLAG_NONE;
   current_dev = 0;
   content = get_pool_memory(PM_BSOCK);   /* it is better to have a 4k buffer */
   content_len = 0;
   acl_nr_errors = 0;
   xattr_nr_errors = 0;
   acl_streams = NULL;
   default_acl_streams = NULL;
   xattr_streams = NULL;
   xattr_skiplist = NULL;
   xattr_acl_skiplist = NULL;
};

/*
 * Enables ACL handling in runtime, could be disabled with disable_acl
 *    when ACL is not configured then cannot change status
 */
void XACL::enable_acl(){
#if defined(HAVE_ACL)
   acl_ena = TRUE;
#endif
};

/*
 * Disables ACL handling in runtime, could be enabled with enable_acl
 *    when ACL is configured
 */
void XACL::disable_acl(){
   acl_ena = FALSE;
};

/*
 * Enables XATTR handling in runtime, could be disabled with disable_xattr
 *    when XATTR is not configured then cannot change status
 */
void XACL::enable_xattr(){
#ifdef HAVE_XATTR
   xattr_ena = TRUE;
#endif
};

/*
 * Disables XATTR handling in runtime, could be enabled with enable_xattr
 *    when XATTR is configured
 */
void XACL::disable_xattr(){
   xattr_ena = FALSE;
};

/*
 * Copies a text into a content variable and sets a constent_len respectively
 *
 * in:
 *    text - a standard null terminated string
 * out:
 *    pointer to content variable to use externally
 */
POOLMEM * XACL::set_content(char *text){
   content_len = pm_strcpy(&content, text);
   return content;
};

/*
 * Copies a data with length of len into a content variable
 *
 * in:
 *    data - data pointer to copy into content buffer
 * out:
 *    pointer to content variable to use externally
 */
POOLMEM * XACL::set_content(char *data, int len){
   content_len = pm_memcpy(&content, data, len);
   return content;
};

/*
 * Check if we changed the device,
 * if so setup a flags
 *
 * in:
 *    jcr - Job Control Record
 * out:
 *    bRC_XACL_ok - change of device checked and finish succesful
 *    bRC_XACL_error - encountered error
 *    bRC_XACL_skip - cannot verify device - no file found
 *    bRC_XACL_inval - invalid input data
 */
bRC_XACL XACL::check_dev (JCR *jcr){

   int lst;
   struct stat st;

   /* sanity check of input variables */
   if (jcr == NULL || jcr->last_fname == NULL){
      return bRC_XACL_inval;
   }

   lst = lstat(jcr->last_fname, &st);
   switch (lst){
      case -1: {
         berrno be;
         switch (errno){
         case ENOENT:
            return bRC_XACL_skip;
         default:
            Mmsg2(jcr->errmsg, _("Unable to stat file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
            Dmsg2(100, "Unable to stat file \"%s\": ERR=%s\n", jcr->last_fname, be.bstrerror());
            return bRC_XACL_error;
         }
         break;
      }
      case 0:
         break;
   }

   check_dev(jcr, st.st_dev);

   return bRC_XACL_ok;
};

/*
 * Check if we changed the device, if so setup a flags
 *
 * in:
 *    jcr - Job Control Record
 * out:
 *    internal flags status set
 */
void XACL::check_dev (JCR *jcr, uint32_t dev){

   /* sanity check of input variables */
   if (jcr == NULL || jcr->last_fname == NULL){
      return;
   }

   if (current_dev != dev){
      flags = XACL_FLAG_NONE;
#if defined(HAVE_AFS_ACL)
      /* handle special fs: AFS */
      if (fstype_equals(jcr->last_fname, "afs")){
         set_flag(XACL_FLAG_AFS);
      } else {
         set_flag(XACL_FLAG_NATIVE);
      }
#else
      set_flag(XACL_FLAG_NATIVE);
#endif
      current_dev = dev;
   }
};

/*
 * It sends a stream located in this->content to Storage Daemon, so the main Bacula
 * backup loop is free from this. It sends a header followed by data.
 *
 * in:
 *    jcr - Job Control Record
 *    stream - a stream number to save
 * out:
 *    bRC_XACL_inval - when supplied variables are incorrect
 *    bRC_XACL_fatal - when we can't send data to the SD
 *    bRC_XACL_ok - send finish without errors
 */
bRC_XACL XACL::send_acl_stream(JCR *jcr, int stream){

   BSOCK * sd;
   POOLMEM * msgsave;
#ifdef FD_NO_SEND_TEST
   return bRC_XACL_ok;
#endif

   /* sanity check of input variables */
   if (jcr == NULL || jcr->store_bsock == NULL){
      return bRC_XACL_inval;
   }
   if (content_len <= 0){
      return bRC_XACL_ok;
   }

   sd = jcr->store_bsock;
   /* send header */
   if (!sd->fsend("%ld %d 0", jcr->JobFiles, stream)){
      Jmsg1(jcr, M_FATAL, 0, _("Network send error to SD. ERR=%s\n"), sd->bstrerror());
      return bRC_XACL_fatal;
   }

   /* send the buffer to the storage deamon */
   Dmsg1(400, "Backing up ACL <%s>\n", content);
   msgsave = sd->msg;
   sd->msg = content;
   sd->msglen = content_len + 1;
   if (!sd->send()){
      sd->msg = msgsave;
      sd->msglen = 0;
      Jmsg1(jcr, M_FATAL, 0, _("Network send error to SD. ERR=%s\n"), sd->bstrerror());
      return bRC_XACL_fatal;
   }

   jcr->JobBytes += sd->msglen;
   sd->msg = msgsave;
   if (!sd->signal(BNET_EOD)){
      Jmsg1(jcr, M_FATAL, 0, _("Network send error to SD. ERR=%s\n"), sd->bstrerror());
      return bRC_XACL_fatal;
   }

   Dmsg1(200, "ACL of file: %s successfully backed up!\n", jcr->last_fname);
   return bRC_XACL_ok;
};

/*
 * It sends a stream located in this->content to Storage Daemon, so the main Bacula
 * backup loop is free from this. It sends a header followed by data.
 *
 * in:
 *    jcr - Job Control Record
 *    stream - a stream number to save
 * out:
 *    bRC_XACL_inval - when supplied variables are incorrect
 *    bRC_XACL_fatal - when we can't send data to the SD
 *    bRC_XACL_ok - send finish without errors
 */
bRC_XACL XACL::send_xattr_stream(JCR *jcr, int stream){

   BSOCK * sd;
   POOLMEM * msgsave;
#ifdef FD_NO_SEND_TEST
   return bRC_XACL_ok;
#endif

   /* sanity check of input variables */
   if (jcr == NULL || jcr->store_bsock == NULL){
      return bRC_XACL_inval;
   }
   if (content_len <= 0){
      return bRC_XACL_ok;
   }

   sd = jcr->store_bsock;
   /* send header */
   if (!sd->fsend("%ld %d 0", jcr->JobFiles, stream)){
      Jmsg1(jcr, M_FATAL, 0, _("Network send error to SD. ERR=%s\n"), sd->bstrerror());
      return bRC_XACL_fatal;
   }

   /* send the buffer to the storage deamon */
   Dmsg1(400, "Backing up XATTR <%s>\n", content);
   msgsave = sd->msg;
   sd->msg = content;
   sd->msglen = content_len;
   if (!sd->send()){
      sd->msg = msgsave;
      sd->msglen = 0;
      Jmsg1(jcr, M_FATAL, 0, _("Network send error to SD. ERR=%s\n"), sd->bstrerror());
      return bRC_XACL_fatal;
   }

   jcr->JobBytes += sd->msglen;
   sd->msg = msgsave;
   if (!sd->signal(BNET_EOD)){
      Jmsg1(jcr, M_FATAL, 0, _("Network send error to SD. ERR=%s\n"), sd->bstrerror());
      return bRC_XACL_fatal;
   }
   Dmsg1(200, "XATTR of file: %s successfully backed up!\n", jcr->last_fname);
   return bRC_XACL_ok;
};

/*
 * The main public backup method for ACL
 *
 * in:
 *    jcr - Job Control Record
 *    ff_pkt - file backup record
 * out:
 *    bRC_XACL_fatal - when ACL backup is not compiled in Bacula
 *    bRC_XACL_ok - backup finish without problems
 *    bRC_XACL_error - when you can't backup acl data because some error
 */
bRC_XACL XACL::backup_acl (JCR *jcr, FF_PKT *ff_pkt){

#if !defined(HAVE_ACL) && !defined(HAVE_AFS_ACL)
   Jmsg(jcr, M_FATAL, 0, "ACL backup requested but not configured in Bacula.\n");
   return bRC_XACL_fatal;
#else
   /* sanity check of input variables and verify if engine is enabled */
   if (acl_ena && jcr != NULL && ff_pkt != NULL){
      /* acl engine enabled, proceed */
      bRC_XACL rc;
      /*
       * No acl request for link or plugin
       *
       * TODO: it should be possible to handle ACL/XATTR for cmd plugins
       */
      if (!(ff_pkt->flags & FO_ACL && ff_pkt->type != FT_LNK && !ff_pkt->cmd_plugin)){
         return bRC_XACL_ok;
      }

      jcr->errmsg[0] = 0;
      check_dev(jcr, ff_pkt->statp.st_dev);

#if defined(HAVE_AFS_ACL)
      if (flags & XACL_FLAG_AFS){
         Dmsg0(400, "make AFS ACL call\n");
         rc = afs_backup_acl(jcr, ff_pkt);
         goto bail_out;
      }
#endif

#if defined(HAVE_ACL)
      if (flags & XACL_FLAG_NATIVE){
         Dmsg0(400, "make Native ACL call\n");
         rc = os_backup_acl(jcr, ff_pkt);
      } else {
         /* skip acl backup */
         return bRC_XACL_ok;
      }
#endif

#if defined(HAVE_AFS_ACL)
   bail_out:
#endif
      if (rc == bRC_XACL_error){
         if (acl_nr_errors < ACL_MAX_ERROR_PRINT_PER_JOB){
            if (!jcr->errmsg[0]){
               Jmsg(jcr, M_WARNING, 0, "No OS ACL configured.\n");
            } else {
               Jmsg(jcr, M_WARNING, 0, "%s", jcr->errmsg);
            }
            inc_acl_errors();
         }
         return bRC_XACL_ok;
      }
      return rc;
   }
   return bRC_XACL_ok;
#endif
};

/*
 * The main public restore method for ACL
 *
 * in:
 *    jcr - Job Control Record
 *    stream - a backup stream type number to restore_acl
 *    data - a potinter to the data stream to restore
 *    length - a data stream length
 * out:
 *    bRC_XACL_fatal - when ACL restore is not compiled in Bacula
 *    bRC_XACL_ok - restore finish without problems
 *    bRC_XACL_error - when you can't restore a stream because some error
 */
bRC_XACL XACL::restore_acl (JCR *jcr, int stream, char *data, uint32_t length){

#if !defined(HAVE_ACL) && !defined(HAVE_AFS_ACL)
   Jmsg(jcr, M_FATAL, 0, "ACL retore requested but not configured in Bacula.\n");
   return bRC_XACL_fatal;
#else
   /* sanity check of input variables and verify if engine is enabled */
   if (acl_ena && jcr != NULL && data != NULL){
      /* acl engine enabled, proceed */
      int a;
      bRC_XACL rc = check_dev(jcr);

      switch (rc){
         case bRC_XACL_skip:
            return bRC_XACL_ok;
         case bRC_XACL_ok:
            break;
         default:
            return rc;
      }

      /* copy a data into a content buffer */
      set_content(data, length);

      switch (stream){
#if defined(HAVE_AFS_ACL)
         case STREAM_XACL_AFS_TEXT:
            if (flags & XACL_FLAG_AFS){
               return afs_restore_acl(jcr, stream);
            } else {
               /*
                * Increment error count but don't log an error again for the same filesystem.
                */
               inc_acl_errors();
               return bRC_XACL_ok;
            }
#endif
#if defined(HAVE_ACL)
         case STREAM_UNIX_ACCESS_ACL:
         case STREAM_UNIX_DEFAULT_ACL:
            if (flags & XACL_FLAG_NATIVE){
               return os_restore_acl(jcr, stream, content, content_len);
            } else {
               inc_acl_errors();
               return bRC_XACL_ok;
            }
            break;
         default:
            if (flags & XACL_FLAG_NATIVE){
               for (a = 0; acl_streams[a] > 0; a++){
                  if (acl_streams[a] == stream){
                     return os_restore_acl(jcr, stream, content, content_len);
                  }
               }
               for (a = 0; default_acl_streams[a] > 0; a++){
                  if (default_acl_streams[a] == stream){
                     return os_restore_acl(jcr, stream, content, content_len);
                  }
               }
            } else {
               inc_acl_errors();
               return bRC_XACL_ok;
            }
            break;
#else
         default:
            break;
#endif
      }
      /* cannot find a valid stream to support */
      Qmsg2(jcr, M_WARNING, 0, _("Can't restore ACLs of %s - incompatible acl stream encountered - %d\n"), jcr->last_fname, stream);
      return bRC_XACL_error;
   }
   return bRC_XACL_ok;
#endif
};

/*
 * The main public backup method for XATTR
 *
 * in:
 *    jcr - Job Control Record
 *    ff_pkt - file backup record
 * out:
 *    bRC_XACL_fatal - when XATTR backup is not compiled in Bacula
 *    bRC_XACL_ok - backup finish without problems
 *    bRC_XACL_error - when you can't backup xattr data because some error
 */
bRC_XACL XACL::backup_xattr (JCR *jcr, FF_PKT *ff_pkt){

#if !defined(HAVE_XATTR)
   Jmsg(jcr, M_FATAL, 0, "XATTR backup requested but not configured in Bacula.\n");
   return bRC_XACL_fatal;
#else
   /* sanity check of input variables and verify if engine is enabled */
   if (xattr_ena && jcr != NULL && ff_pkt != NULL){
      /* xattr engine enabled, proceed */
      bRC_XACL rc;
      /*
       * No xattr request for plugin
       *
       * TODO: it should be possible to handle ACL/XATTR for cmd plugins
       */
      if (!(ff_pkt->flags & FO_XATTR && !ff_pkt->cmd_plugin)){
         return bRC_XACL_ok;
      }

      jcr->errmsg[0] = 0;
      check_dev(jcr, ff_pkt->statp.st_dev);

      if (flags & XACL_FLAG_NATIVE){
         Dmsg0(400, "make Native XATTR call\n");
         rc = os_backup_xattr(jcr, ff_pkt);
      } else {
         /* skip xattr backup */
         return bRC_XACL_ok;
      }

      if (rc == bRC_XACL_error){
         if (xattr_nr_errors < XATTR_MAX_ERROR_PRINT_PER_JOB){
            if (!jcr->errmsg[0]){
               Jmsg(jcr, M_WARNING, 0, "No OS XATTR configured.\n");
            } else {
               Jmsg(jcr, M_WARNING, 0, "%s", jcr->errmsg);
            }
            inc_xattr_errors();
         }
         return bRC_XACL_ok;
      }
      return rc;
   }
   return bRC_XACL_ok;
#endif
};

/*
 * The main public restore method for XATTR
 *
 * in:
 *    jcr - Job Control Record
 *    stream - a backup stream type number to restore_acl
 *    data - a potinter to the data stream to restore
 *    length - a data stream length
 * out:
 *    bRC_XACL_fatal - when XATTR restore is not compiled in Bacula
 *    bRC_XACL_ok - restore finish without problems
 *    bRC_XACL_error - when you can't restore a stream because some error
 */
bRC_XACL XACL::restore_xattr (JCR *jcr, int stream, char *data, uint32_t length){

#if !defined(HAVE_XATTR)
   Jmsg(jcr, M_FATAL, 0, "XATTR retore requested but not configured in Bacula.\n");
   return bRC_XACL_fatal;
#else
   /* sanity check of input variables and verify if engine is enabled */
   if (xattr_ena && jcr != NULL && data != NULL){
      /* xattr engine enabled, proceed */
      int a;
      bRC_XACL rc = check_dev(jcr);

      switch (rc){
         case bRC_XACL_skip:
            return bRC_XACL_ok;
         case bRC_XACL_ok:
            break;
         default:
            return rc;
      }

      /* copy a data into a content buffer */
      set_content(data, length);

      if (flags & XACL_FLAG_NATIVE){
         for (a = 0; xattr_streams[a] > 0; a++){
            if (xattr_streams[a] == stream){
               return os_restore_xattr(jcr, stream, content, content_len);
            }
         }
      } else {
         inc_xattr_errors();
         return bRC_XACL_ok;
      }
      /* cannot find a valid stream to support */
      Qmsg2(jcr, M_WARNING, 0, _("Can't restore Extended Attributes of %s - incompatible xattr stream encountered - %d\n"), jcr->last_fname, stream);
      return bRC_XACL_error;
   }
   return bRC_XACL_ok;
#endif
};

/*
 * Performs a generic ACL backup using OS specyfic methods for
 * getting acl data from file
 *
 * in:
 *    jcr - Job Control Record
 *    ff_pkt - file to backup control package
 * out:
 *    bRC_XACL_ok - backup of acl's was successful
 *    bRC_XACL_fatal - was an error during acl backup
 */
bRC_XACL XACL::generic_backup_acl (JCR *jcr, FF_PKT *ff_pkt){

   /* sanity check of input variables */
   if (jcr == NULL || ff_pkt == NULL){
      return bRC_XACL_inval;
   }

   if (os_get_acl(jcr, XACL_TYPE_ACCESS) == bRC_XACL_fatal){
      /* XXX: check if os_get_acl return fatal and decide what to do when error is returned */
      return bRC_XACL_fatal;
   }

   if (content_len > 0){
      if (send_acl_stream(jcr, acl_streams[0]) == bRC_XACL_fatal){
         return bRC_XACL_fatal;
      }
   }

   if (ff_pkt->type == FT_DIREND){
      if (os_get_acl(jcr, XACL_TYPE_DEFAULT) == bRC_XACL_fatal){
         return bRC_XACL_fatal;
      }
      if (content_len > 0){
         if (send_acl_stream(jcr, default_acl_streams[0]) == bRC_XACL_fatal){
            return bRC_XACL_fatal;
         }
      }
   }
   return bRC_XACL_ok;
};

/*
 * Performs a generic ACL restore using OS specyfic methods for
 * setting acl data on file.
 *
 * in:
 *    jcr - Job Control Record
 *    stream - a stream number to restore
 * out:
 *    bRC_XACL_ok - backup of acl's was successful
 *    bRC_XACL_error - was an error during acl backup
 *    bRC_XACL_fatal - was a fatal error during acl backup or input data is invalid
 */
bRC_XACL XACL::generic_restore_acl (JCR *jcr, int stream){

   unsigned int count;

   /* sanity check of input variables */
   if (jcr == NULL){
      return bRC_XACL_inval;
   }

   switch (stream){
      case STREAM_UNIX_ACCESS_ACL:
         return os_set_acl(jcr, XACL_TYPE_ACCESS, content, content_len);
      case STREAM_UNIX_DEFAULT_ACL:
         return os_set_acl(jcr, XACL_TYPE_DEFAULT, content, content_len);
      default:
         for (count = 0; acl_streams[count] > 0; count++){
            if (acl_streams[count] == stream){
               return os_set_acl(jcr, XACL_TYPE_ACCESS, content, content_len);
            }
         }
         for (count = 0; default_acl_streams[count] > 0; count++){
            if (default_acl_streams[count] == stream){
               return os_set_acl(jcr, XACL_TYPE_DEFAULT, content, content_len);
            }
         }
         break;
   }
   return bRC_XACL_error;
};

/*
 * Checks if supplied xattr attribute name is indicated on OS specyfic lists
 *
 * in:
 *    jcr - Job Control Record
 *    ff_pkt - file to backup control package
 *    name - a name of the attribute to check
 * out:
 *    TRUE - the attribute name is found on OS specyfic skip lists and should be skipped during backup
 *    FALSE - the attribute should be saved on backup stream
 */
bool XACL::check_xattr_skiplists (JCR *jcr, FF_PKT *ff_pkt, char * name){

   bool skip = FALSE;
   int count;

   /* sanity check of input variables */
   if (jcr == NULL || ff_pkt == NULL || name ==  NULL){
      return false;
   }

   /*
    * On some OSes you also get the acls in the extented attribute list.
    * So we check if we are already backing up acls and if we do we
    * don't store the extended attribute with the same info.
    */
   if (ff_pkt->flags & FO_ACL){
      for (count = 0; xattr_acl_skiplist[count] != NULL; count++){
         if (bstrcmp(name, xattr_acl_skiplist[count])){
            skip = true;
            break;
         }
      }
   }
   /* on some OSes we want to skip certain xattrs which are in the xattr_skiplist array. */
   if (!skip){
      for (count = 0; xattr_skiplist[count] != NULL; count++){
         if (bstrcmp(name, xattr_skiplist[count])){
            skip = true;
            break;
         }
      }
   }

   return skip;
};


/*
 * Performs generic XATTR backup using OS specyfic methods for
 * getting xattr data from files - os_get_xattr_names and os_get_xattr_value
 *
 * in:
 *    jcr - Job Control Record
 *    ff_pkt - file to backup control package
 * out:
 *    bRC_XACL_ok - xattr backup ok or no xattr to backup found
 *    bRC_XACL_error/fatal - an error or fatal error occurred
 *    bRC_XACL_inval - input variables was invalid
 */
bRC_XACL XACL::generic_backup_xattr (JCR *jcr, FF_PKT *ff_pkt){

   bRC_XACL rc;
   POOLMEM *xlist;
   uint32_t xlen;
   char *name;
   uint32_t name_len;
   POOLMEM *value;
   uint32_t value_len;
   bool skip;
   alist *xattr_list = NULL;
   int xattr_count = 0;
   uint32_t len = 0;
   XACL_xattr *xattr;

   /* sanity check of input variables */
   if (jcr == NULL || ff_pkt == NULL){
      return bRC_XACL_inval;
   }

   /* xlist is allocated as POOLMEM by os_get_xattr_names */
   rc = os_get_xattr_names(jcr, &xlist, &xlen);
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

   /* follow the list of xattr names and get the values
    * TODO: change a standard NULL-terminated list of names into alist of structures */
   for (name = xlist; (name - xlist) + 1 < xlen; name = strchr(name, '\0') + 1){

      name_len = strlen(name);
      skip =  check_xattr_skiplists(jcr, ff_pkt, name);
      if (skip || name_len == 0){
         Dmsg1(100, "Skipping xattr named \"%s\"\n", name);
         continue;
      }

      /* value is allocated as POOLMEM by os_get_xattr_value */
      rc = os_get_xattr_value(jcr, name, &value, &value_len);
      switch (rc){
         case bRC_XACL_ok:
            /* it's ok, so go further */
            break;
         case bRC_XACL_skip:
            /* no xattr available, so skip rest of it */
            free_pool_memory(xlist);
            return bRC_XACL_ok;
         default:
            /* error / fatal */
            free_pool_memory(xlist);
            return rc;
      }

      /*
       * we have a name of the extended attribute in the name variable
       * and value of the extended attribute in the value variable
       * so we need to build a list
       */
      xattr = (XACL_xattr*)malloc(sizeof(XACL_xattr));
      xattr->name_len = name_len;
      xattr->name = name;
      xattr->value_len = value_len;
      xattr->value = value;
      /*       magic              name_len          name        value_len       value */
      len += sizeof(uint32_t) + sizeof(uint32_t) + name_len + sizeof(uint32_t) + value_len;

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
         goto bailout;
      } else {
         /* send data to SD */
         rc = send_xattr_stream(jcr, xattr_streams[0]);
      }
   } else {
      rc = bRC_XACL_ok;
   }

bailout:
   /* free allocated data */
   if (xattr_list != NULL){
      foreach_alist(xattr, xattr_list){
         if (xattr == NULL){
            break;
         }
         if (xattr->value){
            free_pool_memory(xattr->value);
         }
         free(xattr);
      }
      delete xattr_list;
   }
   if (xlist != NULL){
      free_pool_memory(xlist);
   }

   return rc;
};

/*
 * Performs a generic XATTR restore using OS specyfic methods for
 * setting XATTR data on file.
 *
 * in:
 *    jcr - Job Control Record
 *    stream - a stream number to restore
 * out:
 *    bRC_XACL_ok - restore of acl's was successful
 *    bRC_XACL_error - was an error during xattr restore
 *    bRC_XACL_fatal - was a fatal error during xattr restore
 *    bRC_XACL_inval - input variables was invalid
 */
bRC_XACL XACL::generic_restore_xattr (JCR *jcr, int stream){

   bRC_XACL rc = bRC_XACL_ok;
   alist *xattr_list;
   XACL_xattr *xattr;

   /* sanity check of input variables */
   if (jcr == NULL){
      return bRC_XACL_inval;
   }

   /* empty list */
   xattr_list = New(alist(10, not_owned_by_alist));

   /* unserialize data */
   unserialize_xattr_stream(jcr, content, content_len, xattr_list);

   /* follow the list to set all attributes */
   foreach_alist(xattr, xattr_list){
      rc = os_set_xattr(jcr, xattr);
      if (rc != bRC_XACL_ok){
         Dmsg2(100, "Failed to set extended attribute %s on file \"%s\"\n", xattr->name, jcr->last_fname);
         goto bailout;
      }
   }

bailout:
   /* free allocated data */
   if (xattr_list != NULL){
      foreach_alist(xattr, xattr_list){
         if (xattr == NULL){
            break;
         }
         if (xattr->name){
            free(xattr->name);
         }
         if (xattr->value){
            free(xattr->value);
         }
         free(xattr);
      }
      delete xattr_list;
   }
   return rc;
};

/*
 * Initialize variables acl_streams and default_acl_streams for a specified OS.
 * The rutine should be called from object instance constructor
 *
 * in:
 *    pacl - acl streams supported for specyfic OS
 *    pacl_def - default (directory) acl streams supported for specyfic OS
 */
void XACL::set_acl_streams (const int *pacl, const int *pacl_def){

   acl_streams = pacl;
   default_acl_streams = pacl_def;
};

/*
 * Initialize a variable xattr_streams for a specified OS.
 * The rutine should be called from object instance constructor
 *
 * in:
 *    pxattr - xattr streams supported for specyfic OS
 */
void XACL::set_xattr_streams (const int *pxattr){

   xattr_streams = pxattr;
};

/*
 * Initialize variables xattr_skiplist and xattr_acl_skiplist for a specified OS.
 * The rutine should be called from object instance constructor
 *
 * in:
 *    pxattr - xattr skip list for specyfic OS
 *    pxattr_acl - xattr acl names skip list for specyfic OS
 */
void XACL::set_xattr_skiplists (const char **pxattr, const char **pxattr_acl){

   xattr_skiplist = pxattr;
   xattr_acl_skiplist = pxattr_acl;
};

/*
 * Serialize the XATTR stream which will be saved into archive. Serialization elements cames from
 * a list and for backward compatibility we produce the same stream as prievous Bacula versions.
 *
 * serialized stream consists of the following elements:
 *    magic - A magic string which makes it easy to detect any binary incompatabilites
 *             required for backward compatibility
 *    name_len - The length of the following xattr name
 *    name - The name of the extended attribute
 *    value_len - The length of the following xattr data
 *    value - The actual content of the extended attribute only if value_len is greater then zero
 *
 * in:
 *    jcr - Job Control Record
 *    len - expected serialize length
 *    list - a list of xattr elements to serialize
 * out:
 *    bRC_XACL_ok - when serialization was perfect
 *    bRC_XACL_inval - when we have invalid variables
 *    bRC_XACL_error - illegal attribute name
 */
bRC_XACL XACL::serialize_xattr_stream(JCR *jcr, uint32_t len, alist *list){

   ser_declare;
   XACL_xattr *xattr;

   /* sanity check of input variables */
   if (jcr == NULL || list == NULL){
      return bRC_XACL_inval;
   }

   /* we serialize data direct to content buffer, so check if data fits */
   content = check_pool_memory_size(content, len + 20);
   ser_begin(content, len + 20);

   foreach_alist(xattr, list){
      if (xattr == NULL){
         break;
      }
      /*
       * serialize data
       *
       * we have to start with the XATTR_MAGIC for backward compatibility (the magic is silly)
       */
      ser_uint32(XATTR_MAGIC);
      /* attribute name length and name itself */
      if (xattr->name_len > 0 && xattr->name){
         ser_uint32(xattr->name_len);
         ser_bytes(xattr->name, xattr->name_len);
      } else {
         /* error - name cannot be empty */
         Mmsg0(jcr->errmsg, _("Illegal empty xattr attribute name\n"));
         Dmsg0(100, "Illegal empty xattr attribute name\n");
         return bRC_XACL_error;
      }
      /* attibute value length and value itself */
      ser_uint32(xattr->value_len);
      if (xattr->value_len > 0 && xattr->value){
         ser_bytes(xattr->value, xattr->value_len);
         Dmsg3(100, "Backup xattr named %s, value %*.s\n", xattr->name, xattr->value_len, xattr->value);
      } else {
         Dmsg1(100, "Backup empty xattr named %s\n", xattr->name);
      }
   }

   ser_end(content, len + 20);
   content_len = ser_length(content);

   return bRC_XACL_ok;
};

/*
 * Unserialize XATTR stream on *content and produce a xattr *list which contain
 * key => value pairs
 *
 * in:
 *    jcr - Job Control Record
 *    content - a stream content to unserialize
 *    length - a content length
 *    list - a pointer to the xattr list to populate
 * out:
 *    bRC_XACL_ok - when unserialize was perfect
 *    bRC_XACL_inval - when we have invalid variables
 *    list - key/value pairs populated xattr list
 */
bRC_XACL XACL::unserialize_xattr_stream(JCR *jcr, char *content, uint32_t length, alist *list){

   unser_declare;
   uint32_t magic;
   XACL_xattr *xattr;

   /* sanity check of input variables */
   if (jcr == NULL || content == NULL || list == NULL){
      return bRC_XACL_inval;
   }

   unser_begin(content, length);
   while (unser_length(content) < length){
      /*
       * Sanity check of correct stream magic number
       * Someone was too paranoid to implement this kind of verification in original Bacula code
       * Unfortunate for backward compatibility we have to follow this insane implementation
       *
       * XXX: design a new xattr stream format
       */
      unser_uint32(magic);
      if (magic != XATTR_MAGIC){
         Mmsg(jcr->errmsg, _("Illegal xattr stream, no XATTR_MAGIC on file \"%s\"\n"), jcr->last_fname);
         Dmsg1(100, "Illegal xattr stream, no XATTR_MAGIC on file \"%s\"\n", jcr->last_fname);
         return bRC_XACL_error;
      }
      /* first attribute name length */
      xattr = (XACL_xattr *)malloc(sizeof(XACL_xattr));
      unser_uint32(xattr->name_len);
      if (xattr->name_len == 0){
         /* attribute name cannot be empty */
         Mmsg(jcr->errmsg, _("Illegal xattr stream, xattr name length <= 0 on file \"%s\"\n"), jcr->last_fname);
         Dmsg1(100, "Illegal xattr stream, xattr name length <= 0 on file \"%s\"\n", jcr->last_fname);
         free(xattr);
         return bRC_XACL_error;
      }
      /* followed by attribute name itself */
      xattr->name = (char *)malloc(xattr->name_len + 1);
      unser_bytes(xattr->name, xattr->name_len);
      xattr->name[xattr->name_len] = '\0';
      /* attribute value */
      unser_uint32(xattr->value_len);
      if (xattr->value_len > 0){
         /* we have a value */
         xattr->value = (char *)malloc(xattr->value_len + 1);
         unser_bytes(xattr->value, xattr->value_len);
         xattr->value[xattr->value_len] = '\0';
         Dmsg3(100, "Restoring xattr named %s, value %.*s\n", xattr->name, xattr->value_len, xattr->value);
      } else {
         /* value is empty */
         xattr->value = NULL;
         Dmsg1(100, "Restoring empty xattr named %s\n", xattr->name);
      }
      list->append(xattr);
   }
   unser_end(content, length);

   return bRC_XACL_ok;
};

#if defined(HAVE_AFS_ACL)

#if defined(HAVE_AFS_AFSINT_H) && defined(HAVE_AFS_VENUS_H)
#include <afs/afsint.h>
#include <afs/venus.h>
#else
#error "configure failed to detect availability of afs/afsint.h and/or afs/venus.h"
#endif

/*
 * External references to functions in the libsys library function not in current include files.
 */
extern "C" {
long pioctl(char *pathp, long opcode, struct ViceIoctl *blobp, int follow);
}

/*
 * Backup ACL data of AFS
 *
 * in:
 *    jcr - Job Control Record
 *    ff_pkt - file backup record
 * out:
 *    bRC_XACL_inval - input variables are invalid (NULL)
 *    bRC_XACL_ok - backup finish without problems
 *    bRC_XACL_error - when you can't backup acl data because some error
 */
bRC_XACL XACL::afs_backup_acl (JCR *jcr, FF_PKT *ff_pkt){

   int rc;
   struct ViceIoctl vip;
   char data[BUFSIZ];

   /* sanity check of input variables */
   if (jcr == NULL || ff_pkt == NULL){
      return bRC_XACL_inval;
   }

   /* AFS ACLs can only be set on a directory, so no need to try other files */
   if (ff_pkt->type != FT_DIREND){
      return bRC_XACL_ok;
   }

   vip.in = NULL;
   vip.in_size = 0;
   vip.out = data;
   vip.out_size = BUFSIZE;
   memset(data, 0, BUFSIZE);

   if ((rc = pioctl(jcr->last_fname, VIOCGETAL, &vip, 0)) < 0){
      berrno be;

      Mmsg2(jcr->errmsg, _("pioctl VIOCGETAL error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
      Dmsg2(100, "pioctl VIOCGETAL error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());
      return bRC_XACL_error;
   }
   set_content(data);
   return send_acl_stream(jcr, STREAM_XACL_AFS_TEXT);
};

/*
 * Restore ACL data of AFS
 * in:
 *    jcr - Job Control Record
 *    stream - a backup stream type number to restore_acl
 * out:
 *    bRC_XACL_inval - input variables are invalid (NULL)
 *    bRC_XACL_ok - backup finish without problems
 *    bRC_XACL_error - when you can't backup acl data because some error
 */
bRC_XACL XACL::afs_restore_acl (JCR *jcr, int stream){

   int rc;
   struct ViceIoctl vip;

   /* sanity check of input variables */
   if (jcr == NULL || ff_pkt == NULL){
      return bRC_XACL_inval;
   }

   vip.in = content;
   vip.in_size = content_len;
   vip.out = NULL;
   vip.out_size = 0;

   if ((rc = pioctl(jcr->last_fname, VIOCSETAL, &vip, 0)) < 0){
      berrno be;

      Mmsg2(jcr->errmsg, _("pioctl VIOCSETAL error on file \"%s\": ERR=%s\n"), jcr->last_fname, be.bstrerror());
      Dmsg2(100, "pioctl VIOCSETAL error file=%s ERR=%s\n", jcr->last_fname, be.bstrerror());

      return bRC_XACL_error;
   }
   return bRC_XACL_ok;
};
#endif /* HAVE_AFS_ACL */

#include "xacl_osx.h"
#include "xacl_linux.h"
#include "xacl_freebsd.h"
#include "xacl_solaris.h"
// #include "xacl_aix.h"

/*
 * Creating the corrent instance of the XACL for a supported OS
 */
void *new_xacl(){
#if   defined(HAVE_DARWIN_OS)
   return new XACL_OSX();
#elif defined(HAVE_LINUX_OS)
   return new XACL_Linux();
#elif defined(HAVE_FREEBSD_OS)
   return new XACL_FreeBSD();
#elif defined(HAVE_HURD_OS)
   return new XACL_Hurd();
#elif defined(HAVE_AIX_OS)
   return new XACL_AIX();
#elif defined(HAVE_IRIX_OS)
   return new XACL_IRIX();
#elif defined(HAVE_OSF1_OS)
   return new XACL_OSF1();
#elif defined(HAVE_SUN_OS)
   return new XACL_Solaris();
#endif
};
