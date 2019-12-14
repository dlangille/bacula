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
 *
 * TODO: add unittests
 */

#include "dkinfo.h"

/*
 * libbac uses its own sscanf implementation which is not compatible with
 * libc implementation, unfortunately.
 * use bsscanf for Bacula sscanf flavor
 */
#ifdef sscanf
#undef sscanf
#endif

/*
 * The base constructor
 */
DKVOLS::DKVOLS(DKINFO *dk)
{
   vol = dk;
   destination = get_pool_memory(PM_FNAME);
};

/*
 * Default class destructor
 */
DKVOLS::~DKVOLS()
{
   // WARINIG: The this->destination is freed outside the class
   // TODO: It should be verified why and where the destination variable is freed!
   // free_and_null_pool_memory(destination);
};

/*
 * DKINFO class constructor, does initialization for unknown.
 */
DKINFO::DKINFO(DKINFO_OBJ_t t)
{
   init(t);
};

/*
 * DKINFO base copy constructor by simple copy variables by value
 */
DKINFO::DKINFO(const DKINFO& dkinfo)
{
   init(dkinfo.Type);
   switch (Type){
      case DOCKER_CONTAINER:
         set_container_id(*dkinfo.data.container.containerid);
         set_container_names(dkinfo.data.container.names);
         set_container_size(dkinfo.data.container.size);
         set_container_mounts(dkinfo.data.container.mounts);
         set_container_status(dkinfo.data.container.status);
         set_container_imagesave(*dkinfo.data.container.imagesave);
         set_container_imagesave_tag(dkinfo.data.container.imagesave_tag);
         break;
      case DOCKER_IMAGE:
         set_image_id(*dkinfo.data.image.imageid);
         set_image_repository(dkinfo.data.image.repository);
         set_image_tag(dkinfo.data.image.tag);
         set_image_size(dkinfo.data.image.size);
         set_image_created(dkinfo.data.image.created);
         break;
      case DOCKER_VOLUME:
         set_volume_name(dkinfo.data.volume.name);
         set_volume_created(dkinfo.data.volume.created);
         set_volume_size(dkinfo.data.volume.size);
         set_volume_linknr(dkinfo.data.volume.linknr);
         break;
   }
};

/*
 * DKINFO destructor, releases all memory allocated.
 */
DKINFO::~DKINFO()
{
   DKVOLS *v;

   switch(Type){
      case DOCKER_CONTAINER:
         if (data.container.containerid){
            delete data.container.containerid;
         }
         if (data.container.imagesave){
            delete data.container.imagesave;
         }
         if (data.container.vols){
            foreach_alist(v, data.container.vols){
               delete v;
            }
            delete data.container.vols;
         }
         free_and_null_pool_memory(data.container.names);
         free_and_null_pool_memory(data.container.mounts);
         free_and_null_pool_memory(data.container.imagesave_tag);
         break;
      case DOCKER_IMAGE:
         if (data.image.imageid){
            delete data.image.imageid;
         }
         free_and_null_pool_memory(data.image.repository);
         free_and_null_pool_memory(data.image.tag);
         free_and_null_pool_memory(data.image.repository_tag);
         break;
      case DOCKER_VOLUME:
         free_and_null_pool_memory(data.volume.name);
         break;
      default:
         break;
   }
};

/*
 * initialization of the DKINFO class.
 */
void DKINFO::init(DKINFO_OBJ_t t)
{
   Type = t;
   switch(Type){
      case DOCKER_CONTAINER:
         data.container.containerid = New(DKID);
         data.container.names = get_pool_memory(PM_NAME);
         data.container.size = 0;
         data.container.mounts = get_pool_memory(PM_MESSAGE);
         data.container.status = DKUNKNOWN;
         data.container.imagesave = New(DKID);
         data.container.imagesave_tag = get_pool_memory(PM_NAME);
         data.container.vols = New(alist(10, not_owned_by_alist));
         break;
      case DOCKER_IMAGE:
         data.image.imageid = New(DKID);
         data.image.repository = get_pool_memory(PM_NAME);
         data.image.size = 0;
         data.image.tag = get_pool_memory(PM_NAME);
         data.image.repository_tag = get_pool_memory(PM_NAME);
         data.image.created = 0;
         break;
      case DOCKER_VOLUME:
         data.volume.name = get_pool_memory(PM_NAME);
         data.volume.created = 0;
         data.volume.linknr = 1;
         break;
      default:
         bmemzero(&data, sizeof(data));
   }
};

/*
 * Sets the container size based on the docker size string:
 *       "123B (virtual 319MB)"
 *
 * in:
 *    s - the string which represents the Docker container size
 * out:
 *    none
 */
