/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2017 Kern Sibbald

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
 *  Bacula fifo list routines
 *
 *  flist is a simple malloc'ed array of pointers. Derived from alist.
 *
 *   Kern Sibbald, August 2014
 *
 */

#include "bacula.h"

void *flist::dequeue()
{
   void *item;

   if (num_items == 0) {
      return NULL;
   }
   num_items--;
   item = items[get_item];
   items[get_item++] = NULL;
   if (get_item >= max_items) {
      get_item = 0;
   }
   return item;
}


/*
 * Queue an item to the list
 */
bool flist::queue(void *item)
{
   if (num_items == max_items) {
      return false;
   }
   num_items++;
   items[add_item++] = item;
   if (add_item >= max_items) {
      add_item = 0;
   }
   return true;
}

/* Destroy the list and its contents */
void flist::destroy()
{
   if (num_items && own_items) {
      for (int i=0; i<num_items; i++) {
         if (items[i]) {
            free(items[i]);
            items[i] = NULL;
         }
      }
   }
   free(items);
   items = NULL;
}

#ifdef TEST_PROGRAM


struct FILESET {
   flist mylist;
};

int main(int argc, char *argv[])
{
   FILESET *fileset;
   char buf[30];
   flist *mlist;
   char *p, *q;
   int i;

   fileset = (FILESET *)malloc(sizeof(FILESET));
   bmemzero(fileset, sizeof(FILESET));
   fileset->mylist.init();

   printf("Manual allocation/destruction of list:\n");

   for (i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      p = bstrdup(buf);
      if (fileset->mylist.queue(p)) {
         printf("Added item = %s\n", p);
      } else {
         q = (char *)fileset->mylist.dequeue();
         printf("Dequeue item = %s\n", q);
         free(q);
         if (fileset->mylist.queue(p)) {
            printf("Added item = %s\n", p);
         } else {
            printf("Big problem could not queue item %d %s\n", i, p);
         }
      }
   }
   while ((q=(char *)fileset->mylist.dequeue())) {
      printf("Dequeue item = %s\n", q);
      free(q);
   }
   for (i=1; !fileset->mylist.empty(); i++) {
      q = (char *)fileset->mylist.dequeue();
      if (!q) {
         break;
      }
      printf("Item %d = %s\n", i, q);
      free(q);
   }
   fileset->mylist.destroy();
   free(fileset);

   printf("Allocation/destruction using new delete\n");
   mlist = New(flist(10));

   for (i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      p = bstrdup(buf);
      if (!mlist->queue(p)) {
         free(p);
         break;
      }
   }
   for (i=1; !mlist->empty(); i++) {
      p = (char *)mlist->dequeue();
      printf("Item %d = %s\n", i, p);
      free(p);
   }

   delete mlist;

   sm_dump(false);       /* test program */

}
#endif
