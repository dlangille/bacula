/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2019 Kern Sibbald

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
 * This is a Bacula plugin for backup/restore Docker using native tools.
 *
 * Author: Radosław Korzeniewski, MMXIX
 * radoslaw@korzeniewski.net, radekk@inteos.pl
 * Inteos Sp. z o.o. http://www.inteos.pl/
 */

#ifndef _DKINFO_H_
#define _DKINFO_H_

#include "dkid.h"
#include "pluglib.h"

/* The Docker container Power states */
typedef enum {
   DKUNKNOWN = 0,
   DKCREATED,
   DKEXITED,
   DKRUNNING,
   DKPAUSED,
} DOCKER_POWER_T;

/* Docker object types */
typedef enum {
   DOCKER_CONTAINER,
   DOCKER_IMAGE,
   DOCKER_VOLUME,
} DKINFO_OBJ_t;

/*
 * Docker objects data variables.
 */
typedef union {
   struct {
      DKID *containerid;
      POOLMEM *names;
      uint64_t size;
      DOCKER_POWER_T status;
      DKID *imagesave;
      POOLMEM *imagesave_tag;
      POOLMEM *mounts;
      alist *vols;
   } container;
   struct {
      DKID *imageid;
      POOLMEM *repository;
      uint64_t size;
      POOLMEM *tag;
      POOLMEM *repository_tag;
      utime_t created;
   } image;
   struct {
      POOLMEM *name;
      utime_t created;
      uint64_t size;
      int linknr;
   } volume;
} DOCKER_OBJ;

/* forward reference */
class DKINFO;

/*
 * This is a simple special struct for handling docker mounts destination mappings
 */
class DKVOLS : public SMARTALLOC {
public:
   DKVOLS(DKINFO *dk);
   ~DKVOLS();
   DKINFO *vol;
   POOLMEM *destination;
};

/*
 * The class which handles DKINFO operations
 */
class DKINFO : public SMARTALLOC {
 public:
    DKINFO(DKINFO_OBJ_t t);
    DKINFO(const DKINFO &dkinfo);
    ~DKINFO();

    /* set methods dedicated to container */
    inline void set_container_id(DKID &id) { if (Type == DOCKER_CONTAINER){ *data.container.containerid = id; }};
    inline void set_container_id(POOL_MEM &id) { if (Type == DOCKER_CONTAINER){ *data.container.containerid = id; }};
    inline void set_container_id(POOLMEM *id) { if (Type == DOCKER_CONTAINER){ *data.container.containerid = (char*)id; }};
    inline void set_container_names(POOLMEM *n) { if (Type == DOCKER_CONTAINER){ pm_strcpy(data.container.names, n); }};
    inline void set_container_names(POOL_MEM &n) { if (Type == DOCKER_CONTAINER){ pm_strcpy(data.container.names, n); }};
    inline void set_container_size(uint64_t s) { if (Type == DOCKER_CONTAINER){ data.container.size = s; }};
    inline void set_container_imagesave(DKID &id) { if (Type == DOCKER_CONTAINER){ *data.container.imagesave = id; }};
    inline void set_container_imagesave(POOL_MEM &id) { if (Type == DOCKER_CONTAINER){ *data.container.imagesave = id; }};
    inline void set_container_imagesave(POOLMEM *id) { if (Type == DOCKER_CONTAINER){ *data.container.imagesave = (char*)id; }};
    inline void set_container_imagesave_tag(POOL_MEM &n) { if (Type == DOCKER_CONTAINER){ pm_strcpy(data.container.imagesave_tag, n); }};
    inline void set_container_imagesave_tag(POOLMEM *n) { if (Type == DOCKER_CONTAINER){ pm_strcpy(data.container.imagesave_tag, n); }};
    inline void set_container_status(DOCKER_POWER_T s) { if (Type == DOCKER_CONTAINER){ data.container.status = s; }};
    void set_container_status(POOL_MEM &s);
    inline void set_container_mounts(POOLMEM *n) { if (Type == DOCKER_CONTAINER){ pm_strcpy(data.container.mounts, n); }};
    inline void set_container_mounts(POOL_MEM &n) { if (Type == DOCKER_CONTAINER){ pm_strcpy(data.container.mounts, n); }};

    /* special vols handling */
    inline void container_append_vols(DKVOLS *dk) { if (Type == DOCKER_CONTAINER){ data.container.vols->append(dk); }};
    inline bool container_has_vols() { return Type == DOCKER_CONTAINER ? !data.container.vols->empty() : false; };
    inline DKVOLS *container_first_vols() { return Type == DOCKER_CONTAINER ? (DKVOLS*)data.container.vols->first() : NULL; };
    inline DKVOLS *container_next_vols() { return Type == DOCKER_CONTAINER ? (DKVOLS*)data.container.vols->next() : NULL; };

