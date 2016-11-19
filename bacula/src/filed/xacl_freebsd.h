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

#ifndef __XACL_FreeBSD_H_
#define __XACL_FreeBSD_H_

#if defined(HAVE_FREEBSD_OS)
#include <sys/types.h>

#ifdef HAVE_SYS_ACL_H
#include <sys/acl.h>
#else
#error "configure failed to detect availability of sys/acl.h"
#endif

#if (!defined(HAVE_EXTATTR_GET_LINK) && !defined(HAVE_EXTATTR_GET_FILE)) || \
    (!defined(HAVE_EXTATTR_SET_LINK) && !defined(HAVE_EXTATTR_SET_FILE)) || \
    (!defined(HAVE_EXTATTR_LIST_LINK) && !defined(HAVE_EXTATTR_LIST_FILE)) || \
    !defined(HAVE_EXTATTR_NAMESPACE_TO_STRING) || \
    !defined(HAVE_EXTATTR_STRING_TO_NAMESPACE)
#error "Missing full support for the extattr functions."
#endif

#ifdef HAVE_SYS_EXTATTR_H
#include <sys/types.h>
#include <sys/extattr.h>
#else
#error "Missing sys/extattr.h header file"
#endif

#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#if !defined(HAVE_EXTATTR_GET_LINK) && defined(HAVE_EXTATTR_GET_FILE)
#define extattr_get_link extattr_get_file
#endif
#if !defined(HAVE_EXTATTR_SET_LINK) && defined(HAVE_EXTATTR_SET_FILE)
#define extattr_set_link extattr_set_file
#endif
#if !defined(HAVE_EXTATTR_LIST_LINK) && defined(HAVE_EXTATTR_LIST_FILE)
#define extattr_list_link extattr_list_file
#endif

/*
 *
 *
 */
class XACL_FreeBSD : public XACL {
private:
   bRC_XACL os_backup_acl (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL os_restore_acl (JCR *jcr, int stream, char *content, uint32_t length);
   bRC_XACL os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length);
   bRC_XACL os_get_acl(JCR *jcr, XACL_type xacltype);
   bRC_XACL os_set_acl(JCR *jcr, XACL_type xacltype, char *content, uint32_t length);
   bRC_XACL os_get_xattr_names (JCR *jcr, const int ns, POOLMEM **list, uint32_t *length);
   bRC_XACL os_get_xattr_value (JCR *jcr, const int ns, char * name, char ** pvalue, uint32_t * plen);
   bRC_XACL os_set_xattr (JCR *jcr, XACL_xattr *xattr);
   /* requires acl.h available */
   bool acl_issimple(acl_t acl);
   acl_type_t get_acltype(XACL_type xacltype);
   int acl_nrentries(acl_t acl);
   bRC_XACL check_xacltype (JCR *jcr, int name);
public:
   XACL_FreeBSD ();
};

#endif /* HAVE_FREEBSD_OS */

#endif /* __XACL_FreeBSD_H_ */
