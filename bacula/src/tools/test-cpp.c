/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2020 Kern Sibbald

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

/* Written by Eric Bollengier March 2021 */

#include "bacula.h"
#include "lib/unittests.h"

/* These macro are defined in src/baconfig.h */
#if __cplusplus >= 201103L
# define bdelete_and_null_auto(a) do{if(a){auto b__ = a; (a)=NULL; delete b__;}} while(0)
#else
# define do{ok(1, "auto not available");} while (0)
#endif

# ifdef HAVE_TYPEOF
#  define bdelete_and_null_typeof(a) do{if(a){typeof(a) b__ = a; (a)=NULL; delete b__;}} while(0)
# else
# define do{ok(1, "typeof not available");} while (0)
#endif

class obj
{
public:
   obj(){};
   ~obj(){ok(1, "delete was called properly");}
};

int main()
{
   Unittests alist_test("test-cpp");
   log("Test C++ Features ...");
   obj *a = new obj();
   obj *b = new obj();
   char *c = (char *)malloc(10);
   bdelete_and_null_auto(a);
   bdelete_and_null_typeof(b);
   bfree_and_null(c);
   is(unittest_get_nb_tests(), 2, "Test if the two delete were done");
   ok(c == NULL, "bfree_and_null()");
   ok(a == NULL, "bdelete_and_null_auto()");
   ok(b == NULL, "bdelete_and_null_typeof()");
   return report();
}