    /* set methods dedicated to image */
    inline void set_image_id(DKID &id) { if (Type == DOCKER_IMAGE){ *data.image.imageid = id; }};
    inline void set_image_id(POOLMEM *id) { if (Type == DOCKER_IMAGE){ *data.image.imageid = (char*)id; }};
    inline void set_image_id(POOL_MEM &id) { if (Type == DOCKER_IMAGE){ *data.image.imageid = id; }};
    inline void set_image_repository(POOLMEM *n) { if (Type == DOCKER_IMAGE){ pm_strcpy(data.image.repository, n); render_image_repository_tag(); }};
    inline void set_image_repository(POOL_MEM &n) { if (Type == DOCKER_IMAGE){ pm_strcpy(data.image.repository, n); render_image_repository_tag(); }};
    inline void set_image_tag(POOLMEM *n) { if (Type == DOCKER_IMAGE){ pm_strcpy(data.image.tag, n); render_image_repository_tag(); }};
    inline void set_image_tag(POOL_MEM &n) { if (Type == DOCKER_IMAGE){ pm_strcpy(data.image.tag, n); render_image_repository_tag(); }};
    inline void set_image_size(uint64_t s) { if (Type == DOCKER_IMAGE){ data.image.size = s; }};
    inline void set_image_created(utime_t t) { if (Type == DOCKER_IMAGE){ data.image.created = t; }};

    /* set methods dedicated to volume */
    inline void set_volume_name(POOLMEM *n) { if (Type == DOCKER_VOLUME){ pm_strcpy(data.volume.name, n); }};
    inline void set_volume_name(POOL_MEM &n) { if (Type == DOCKER_VOLUME){ pm_strcpy(data.volume.name, n); }};
    inline void set_volume_created(utime_t t) { if (Type == DOCKER_VOLUME){ data.volume.created = t; }};
    inline void set_volume_size(uint64_t s) { if (Type == DOCKER_VOLUME){ data.volume.size = s; }};
    inline void set_volume_linknr(int l) { if (Type == DOCKER_VOLUME){ data.volume.linknr = l; }};
    inline int inc_volume_linknr() { return Type == DOCKER_VOLUME ? ++data.volume.linknr : 0; };

    /* scanning methods */
    void scan_container_size(POOLMEM *str);
    void scan_image_size(POOLMEM *str);
    void scan_image_repository_tag(POOL_MEM &rt);
    void scan_volume_size(POOLMEM *str);

    /* get methods dedicated to container */
    inline DKID *get_container_id() { return Type == DOCKER_CONTAINER ? data.container.containerid : NULL; };
    inline DKID *get_container_imagesave() { return Type == DOCKER_CONTAINER ? data.container.imagesave : NULL; };
    inline POOLMEM *get_container_names() { return Type == DOCKER_CONTAINER ? data.container.names : NULL; };
    inline DOCKER_POWER_T get_container_status() { return Type == DOCKER_CONTAINER ? data.container.status : DKUNKNOWN; };
    inline uint64_t get_container_size() { return Type == DOCKER_CONTAINER ? data.container.size : 0; };
    inline POOLMEM *get_container_imagesave_tag() { return Type == DOCKER_CONTAINER ? data.container.imagesave_tag : NULL; };
    inline POOLMEM *get_container_mounts() { return Type == DOCKER_CONTAINER ? data.container.mounts : NULL; };

    /* get methods dedicated to image */
    inline DKID *get_image_id() { return Type == DOCKER_IMAGE ? data.image.imageid : NULL; };
    inline POOLMEM *get_image_repository() { return Type == DOCKER_IMAGE ? data.image.repository : NULL; };
    inline POOLMEM *get_image_tag() { return Type == DOCKER_IMAGE ? data.image.tag : NULL; };
    inline POOLMEM *get_image_repository_tag() { return Type == DOCKER_IMAGE ? data.image.repository_tag : NULL; };
    inline uint64_t get_image_size() { return Type == DOCKER_IMAGE ? data.image.size : 0; };
    inline utime_t get_image_created() { return Type == DOCKER_IMAGE ? data.image.created : 0; };

    /* get methods dedicated to volume */
    inline POOLMEM *get_volume_name() { return Type == DOCKER_VOLUME ? data.volume.name : NULL; };
    inline utime_t get_volume_created() { return Type == DOCKER_VOLUME ? data.volume.created : 0; };
    inline uint64_t get_volume_size() { return Type == DOCKER_VOLUME ? data.volume.size : 0; };
    inline int get_volume_linknr() { return Type == DOCKER_VOLUME ? data.volume.linknr : 0; };

    /* generic get methods which check dkinfo type */
    DKID *id();
    POOLMEM *name();
    uint64_t size();
    inline DKINFO_OBJ_t type() { return Type; };
    const char *type_str();

 private:
    DKINFO_OBJ_t Type;
    DOCKER_OBJ data;

    void init(DKINFO_OBJ_t t);
    inline void render_image_repository_tag()
    {
      pm_strcpy(data.image.repository_tag, data.image.repository);
      pm_strcat(data.image.repository_tag, ":");
      pm_strcat(data.image.repository_tag, data.image.tag);
    };
};

#endif   /* _DKINFO_H_ */