void DKINFO::scan_container_size(POOLMEM* str)
{
   int status;
   float srw;
   char suff[2];
   float sv;
   uint64_t srwsize, svsize;

   if (Type == DOCKER_CONTAINER && str){
      status = sscanf(str, "%f%c%*c%*s%f%c", &srw, &suff[0], &sv, &suff[1]);
      if (status == 4){
         /* scan successful */
         srwsize = pluglib_size_suffix(srw, suff[0]);
         svsize = pluglib_size_suffix(sv, suff[1]);
         data.container.size = srwsize + svsize;
      }
   }
};

/*
 * Sets the image size based on the docker size string:
 *       "319MB"
 *
 * in:
 *    s - the string which represents the Docker image size
 * out:
 *    none
 */
void DKINFO::scan_image_size(POOLMEM* str)
{
   int status;
   float fsize;
   char suff;

   if (Type == DOCKER_IMAGE && str){
      status = sscanf(str, "%f%c", &fsize, &suff);
      if (status == 2){
         /* scan successful */
         data.image.size = pluglib_size_suffix(fsize, suff);
      }
   }
};

/*
 * Sets the volume size based on the docker volume size string:
 *       "319MB"
 *
 * in:
 *    s - the string which represents the Docker volume size
 * out:
 *    none
 */
void DKINFO::scan_volume_size(POOLMEM* str)
{
   int status;
   float fsize;
   char suff;

   if (Type == DOCKER_VOLUME && str){
      if (bstrcmp(str, "N/A")){
         data.volume.size = 0;
      } else {
         status = sscanf(str, "%f%c", &fsize, &suff);
         if (status == 2){
            /* scan successful */
            data.volume.size = pluglib_size_suffix(fsize, suff);
         }
      }
   }
};

/*
 * Setup an image repository/tag variables from a single image repository:tag string.
 *    The class uses three variables to store repository:tag data.
 *    - data.image.repository_tag is used for full info string
 *    - data.image.repository is used to store repository part
 *    - data.image.tag is used to store a tag part
 * TODO: optimize repository_tag storage
 *
 * in:
 *    rt - a repository:tag string
 * out:
 *    none
 */
void DKINFO::scan_image_repository_tag(POOL_MEM& rt)
{
   char *colon;

   if (Type == DOCKER_IMAGE){
      pm_strcpy(data.image.repository_tag, rt);
      colon = strchr(data.image.repository_tag, ':');
      if (colon){
         /* have a colon in string, so split it */
         pm_strcpy(data.image.tag, colon);
         *colon = 0;    // temporary usage
         pm_strcpy(data.image.repository, data.image.repository_tag);
         *colon = ':';  // restore
      } else {
         pm_strcpy(data.image.repository, rt);
         pm_strcpy(data.image.tag, NULL);
      }
   }
};

/*
 * Sets the container status based on the status string.
 *
 * in:
 *    s - the string which represents the status
 * out:
 *    none
 */
void DKINFO::set_container_status(POOL_MEM &s)
{
   if (Type == DOCKER_CONTAINER){
      /* scan a container state and save it */
      if (bstrcmp(s.c_str(), "exited")){
         /* container exited */
         data.container.status = DKEXITED;
      } else
      if (bstrcmp(s.c_str(), "running")){
         /* vm is running */
         data.container.status = DKRUNNING;
      } else
      if (bstrcmp(s.c_str(), "paused")){
         /* container paused */
         data.container.status = DKPAUSED;
      } else {
         data.container.status = DKUNKNOWN;
      }
   }
}

/* fake dkid for volumes */
static DKID volfakeid;

/*
 * Return object ID based on object type.
 */
DKID *DKINFO::id()
{
   switch(Type){
      case DOCKER_CONTAINER:
         return data.container.containerid;
      case DOCKER_IMAGE:
         return data.image.imageid;
      case DOCKER_VOLUME:
         return &volfakeid;
   }
   return NULL;
};

/*
 * Return object name based on object type.
 */
POOLMEM *DKINFO::name()
{
   switch(Type){
      case DOCKER_CONTAINER:
         return data.container.names;
      case DOCKER_IMAGE:
         return data.image.repository_tag;
      case DOCKER_VOLUME:
         return data.volume.name;
   }
   return NULL;
};

/*
 * Return object type string constant.
 */
const char *DKINFO::type_str()
{
   switch(Type){
      case DOCKER_CONTAINER:
         return "Docker Container";
      case DOCKER_IMAGE:
         return "Docker Image";
      case DOCKER_VOLUME:
         return "Docker Volume";
   }
   return "Unknown";
};

/*
 * Return object size info.
 */
uint64_t DKINFO::size()
{
   switch(Type){
      case DOCKER_CONTAINER:
         return data.container.size;
      case DOCKER_IMAGE:
         return data.image.size;
      default:
         break;
   }
   return 0;
};
