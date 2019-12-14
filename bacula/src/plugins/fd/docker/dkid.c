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

#include "dkid.h"

/*
 * DKID class constructor, does default initialization.
 */
DKID::DKID()
{
   bmemzero(Digest, DKIDDIGESTSIZE + 1);
   ShortD = DKIDInvalid;
   shortonly = false;
};

/*
 * DKID class constructor, does parm initialization.
 */
DKID::DKID(const char* data)
{
   init(data);
};

/*
 * DKID class constructor, does parm initialization.
 */
DKID::DKID(POOL_MEM& data)
{
   init(data.c_str());
};

/*
 * DKID initialization from string.
 *    as the usable area of short sha256 version used in Docker is 6bytes/48bits
 *    and we are using a 64bit (signed) integer then we have a plenty of space to mark
 *    invalid sha256 conversion with a negative ShortD value.
 */
void DKID::init(const char* data)
{
   int len;
   int a;
   unsigned char c;
   bool valid = true;
   char *dig = (char*)data;

   if (dig != NULL){
      /* check for sha256: prefix*/
      if (strstr(dig, "sha256:") == dig){
         dig += 7;
      }
      len = strlen(dig);
      /* check for invalid input data */
      for (a = 0; a < (len > DKIDDIGESTShortSIZE ? DKIDDIGESTShortSIZE : len); a++){
         // we are checking for ASCII codes, a subset of UTF-8 for short digest only
         c = (unsigned char)dig[a];
         if (c > 'f' || (c > '9' && c < 'A') || (c > 'F' && c < 'a')){
            valid = false;
            break;
         }
      }
      if (valid){
         if (len > DKIDDIGESTShortSIZE){
            /* initialize from full data */
            memcpy(Digest, dig, DKIDDIGESTSIZE);
            Digest[DKIDDIGESTSIZE] = 0;
            shortonly = false;
         } else {
            /* handle short data */
            memcpy(Digest, dig, len);
            memcpy(Digest + len, "(...)\0", 6);
            shortonly = true;
         }
         memcpy(DigestShort, dig, DKIDDIGESTShortSIZE);
         DigestShort[DKIDDIGESTShortSIZE] = 0;
         ShortD = strtol(DigestShort, NULL, 16);
      } else {
         ShortD = DKIDInvalid;
         shortonly = false;
      }
   }
};

/*
 * Basic assignment operator overloading for string.
 *
 * in:
 *    data - the null terminated string where up to 64 chars will be used
 * out:
 *    reinitialized DKID class
 */
DKID& DKID::operator= (char* data)
{
   init(data);
   return *this;
};

/*
 * Basic assignment operator overloading for POOL_MEM class.
 *
 * in:
 *    data - a reference to POOL_MEM class instance which is used as a source
 *           of null terminated string for initialization
 * out:
 *    reinitialized DKID class
 */
DKID& DKID::operator =(POOL_MEM &data)
{
   init(data.c_str());
   return *this;
}

/*
 * Basic assignment operator overloading for DKID class.
 *
 * in:
 *    other - a reference to another DKID class instance which will be used for
 *            assignment
 * out:
 *    reinitialized DKID class
 */
DKID& DKID::operator =(DKID &other)
{
   memcpy(Digest, other.Digest, DKIDDIGESTSIZE);
   memcpy(DigestShort, other.DigestShort, DKIDDIGESTShortSIZE);
   Digest[DKIDDIGESTSIZE] = 0;
   DigestShort[DKIDDIGESTShortSIZE] = 0;
   ShortD = other.ShortD;
   shortonly = other.shortonly;
   return *this;
}

/*
 * Equal to operator overloading for DKID class.
 *
 * in:
 *    other - a reference to another DKID class instance which will be used for
 *            comparison
 * out:
 *    true - if both ShortD are the same
 *    false - if ShortD variables differ or any DKID is invalid
 */
bool DKID::operator ==(DKID &other)
{
   if (ShortD >= 0 && other.ShortD >= 0 && ShortD == other.ShortD &&
         (shortonly || other.shortonly || bstrcmp(Digest, other.Digest))){
      return true;
   }
   return false;
}

/*
 * Not-Equal to operator overloading for DKID class.
 *
 * in:
 *    other - a reference to another DKID class instance which will be used for
 *            comparison
 * out:
 *    true - if ShortD variables differ and none of them are invalid
 *    false - if both ShortD are the same or any DKID is invalid
 */
bool DKID::operator !=(DKID &other)
{
   if (ShortD >= 0 && other.ShortD >= 0 && ShortD != other.ShortD){
      return true;
   }
   if (!shortonly && !other.shortonly && !bstrcmp(Digest, other.Digest)){
      return true;
   }
   return false;
}

#ifndef TEST_PROGRAM
#define TEST_PROGRAM_A
#endif

#ifdef TEST_PROGRAM
#include "unittests.h"

void DKID::dump()
{
   printf ("%p::ShortD: %ld\n", this, ShortD);
   printf ("%p::Digest: %s\n", this, Digest);
   printf ("%p::shortonly: %s\n", this, shortonly?"true":"false");
   printf ("%p::DigestShort: %s\n", this, DigestShort);
};

