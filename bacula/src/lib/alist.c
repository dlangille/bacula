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
 *  Bacula array list routines
 *
 *    alist is a simple malloc'ed array of pointers.  For the moment,
 *      it simply malloc's a bigger array controlled by num_grow.
 *      Default is to realloc the pointer array for each new member.
 *
 *    Note: the list can have holes (empty items). This is done by
 *      using get() and put().  If you are using this kind of indexed
 *      list, you cannot use: prepend() and remove() as they will
 *      reorder the list. So, in the ilist array, these functions are
 *      disabled and the put method is defined.
 *
 *   Kern Sibbald, June MMIII
 *
 */

#include "bacula.h"

/*
 * Private grow list function. Used to insure that
 *   at least one more "slot" is available.
 */
void baselist::grow_list()
{
   int i;
   int new_max_items;

   /* put() can insert and item anywhere in the list so 
   it's important to allocate at least last_item+1 items */
   int min_grow = MAX(10, last_item+1);
   if (num_grow < min_grow) {
      num_grow = min_grow;               /* default if not initialized */
   }

   if (items == NULL) {
      items = (void **)malloc(num_grow * sizeof(void *));
      for (i=0; i<num_grow; i++) {
         items[i] = NULL;
      }
      max_items = num_grow;
   } else if (last_item >= max_items) {
      new_max_items = last_item + num_grow;
      items = (void **)realloc(items, new_max_items * sizeof(void *));
      for (i=max_items; i<new_max_items; i++) {
         items[i] = NULL;
      }
      max_items = new_max_items;
   }
}

void *alist::first()
{
   cur_item = 1;
   if (num_items == 0) {
      return NULL;
   } else {
      return items[0];
   }
}

void *alist::last()
{
   if (num_items == 0) {
      return NULL;
   } else {
      cur_item = last_item;
      return items[last_item-1];
   }
}

void *alist::next()
{
   if (cur_item >= last_item) {
      return NULL;
   } else {
      return items[cur_item++];
   }
}

void *alist::prev()
{
   if (cur_item <= 1) {
      return NULL;
   } else {
      return items[--cur_item];
   }
}

/*
 * prepend an item to the list -- i.e. add to beginning
 */
void alist::prepend(void *item)
{
   grow_list();
   if (num_items == 0) {
      items[num_items++] = item;
      if (num_items > last_item) {
         last_item = num_items;
      }
      return;
   }
   for (int i=last_item; i > 0; i--) {
      items[i] = items[i-1];
   }
   items[0] = item;
   num_items++;
   last_item++;
}


/*
 * Append an item to the list
 */
void baselist::append(void *item)
{
   grow_list();
   items[last_item++] = item;
   num_items++;
}

/*
 * Put an item at a particular index
 */
void ilist::put(int index, void *item)
{
   if (index > last_item) {
      last_item = index;
   }
   grow_list();
   if (items[index] == NULL) {
      num_items++;
   }
   items[index] = item;
}


/*
 * Remove an item from the list
 * Note: you must free the item when
 *   you are done with it.
 */
void * baselist::remove_item(int index)
{
   void *item;
   if (index < 0 || index >= last_item) {
      return NULL;
   }
   item = items[index];

   /* last_item is from 1..n, we work from 0..n-1 */
   for (int i=index; i < (last_item-1); i++) {
      items[i] = items[i+1];
   }

   items[last_item-1] = NULL;   /* The last item is shifted by one, the last slot is always free */

   last_item--;                 /* We have shifted all items by 1 */
   num_items--;                 /* We have 1 item less */

   return item;
}


/* Get the index item -- we should probably allow real indexing here */
void * baselist::get(int index)
{
   if (items == NULL || index < 0 || index > last_item) {
      return NULL;
   }
   return items[index];
}

/* Destroy the list and its contents */
void baselist::destroy()
{
   if (items) {
      if (own_items) {
         for (int i=0; i<max_items; i++) {
            if (items[i]) {
               free(items[i]);
               items[i] = NULL;
            }
         }
      }
      free(items);
      items = NULL;
   }
   num_items = 0;
   last_item = 0;
   max_items = 0;
   num_grow = 0;
}

#ifdef TEST_PROGRAM


struct FILESET {
   alist mylist;
};

