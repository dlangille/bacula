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

#ifndef __BXACL_H_
#define __BXACL_H_

/*
 * Magic used in the magic field of the xattr struct.
 * This way we can see if we encounter a valid xattr struct.
 * Used for backward compatibility only.
 */
#define XATTR_MAGIC 0x5C5884

/*
 * Return value status enumeration
 * You have an error when value is less then zero.
 * You have a positive status when value is not negative
 * (greater or equal to zero).
 */
enum bRC_XACL {
   bRC_XACL_inval = -3,       // input data invalid
   bRC_XACL_fatal = -2,       // a fatal error
   bRC_XACL_error = -1,       // standard error
   bRC_XACL_ok	  = 0,	      // success
   bRC_XACL_skip  = 1,	      // processing should skip current runtime
   bRC_XACL_cont  = 2	      // processing should skip current element
			      // and continue with next one
};

/*
 * We support the following types of ACLs
 */
typedef enum {
   XACL_TYPE_NONE	      = 0,
   XACL_TYPE_ACCESS	      = 1,
   XACL_TYPE_DEFAULT	      = 2,
   XACL_TYPE_DEFAULT_DIR      = 3,
   XACL_TYPE_EXTENDED	      = 4,
   XACL_TYPE_NFS4	      = 5,
   XACL_TYPE_PLUGIN	      = 6
} XACL_type;

/*
 * Flags which control what ACL/XATTR engine
 * to use for backup/restore
 */
#define XACL_FLAG_NONE	      0
#define XACL_FLAG_NATIVE      0x01
#define XACL_FLAG_AFS	      0x02
#define XACL_FLAG_PLUGIN      0x04

/*
 * Ensure we have none
 */
#ifndef ACL_TYPE_NONE
#define ACL_TYPE_NONE	      0x0
#endif

/*
 * Extended attribute (xattr) list element.
 *
 * Every xattr consist of a Key=>Value pair where
 * both could be a binary data.
 */
struct XACL_xattr {
   uint32_t name_len;
   char *name;
   uint32_t value_len;
   char *value;
};

/*
 * Basic ACL/XATTR class which is a foundation for any other OS specyfic implementation.
 *
 * This class cannot be used directly as it is an abstraction class with a lot of virtual
 * methods laying around. As a basic class it has all public API available for backup and
 * restore functionality. As a bonus it handles all ACL/XATTR generic functions and OS
 * independent API, i.e. for AFS ACL/XATTR or Plugins ACL/XATTR (future functionality).
 */
class XACL {
private:
   bool acl_ena;
   bool xattr_ena;
   uint32_t flags;
   uint32_t current_dev;
   POOLMEM *content;
   uint32_t content_len;
   uint32_t acl_nr_errors;
   uint32_t xattr_nr_errors;
   const int *acl_streams;
   const int *default_acl_streams;
   const int *xattr_streams;
   const char **xattr_skiplist;
   const char **xattr_acl_skiplist;

   void init();

   /**
    * Perform OS specyfic ACL backup.
    * in:
    *	 jcr - Job Control Record
    *	 ff_pkt - file to backup information rector
    * out:
    *	 bRC_XACL_ok - backup performed without problems
    *	 any other - some error ocurred
    */
   virtual bRC_XACL os_backup_acl (JCR *jcr, FF_PKT *ff_pkt){return bRC_XACL_fatal;};

   /**
    * Perform OS specyfic ACL restore. Runtime is called only when stream is supported by OS.
    * in:
    *	 jcr - Job Control Record
    *	 ff_pkt - file to backup information rector
    * out:
    *	 bRC_XACL_ok - backup performed without problems
    *	 any other - some error ocurred
    */
   virtual bRC_XACL os_restore_acl (JCR *jcr, int stream, char *content, uint32_t length){return bRC_XACL_fatal;};

   /**
    * Perform OS specyfic XATTR backup.
    *
    * in:
    *	 jcr - Job Control Record
    *	 ff_pkt - file to backup control package
    * out:
    *	 bRC_XACL_ok - xattr backup ok or no xattr to backup found
    *	 bRC_XACL_error/fatal - an error or fatal error occurred
    */
   virtual bRC_XACL os_backup_xattr (JCR *jcr, FF_PKT *ff_pkt){return bRC_XACL_fatal;};

   /**
    * Perform OS specyfic XATTR restore. Runtime is called only when stream is supported by OS.
    *
    * in:
    *	 jcr - Job Control Record
    *	 stream - backup stream number
    *	 content - a buffer with data to restore
    *	 length - a data restore length
    * out:
    *	 bRC_XACL_ok - xattr backup ok or no xattr to backup found
    *	 bRC_XACL_error/fatal - an error or fatal error occurred
    */
   virtual bRC_XACL os_restore_xattr (JCR *jcr, int stream, char *content, uint32_t length){return bRC_XACL_fatal;};

   /**
    * Low level OS specyfic runtime to get ACL data from file. The ACL data is set in internal content buffer.
    *
    * in:
    *	 jcr - Job Control Record
    *	 xacltype - the acl type to restore
    * out:
    *	 bRC_XACL_ok -
    *	 bRC_XACL_error/fatal - an error or fatal error occurred
    */
   virtual bRC_XACL os_get_acl (JCR *jcr, XACL_type xacltype){return bRC_XACL_fatal;};

