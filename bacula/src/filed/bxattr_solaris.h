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
 * Major refactoring of XATTR code written by:
 *
 *  Radosław Korzeniewski, MMXVI
 *  radoslaw@korzeniewski.net, radekk@inteos.pl
 *  Inteos Sp. z o.o. http://www.inteos.pl/
 *
 */

#ifndef __BXATTR_Solaris_H_
#define __BXATTR_Solaris_H_

#if defined(HAVE_SUN_OS)

/* check if XATTR support is enabled */
#if defined(HAVE_XATTR)

/*
 *
 */
#ifdef HAVE_SYS_ATTR_H
#include <sys/attr.h>
#endif

/*
 * Required for XATTR/ACL backup
 */
#ifdef HAVE_SYS_ACL_H
#include <sys/acl.h>
#endif

/*
 * Cache structure in alist
 */
struct BXATTR_Solaris_Cache {
   ino_t inode;
   char * name;
};

/*
 *
 *
 */
class BXATTR_Solaris : public BXATTR {
private:
   alist * cache;
   bRC_BXATTR os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt);
   bRC_BXATTR os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length);
   bRC_BXATTR os_get_xattr_names (JCR *jcr, POOLMEM **list, uint32_t *length);
   bRC_BXATTR os_get_xattr_value (JCR *jcr, char * name, char ** pvalue, uint32_t * plen);
   bRC_BXATTR os_set_xattr (JCR *jcr, bool extended, char *content, uint32_t length);
   bRC_BXATTR os_get_xattr_acl(JCR *jcr, int fd, char **buffer);
   bRC_BXATTR os_set_xattr_acl(JCR *jcr, int fd, char *name, char *acltext);
   inline char * find_xattr_cache(JCR *jcr, ino_t ino, char * name);
   inline void delete_xattr_cache();
public:
   BXATTR_Solaris ();
   ~BXATTR_Solaris ();
};

#endif /* HAVE_XATTR */

#endif /* HAVE_SUN_OS */

#endif /* __BXATTR_Solaris_H_ */
