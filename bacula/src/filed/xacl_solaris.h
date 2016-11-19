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

#ifndef __XACL_Solaris_H_
#define __XACL_Solaris_H_

#if defined(HAVE_SUN_OS)
#ifdef HAVE_SYS_ACL_H
#include <sys/acl.h>
#else
#error "configure failed to detect availability of sys/acl.h"
#endif

#ifdef HAVE_SYS_ATTR_H
#include <sys/attr.h>
#endif

/*
 *
 */
#if defined(HAVE_EXTENDED_ACL)
#if !defined(_SYS_ACL_IMPL_H)
typedef enum acl_type {
   ACLENT_T = 0,
   ACE_T = 1
} acl_type_t;
#endif

/*
 *
 */
extern "C" {
int acl_type(acl_t *);
char *acl_strerror(int);
};
#endif

/*
 * Cache structure in alist
 */
struct XACL_Solaris_Cache {
   ino_t inode;
   char * name;
};

/*
 *
 *
 */
class XACL_Solaris : public XACL {
private:
   alist * cache;
   bRC_XACL os_backup_acl (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL os_restore_acl (JCR *jcr, int stream, char *content, uint32_t length);
   bRC_XACL os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length);
   bRC_XACL os_get_acl(JCR *jcr, int *stream);
   bRC_XACL os_set_acl(JCR *jcr, int stream, char *content, uint32_t length);
   bRC_XACL os_get_xattr_names (JCR *jcr, POOLMEM **list, uint32_t *length);
   bRC_XACL os_get_xattr_value (JCR *jcr, char * name, char ** pvalue, uint32_t * plen);
   bRC_XACL os_set_xattr (JCR *jcr, bool extended, char *content, uint32_t length);
   bRC_XACL os_get_xattr_acl(JCR *jcr, int fd, char **buffer);
   bRC_XACL os_set_xattr_acl(JCR *jcr, int fd, char *name, char *acltext);
   /* requires acl.h available */
   bRC_XACL check_xacltype (JCR *jcr, int name);
   inline char * find_xattr_cache(JCR *jcr, ino_t ino, char * name);
   inline void delete_xattr_cache();
public:
   XACL_Solaris ();
   ~XACL_Solaris ();
};

#endif /* HAVE_SUN_OS */

#endif /* __XACL_Solaris_H_ */