   /**
    * Low level OS specyfic runtime to set ACL data on file.
    *
    * in:
    *	 jcr - Job Control Record
    *	 xacltype - the acl type to restore
    *	 content - a buffer with data to restore
    *	 length - a data restore length
    * out:
    *	 bRC_XACL_ok -
    *	 bRC_XACL_error/fatal - an error or fatal error occurred
    */
   virtual bRC_XACL os_set_acl (JCR *jcr, XACL_type xacltype, char *content, uint32_t length){return bRC_XACL_fatal;};

   /**
    * Returns a list of xattr names in newly allocated pool memory and a length of the allocated buffer.
    * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
    * when not needed. The list of xattr names is returned as an unordered array of NULL terminated
    * character strings (attribute names are separated by NULL characters), like this:
    *  user.name1\0system.name1\0user.name2\0
    * The format of the list is based on standard "llistxattr" function call.
    * TODO: change the format of the list from an array of NULL terminated strings into an alist of structures.
    *
    * in:
    *	 jcr - Job Control Record
    *	 xlen - non NULL pointer to the uint32_t variable for storing a length of the xattr names list
    *	 pxlist - non NULL pointer to the char* variable for allocating a memoty data for xattr names list
    * out:
    *	 bRC_XACL_ok - we've got a xattr data to backup
    *	 bRC_XACL_skip - no xattr data available, no fatal error, skip rest of the runtime
    *	 bRC_XACL_fatal - when required buffers are unallocated
    *	 bRC_XACL_error - in case of any error
    */
   virtual bRC_XACL os_get_xattr_names (JCR *jcr, POOLMEM ** pxlist, uint32_t * xlen){return bRC_XACL_fatal;};

   /**
    * Returns a value of the requested attribute name and a length of the allocated buffer.
    * It allocates a memory with poolmem subroutines every time a function is called, so it must be freed
    * when not needed.
    *
    * in:
    *	 jcr - Job Control Record
    *	 name - a name of the extended attribute
    *	 pvalue - the pointer for the buffer with value - it is allocated by function and should be freed when no needed
    *	 plen - the pointer for the length of the allocated buffer
    *
    * out:
    *	 pxlist - the atributes list
    *	 bRC_XACL_ok - we've got a xattr data which could be empty when xlen=0
    *	 bRC_XACL_skip - no xattr data available, no fatal error, skip rest of the runtime
    *	 bRC_XACL_error - error getting an attribute
    *	 bRC_XACL_fatal - required buffers are unallocated
    */
   virtual bRC_XACL os_get_xattr_value (JCR *jcr, char * name, char ** pvalue, uint32_t * plen){return bRC_XACL_fatal;};

   /**
    * Low level OS specyfic runtime to set extended attribute on file
    *
    * in:
    *	 jcr - Job Control Record
    *	 xattr - the struct with attribute/value to set
    *
    * out:
    *	 bRC_XACL_ok - setting the attribute was ok
    *	 bRC_XACL_error - error during extattribute set
    *	 bRC_XACL_fatal - required buffers are unallocated
    */
   virtual bRC_XACL os_set_xattr (JCR *jcr, XACL_xattr *xattr){return bRC_XACL_fatal;};

   void inc_acl_errors(){ acl_nr_errors++;};
   void inc_xattr_errors(){ xattr_nr_errors++;};
   bRC_XACL check_dev (JCR *jcr);
   void check_dev (JCR *jcr, uint32_t dev);

public:
   XACL ();
   virtual ~XACL();

   /* enable/disable functionality */
   void enable_acl();
   void disable_acl();
   void enable_xattr();
   void disable_xattr();

   /*
    * public methods used outside the class or derivatives
    */
   bRC_XACL backup_acl (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL restore_acl (JCR *jcr, int stream, char *content, uint32_t content_length);
   bRC_XACL backup_xattr (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL restore_xattr (JCR *jcr, int stream, char *content, uint32_t content_length);

   /* utility functions */
   inline uint32_t get_acl_nr_errors(){ return acl_nr_errors;};
   inline uint32_t get_xattr_nr_errors(){ return xattr_nr_errors;};
   void set_acl_streams (const int *pacl, const int *pacl_def);
   void set_xattr_streams (const int *pxattr);
   void set_xattr_skiplists (const char **pxattr, const char **pxattr_acl);
   inline void clear_flag (uint32_t flag){ flags &= ~flag;};
   inline void set_flag (uint32_t flag){ flags |= flag;};
   POOLMEM * set_content (char *text);
   POOLMEM * set_content(char *data, int len);
   inline POOLMEM * get_content (void){ return content;};
   inline uint32_t get_content_size (void){ return sizeof_pool_memory(content);};
   inline uint32_t get_content_len (void){ return content_len;};
   bool check_xattr_skiplists (JCR *jcr, FF_PKT *ff_pkt, char * name);

   /* sending data to the storage */
   bRC_XACL send_acl_stream (JCR *jcr, int stream);
   bRC_XACL send_xattr_stream (JCR *jcr, int stream);

   /* serialize / unserialize stream */
   bRC_XACL unserialize_xattr_stream(JCR *jcr, char *content, uint32_t length, alist *list);
   bRC_XACL serialize_xattr_stream(JCR *jcr, uint32_t len, alist *list);

   /* generic functions */
   bRC_XACL generic_backup_acl (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL generic_restore_acl (JCR *jcr, int stream);
   bRC_XACL afs_backup_acl (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL afs_restore_acl (JCR *jcr, int stream);
   bRC_XACL generic_backup_xattr (JCR *jcr, FF_PKT *ff_pkt);
   bRC_XACL generic_restore_xattr (JCR *jcr, int stream);
};

void *new_xacl();

#endif /* __BXACL_H_ */