const char *dig1     =  "66f45d8601bae26a6b2ffeb46922318534d3b3905377b3a224693bd78601cb3b";
const char *sdig1    =  "66f45d8601ba";
const int64_t vdig1 = 0x66f45d8601ba;
const char *dig2     =  "B546087C43F75A2C1484B4AEE0737499AA69A09067B04237907FCCD4BDE938C7";
const char *sdig2    =  "B546087C43F7";
const int64_t vdig2 = 0xb546087c43f7;
const char *sdig3    =  "0f601bcb1ef5";
const int64_t vdig3 = 0x0f601bcb1ef5;
const char *sdig4    =  "00571da76d";
const int64_t vdig4 = 0x00571da76d;
const char *dig5     = "sha256:daabf4372f900cb1ad0db17d26abf3acce55224275d1850f02459180e4dacf1d";
const char *tdig5    = "daabf4372f900cb1ad0db17d26abf3acce55224275d1850f02459180e4dacf1d";
const char *sdig5    = "daabf4372f90";
const int64_t vdig5  = 0xdaabf4372f90;
const char *sinv1 = "Invalid initialization string";
const char *sinv2 = "brave_edison";
const char *sinv3 = "0xDEADBEEF";
const char *sinv4 = "c0a478d317195b…";
const char *sinv5 = "a478d317195b…";
const char *sinv6 = "78d317195b…";

int main()
{
   Unittests dkid_test("dkid_test");
   DKID *id;
   DKID id2(dig2);
   char *p;
   int64_t v;
   POOL_MEM m(PM_FNAME);

   Pmsg0(0, "Initialize tests ...\n");

   id = New(DKID);
   ok(id && id->id() == DKIDInvalid, "Check default initialization short");
   ok(id && strlen(id->digest()) == 0, "Check default initialization full");
   ok(id && strlen(id->digest_short()) == 0, "Check short default initialization full");
   delete(id);

   id = New(DKID(dig1));
   ok(id && id->id() == vdig1, "Check param initialization short");
   ok(id && bstrcmp(id->digest(), dig1), "Check param initialization full");
   ok(id && bstrcmp(id->digest_short(), sdig1), "Check short param initialization");
   delete(id);

   id = New(DKID(dig2));
   ok(id && id->id() == vdig2, "Check param initialization short upper");
   ok(id && bstrcmp(id->digest(), dig2), "Check param initialization full upper");
   ok(id && bstrcmp(id->digest_short(), sdig2), "Check short param initialization full upper");
   delete(id);

   Mmsg(m, "%s", dig1);
   id = New(DKID(m));
   ok(id && id->id() == vdig1, "Check pool_mem initialization short");
   ok(id && bstrcmp(id->digest(), dig1), "Check pool_mem initialization full");
   ok(id && bstrcmp(id->digest_short(), sdig1), "Check short pool_mem initialization full");
   delete(id);

   id = New(DKID(sdig3));
   ok(id && id->id() == vdig3, "Check short digest initialization");
   Mmsg(m, "%s(...)", sdig3);
   ok(id && bstrcmp(id->digest(), m.c_str()), "Check short digest initialization full str");
   ok(id && bstrcmp(id->digest_short(), sdig3), "Check short for short digest initialization");
   delete(id);

   id = New(DKID(sdig4));
   ok(id && id->id() == vdig4, "Check shorter digest initialization");
   Mmsg(m, "%s(...)", sdig4);
   ok(id && bstrcmp(id->digest(), m.c_str()), "Check shorter digest initialization full str");
   ok(id && bstrcmp(id->digest_short(), sdig4), "Check short for shorter digest initialization");
   delete(id);

   id = New(DKID(dig5));
   ok(id && id->id() == vdig5, "Check param initialization with sha256: prefix");
   ok(id && bstrcmp(id->digest(), tdig5), "Check param initialization full with sha256: prefix");
   ok(id && bstrcmp(id->digest_short(), sdig5), "Check short param initialization with sha256: prefix");
   delete(id);

   Pmsg0(0, "Invalid initialization tests ...\n");

   id = New(DKID(sinv1));
   ok(id && id->id() < 0, "Checking invalid digest string long");
   delete(id);

   id = New(DKID(sinv2));
   ok(id && id->id() < 0, "Checking invalid digest string short");
   delete(id);

   id = New(DKID(sinv3));
   ok(id && id->id() < 0, "Checking invalid digest string hex");
   delete(id);

   id = New(DKID(sinv4));
   ok(id && id->id() >= 0, "Checking digest string with ellipsis");
   delete(id);

   id = New(DKID(sinv5));
   ok(id && id->id() >= 0, "Checking digest string with ellipsis short");
   delete(id);

   id = New(DKID(sinv6));
   ok(id && id->id() < 0, "Checking invalid digest string with ellipsis short");
   delete(id);

   Pmsg0(0, "Operators tests ...\n");

   id = New(DKID(dig1));
   p = (char*)id;
   ok(bstrcmp(p, dig1), "Checking operator char* ()");
   v = *id;
   ok(v == vdig1, "Checking operator int64_t ()");

   id2 = *id;
   ok(id2.id() == vdig1, "Checking operator= (DKID&)");
   ok(id2 == *id, "Checking operator== on the same");
   nok(id2 != *id, "Checking operator!= on the same");

   *id = (char*)dig2;
   ok(id->id() == vdig2, "Checking operator= (char*)");
   nok(id2 == *id, "Checking operator== on different");
   ok(id2 != *id, "Checking operator!= on different");

   *id = m;
   ok(id2.id() == vdig1, "Checking operator= (POOL_MEM&)");

   id2 = (char*)dig2;
   ok(id2.id() == vdig2, "Checking operator= (char*)");
   delete(id);

   id = New(DKID(sinv1));
   id2 = *id;
   nok (id2 == *id, "Checking operator== on invalid digest");
   nok (id2 != *id, "Checking operator!= on invalid digest");
   delete(id);

   id = New(DKID(sdig1));
   id2 = (char*)dig1;
   ok (id2 == *id, "Checking operator== on full and short digest");
   nok (id2 != *id, "Checking operator!= on full and short digest");
   delete(id);

   return report();
};

#endif   /* TEST_PROGRAM */