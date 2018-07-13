/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2018 Kern Sibbald

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
               bfree(items[i]);
               items[i] = NULL;
            }
         }
      }
      bfree(items);
      items = NULL;
   }
   num_items = 0;
   last_item = 0;
   max_items = 0;
   num_grow = 0;
}

#ifdef TEST_PROGRAM

#include "rtest.h"

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

   log("Automatic allocation/destruction of list:");

   for (int i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      fileset->mylist.append(bstrdup(buf));
   }
/*
 * for (int i=0; i< fileset->mylist.size(); i++) {
 *    log("Item %d = %s", i, (char *)fileset->mylist[i]);
 * }
 */
   ok(fileset->mylist.size() == 20, "Check fileset size");
   ok(strcmp((char*)fileset->mylist.last(), "This is item 19") == 0, "Check last item");
   fileset->mylist.destroy();
   ok(fileset->mylist.size() == 0, "Check fileset size after delete");
   ok(fileset->mylist.last() == NULL, "Check fileset last after delete");
   free(fileset);

   log("Allocation/destruction using new delete");
   mlist = New(alist(50));

   for (int i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      mlist->append(bstrdup(buf));
   }
/*
 * for (int i=0; i< mlist->size(); i++) {
 *    log("Item %d = %s", i, (char *)mlist->get(i));
 * }
 */
   ok(mlist->size() == 20, "Check fileset size");
   ok(strcmp((char*)mlist->last(), "This is item 19") == 0, "Check last item");

   /* Test indexed list */
   mlist->destroy();
   ok(mlist->size() == 0, "Check fileset size after delete");
   ok(mlist->last() == NULL, "Check fileset last after delete");
   delete mlist;

   {
      log("Test alist::remove(0)");
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
         ok(nb == i++, "Check list index after remove(0)");
      }
      delete alst;
   }

   {
      log("Test alist::remove(3)");
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
         ok(nb == i++, "Check list index after remove(3)");
      }
      delete alst;
   }

   {
      log("Test alist::remove(last)");
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
         ok(nb == i++, "Check list index after remove(last)");
      }
      delete alst;
   }

   {
      log("Test alist::remove(last+1)");
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("3"));
      alst->append(bstrdup("4"));
      ok(alst->remove(5) == NULL, "Check remove returns null");
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ok(nb == i++, "Check list index after remove(last+1)");
      }
      delete alst;
   }

   {
      log("Test alist::pop()");
      char *elt;
      int i=0;
      alist *alst = New(alist(10, owned_by_alist));
      alst->append(bstrdup("0"));
      alst->append(bstrdup("1"));
      alst->append(bstrdup("2"));
      alst->append(bstrdup("3"));
      alst->append(bstrdup("trash"));
      ok(alst->last_index() == 5, "Check last_index");
      free(alst->pop());
      ok(alst->last_index() == 4, "Check last_index after pop()");
      foreach_alist(elt, alst) {
         int nb = atoi(elt);
         ok(nb == i++, "Check list index after pop()");
      }
      delete alst;
   }
   {
      log("Test ilist::put()");
      ilist *ilst = New(ilist(10, owned_by_alist));
      sprintf(buf, "This is item 10");
      ilst->put(10, bstrdup(buf));
      log("ilst size is %d. last_item=%d.  max_items=%d",
          ilst->size(), ilst->last_index(), ilst->max_size());
      ok(ilst->size() == 1, "Check size() after put()");
      delete ilst;
   }

   {
      log("Test ilist::last_index");
      ilist *ilst = New(ilist(10, not_owned_by_alist));
      ilst->put(15, (char *)"something");
      log("ilst size is %d. last_item=%d.  max_items=%d",
         ilst->size(), ilst->last_index(), ilst->max_size());
      ok(ilst->size() == 1, "Check size() after put()");
      ok(ilst->last_index() == 15, "check last index after put()");
      delete ilst;
   }

   log("Test ilist with multiple put()");
   ilist *ilst = New(ilist(50));
   for (int i=0; i<115; i++) {
      sprintf(buf, "This is item %d", i);
      ilst->put(i, bstrdup(buf));
   }
   log("ilst size is %d. last_item=%d.  max_items=%d",
       ilst->size(), ilst->last_index(), ilst->max_size());
   ok(ilst->size() == 115, "Check ilist size after put()");
   ok(ilst->last_index() == 114, "Check ilist last_index after put()");
   for (int i=0; i< ilst->size(); i++) {
      sprintf(buf, "This is item %d", i);
      ok(strcmp(buf, (char *)ilst->get(i)) == 0, "Check ilist content");
   }

   delete ilst;

   log("Test alist push().");
   mlist = New(alist(10));
   ok(ilst->size() == 0, "Check ilist size after new()");
   ok(ilst->last_index() == 0, "Check ilist last_index after new()");

   log("mlist size is %d. last_item=%d.  max_items=%d",
       mlist->size(), mlist->last_index(), mlist->max_size());
   for (int i=0; i<20; i++) {
      sprintf(buf, "This is item %d", i);
      mlist->push(bstrdup(buf));
      ok(mlist->size() == i+1, "Check current size after push");
      ok(mlist->last_index() == i+1, "Check last_index after push");
   }
   log("Test alist pop()");
   for (int i=19; (bp=(char *)mlist->pop()); i--) {
      sprintf(buf, "This is item %d", i);
      ok(strcmp(buf, bp) == 0, "Check ilist content after pop()");
      free(bp);
   }

   ok(ilst->size() == 0, "Check ilist size after pop()");
   ok(ilst->last_index() == 0, "Check ilist last_index after pop()");

   log("mlist size is %d. last_item=%d.  max_items=%d",
       mlist->size(), mlist->last_index(), mlist->max_size());

   for (int i=0; i<mlist->max_size(); i++) {
      bp = (char *) mlist->get(i);
      ok(bp == NULL, "Check get() after pop(). Should be NULL");
   }
   delete mlist;

   ilst = New(ilist(10, not_owned_by_alist));
   ilst->put(1, ilst);
   ilst->append((void*)1);
   //ilst->first();
   //ilst->remove(1);
   delete ilst;
   {
      ilist a(4, not_owned_by_alist);
      a.append((void*)"test1");

      ilist b(4, not_owned_by_alist);
      bmemzero(&b, sizeof b);
      b.append((void*)"test1");
   }
   sm_dump(false);       /* test program */
   return report();
}
#endif