int main()
{
   FILESET *fileset;
   char buf[30];
   alist *mlist;
   char *bp;

   fileset = (FILESET *)malloc(sizeof(FILESET));
   bmemzero(fileset, sizeof(FILESET));
   fileset->mylist.init();

   printf("Manual allocation/destruction of list:\n");

   for (int i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      fileset->mylist.append(bstrdup(buf));
   }
   for (int i=0; i< fileset->mylist.size(); i++) {
      printf("Item %d = %s\n", i, (char *)fileset->mylist[i]);
   }
   fileset->mylist.destroy();
   free(fileset);

   printf("Allocation/destruction using new delete\n");
   mlist = New(alist(50));

   for (int i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      mlist->append(bstrdup(buf));
   }
   for (int i=0; i< mlist->size(); i++) {
      printf("Item %d = %s\n", i, (char *)mlist->get(i));
   }
   printf("\nIndexed test. Insert 210 items.\n");
   /* Test indexed list */
   mlist->destroy();
   delete mlist;

   {
      printf("Test remove()\n");
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("trash"));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("3"));
      free(alst->remove(0));
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ASSERT(nb == i);
         printf("%d %s\n", i++, elt);
      }
      delete alst;
   }

   {
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("trash"));
      alst->append(bstrdup("3"));
      free(alst->remove(3));
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ASSERT(nb == i);
         printf("%d %s\n", i++, elt);
      }
      delete alst;
   }

   {
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("3"));
      alst->append(bstrdup("trash"));
      free(alst->remove(4));
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ASSERT(nb == i);
         printf("%d %s\n", i++, elt);
      }
      delete alst;
   }

   {
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("3"));
      alst->append(bstrdup("4"));
      ASSERT(alst->remove(5) == NULL);
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ASSERT(nb == i);
         printf("%d %s\n", i++, elt);
      }
      delete alst;
   }

   {
      printf("Test pop()\n");
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("3"));
      alst->append(bstrdup("trash"));
      ASSERT(alst->last_index() == 5);
      free(alst->pop());
      ASSERT(alst->last_index() == 4);
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ASSERT(nb == i);
         printf("%d %s\n", i++, elt);
      }
      delete alst;
   }
   {
      ilist *ilst = New(ilist(10, owned_by_alist));
      sprintf(buf, "This is item 10");
      ilst->put(10, bstrdup(buf));
      printf("ilst size is %d. last_item=%d.  max_items=%d\n",
         ilst->size(), ilst->last_index(), ilst->max_size());
      ASSERT(ilst->size() == 1);
      delete ilst;
   }

   {
      ilist *ilst = New(ilist(10, not_owned_by_alist));
      ilst->put(15, (char *)"something");
      printf("ilst size is %d. last_item=%d.  max_items=%d\n",
         ilst->size(), ilst->last_index(), ilst->max_size());
      ASSERT(ilst->size() == 1);
      ASSERT(ilst->last_index() == 15);
      delete ilst;
   }

   ilist *ilst = New(ilist(50));
   for (int i=0; i<115; i++) {
      sprintf(buf, "This is item %d", i);
      ilst->put(i, bstrdup(buf));
      printf("%s\n", buf);
   }
   printf("ilst size is %d. last_item=%d.  max_items=%d\n",
       ilst->size(), ilst->last_index(), ilst->max_size());
   for (int i=0; i< ilst->size(); i++) {
      printf("Item %d = %s\n", i, (char *)ilst->get(i));
   }

   delete ilst;

   printf("Test alist push().\n");
   mlist = New(alist(10));

   printf("mlist size is %d. last_item=%d.  max_items=%d\n",
       mlist->size(), mlist->last_index(), mlist->max_size());
   for (int i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      mlist->push(bstrdup(buf));
      printf("mlist size is %d. last_item=%d.  max_items=%d\n",
          mlist->size(), mlist->last_index(), mlist->max_size());
   }
   printf("Test alist pop()\n");
   for (int i=0; (bp=(char *)mlist->pop()); i++) {
      printf("Item %d = %s\n", i, bp);
      free(bp);
   }
   printf("mlist size is %d. last_item=%d.  max_items=%d\n",
       mlist->size(), mlist->last_index(), mlist->max_size());

   for (int i=0; i<mlist->max_size(); i++) {
      bp = (char *) mlist->get(i);
      if (bp != NULL) {
         printf("Big problem. Item %d item=%p, but should be NULL\n", i, bp);
      }
   }
   printf("Done push() pop() tests\n");

   delete mlist;

   ilst = New(ilist(10, not_owned_by_alist));
   ilst->put(1, ilst);
   ilst->append((void*)1);
   //ilist->first();
   //ilist->remove(1);
   delete ilst;
   {
      ilist a(4, not_owned_by_alist);
      a.append((void*)"test1");

      ilist b(4, not_owned_by_alist);
      bmemzero(&b, sizeof b);
      b.append((void*)"test1");
   }
   sm_dump(false);       /* test program */
}
#endif
