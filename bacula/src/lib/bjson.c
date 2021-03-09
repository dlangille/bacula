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
/*
 *
 *   Bacula Json library routines
 *
 *     Kern Sibbald, September MMXII
 *
 */

#include "bacula.h"
#include "lib/breg.h"

extern s_kw msg_types[];
extern s_collt collectortypes[];
extern RES_TABLE resources[];

union URES {
   MSGS  res_msgs;
   RES hdr;
};

#if defined(_MSC_VER)
extern "C" { // work around visual compiler mangling variables
   extern URES res_all;
}
#else
extern URES res_all;
#endif

struct display_filter
{
   /* default                   { { "Director": { "Name": aa, ...} }, { "Job": {..} */
   bool do_list;             /* [ {}, {}, ..] or { "aa": {}, "bb": {}, ...} */
   bool do_one;              /* { "Name": "aa", "Description": "test, ... } */
   bool do_only_data;        /* [ {}, {}, {}, ] */
   char *resource_type;
   char *resource_name;
   regex_t directive_reg;
};

void bjson_sendit(HPKT &hpkt, const char *fmt, ...)
{
   va_list arg_ptr;
   bool done=false;

   while (!done) {
      uint32_t len = sizeof_pool_memory(hpkt.out);
      va_start(arg_ptr, fmt);
      // The return value of our bvsnprintf() doesn't represent
      // the full length of the string, we need to check it afterward
      bvsnprintf(hpkt.out, len, (char *)fmt, arg_ptr);
      va_end(arg_ptr);

      if (strlen(hpkt.out) >= (len - 1)) {
         // We got an overflow, we need more room to display the output
         hpkt.out = check_pool_memory_size(hpkt.out, len*2);

      } else {
         done = true;
      }
   }

   fputs(hpkt.out, stdout);
   fflush(stdout);
}

void init_hpkt(HPKT &hpkt)
{
   memset(&hpkt, 0, sizeof(hpkt));
   hpkt.edbuf = get_pool_memory(PM_EMSG);
   hpkt.edbuf2 = get_pool_memory(PM_EMSG);
   hpkt.out = get_pool_memory(PM_EMSG);
   hpkt.json = true;
   hpkt.hfunc = HF_DISPLAY;
   hpkt.sendit = bjson_sendit;
}

void term_hpkt(HPKT &hpkt)
{
   free_pool_memory(hpkt.edbuf);
   free_pool_memory(hpkt.edbuf2);
   free_pool_memory(hpkt.out);
   memset(&hpkt, 0, sizeof(hpkt));
}

/*
 * Strip long options out of fo->opts string so that
 *   they will not give us false matches for regular
 *   1 or 2 character options.
 */
void strip_long_opts(char *out, const char *in)
{
   const char *p;
   for (p=in; *p; p++) {
      switch (*p) {
      /* V, C, J, and P are long options, skip them */
      case 'V':
      case 'C':
      case 'J':
      case 'P':
         while (*p != ':') {
            p++;       /* skip to after : */
         }
         break;
      /* Copy everything else */
      default:
         *out++ = *p;
         break;
      }
   }
   *out = 0;           /* terminate string */
}

void edit_alist(HPKT &hpkt)
{
   bool f = true;
   char *citem;

   pm_strcpy(hpkt.edbuf, " [");
   foreach_alist(citem, hpkt.list) {
      if (!f) {
         pm_strcat(hpkt.edbuf, ", ");
      }
      pm_strcat(hpkt.edbuf, quote_string(hpkt.edbuf2, citem));
      f = false;
   }
   pm_strcat(hpkt.edbuf, "]");
}

void edit_msg_types(HPKT &hpkt, DEST *dest)
{
   int i, j, count = 0;
   bool first_type = true;
   bool found;

   pm_strcpy(hpkt.edbuf, "[");
   for (i=1; i<=M_MAX; i++) {
      if (bit_is_set(i, dest->msg_types)) {
         found = false;
         if (!first_type) pm_strcat(hpkt.edbuf, ",");
         first_type = false;
         for (j=0; msg_types[j].name; j++) {
            if ((int)msg_types[j].token == i) {
               pm_strcat(hpkt.edbuf, "\"");
               pm_strcat(hpkt.edbuf, msg_types[j].name);
               pm_strcat(hpkt.edbuf, "\"");
               found = true;
               break;
            }
         }
         if (!found) {
            bjson_sendit(hpkt, "No find for type=%d\n", i);
         }
         count++;
      }
   }
   /*
    * Note, if we have more than half of the total items,
    *   redo using All and !item, which will give fewer items
    *   total.
    */
   if (count > M_MAX/2) {
      pm_strcpy(hpkt.edbuf, "[\"All\"");
      for (i=1; i<=M_MAX; i++) {
         if (!bit_is_set(i, dest->msg_types)) {
            found = false;
            for (j=0; msg_types[j].name; j++) {
               if ((int)msg_types[j].token == i) {
                  /* Do not display them, they are included in All */
                  if (msg_types[j].token != M_DEBUG &&
                      msg_types[j].token != M_EVENTS &&
                      msg_types[j].token != M_SAVED)
                  {
                     pm_strcat(hpkt.edbuf, ",");
                     pm_strcat(hpkt.edbuf, "\"!");
                     pm_strcat(hpkt.edbuf, msg_types[j].name);
                     pm_strcat(hpkt.edbuf, "\"");
                  }
                  found = true;
                  break;
               }
            }
            if (!found) {
               bjson_sendit(hpkt, "No find for type=%d in second loop\n", i);
            }
         } else if (i == M_SAVED) {
            /* Saved is not set by default, users must explicitly use it
             * on the configuration line
             */
            pm_strcat(hpkt.edbuf, ",\"Saved\"");

         } else if (i == M_EVENTS) {
            /* Events is not set by default, users must explicitly use it
             * on the configuration line
             */
            pm_strcat(hpkt.edbuf, ",\"Events\"");
         }
      }
   }
   /* Now handle custom type */
   edit_custom_type(&hpkt.edbuf, (MSGS *)hpkt.ritem->value, dest->msg_types);
   pm_strcat(hpkt.edbuf, "]");
}

/* -1 nothing displayed, 1 found, 0 not found */
int display_global_item(HPKT &hpkt)
{
   bool found = true;
   bool has_something = true;
   if (hpkt.ritem->handler == store_res) {
      display_res(hpkt);
   } else if (hpkt.ritem->handler == store_str ||
              hpkt.ritem->handler == store_name ||
              hpkt.ritem->handler == store_password ||
              hpkt.ritem->handler == store_strname ||
              hpkt.ritem->handler == store_dir) {
      display_string_pair(hpkt);
   } else if (hpkt.ritem->handler == store_int32 ||
              hpkt.ritem->handler == store_pint32 ||
              hpkt.ritem->handler == store_size32) {
      display_int32_pair(hpkt);
   } else if (hpkt.ritem->handler == store_size64 ||
              hpkt.ritem->handler == store_int64 ||
              hpkt.ritem->handler == store_time ||
              hpkt.ritem->handler == store_speed) {
      display_int64_pair(hpkt);
   } else if (hpkt.ritem->handler == store_bool) {
      display_bool_pair(hpkt);
   } else if (hpkt.ritem->handler == store_msgs) {
      has_something = display_msgs(hpkt);
   } else if (hpkt.ritem->handler == store_bit) {
      display_bit_pair(hpkt);
   } else if (hpkt.ritem->handler == store_alist_res) {
      has_something = display_alist_res(hpkt); /* In some cases, the list is null... */
   } else if (hpkt.ritem->handler == store_alist_str) {
      has_something = display_alist_str(hpkt); /* In some cases, the list is null... */
   } else {
      found = false;
   }

   if (found) {
      return has_something ? 1 : -1;
   } else {
      return 0;
   }
}

/*
 * Called here for each store_msgs resource
 */
bool display_msgs(HPKT &hpkt)
{
   MSGS *msgs = (MSGS *)hpkt.ritem->value;  /* Message res */
   DEST *dest;   /* destination chain */
   int first = true;

   if (!hpkt.in_store_msg) {
      hpkt.in_store_msg = true;
      bjson_sendit(hpkt, "\n    \"Destinations\": [");
   }
   for (dest=msgs->dest_chain; dest; dest=dest->next) {
      if (dest->dest_code == hpkt.ritem->code) {
         if (!first) bjson_sendit(hpkt, ",");
         first = false;
         edit_msg_types(hpkt, dest);
         switch (hpkt.ritem->code) {
         /* Output only message types */
         case MD_STDOUT:
         case MD_STDERR:
         case MD_SYSLOG:
         case MD_CONSOLE:
         case MD_CATALOG:
            bjson_sendit(hpkt, "\n      {\n        \"Type\": \"%s\","
                         "\n        \"MsgTypes\": %s\n      }",
               hpkt.ritem->name, hpkt.edbuf);
            break;
         /* Output MsgTypes, Where */
         case MD_DIRECTOR:
         case MD_FILE:
         case MD_APPEND:
            bjson_sendit(hpkt, "\n      {\n        \"Type\": \"%s\","
                         "\n        \"MsgTypes\": %s,\n",
               hpkt.ritem->name, hpkt.edbuf);
            bjson_sendit(hpkt, "        \"Where\": [%s]\n      }",
               quote_where(hpkt.edbuf, dest->where));
            break;
         /* Now we edit MsgTypes, Where, and Command */
         case MD_MAIL:
         case MD_OPERATOR:
         case MD_MAIL_ON_ERROR:
         case MD_MAIL_ON_SUCCESS:
            bjson_sendit(hpkt, "\n      {\n        \"Type\": \"%s\","
                         "\n        \"MsgTypes\": %s,\n",
               hpkt.ritem->name, hpkt.edbuf);
            bjson_sendit(hpkt, "        \"Where\": [%s],\n",
               quote_where(hpkt.edbuf, dest->where));
            bjson_sendit(hpkt, "        \"Command\": %s\n      }",
               quote_string(hpkt.edbuf, dest->mail_cmd));
            break;
         default:
            Dmsg1(50, "got %d\n", hpkt.ritem->code);
         }
      }
   }
   return (first == false);      // We found nothing to display
}

/*
 * Called here if the ITEM_LAST is set in flags,
 *  that means there are no more items to examine
 *  for this resource and that we can close any
 *  open json list.
 */
void display_last(HPKT &hpkt)
{
   if (hpkt.in_store_msg) {
      hpkt.in_store_msg = false;
      bjson_sendit(hpkt, "\n    ]");
   }
}

void display_alist(HPKT &hpkt)
{
   edit_alist(hpkt);
   bjson_sendit(hpkt, "%s", hpkt.edbuf);
}

bool display_alist_str(HPKT &hpkt)
{
   hpkt.list = (alist *)(*(hpkt.ritem->value));
   if (!hpkt.list) {
      return false;
   }
   bjson_sendit(hpkt, "\n    \"%s\":", hpkt.ritem->name);
   display_alist(hpkt);
   return true;
}

bool display_alist_res(HPKT &hpkt)
{
   bool f = true;
   alist *list;
   RES *res;

   list = (alist *)(*(hpkt.ritem->value));
   if (!list) {
      return false;
   }
   bjson_sendit(hpkt, "\n    \"%s\":", hpkt.ritem->name);
   bjson_sendit(hpkt, " [");
   foreach_alist(res, list) {
      if (!f) {
         bjson_sendit(hpkt, ", ");
      }
      bjson_sendit(hpkt, "%s", quote_string(hpkt.edbuf, res->name));
      f = false;
   }
   bjson_sendit(hpkt, "]");
   return true;
}

void display_res(HPKT &hpkt)
{
   RES *res;

   res = (RES *)*hpkt.ritem->value;
   bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
      quote_string(hpkt.edbuf, res->name));
}

void display_string_pair(HPKT &hpkt)
{
   bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
      quote_string(hpkt.edbuf, *hpkt.ritem->value));
}

void display_int32_pair(HPKT &hpkt)
{
   char ed1[50];
   bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
      edit_int64(*(int32_t *)hpkt.ritem->value, ed1));
}

void display_int64_pair(HPKT &hpkt)
{
   char ed1[50];
   bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
      edit_int64(*(int64_t *)hpkt.ritem->value, ed1));
}

void display_bool_pair(HPKT &hpkt)
{
   bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
      ((*(bool *)(hpkt.ritem->value)) == 0)?"false":"true");
}

void display_bit_pair(HPKT &hpkt)
{
   bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
      ((*(uint32_t *)(hpkt.ritem->value) & hpkt.ritem->code)
         == 0)?"false":"true");
}

bool byte_is_set(char *byte, int num)
{
   int i;
   bool found = false;
   for (i=0; i<num; i++) {
      if (byte[i]) {
         found = true;
         break;
      }
   }
   return found;
}

void display_bit_array(HPKT &hpkt, char *array, int num)
{
   int i;
   bool first = true;
   bjson_sendit(hpkt, " [");
   for (i=0; i<num; i++) {
      if (bit_is_set(i, array)) {
         if (!first) bjson_sendit(hpkt, ", ");
         first = false;
         bjson_sendit(hpkt, "%d", i);
      }
   }
   bjson_sendit(hpkt, "]");
}

void display_collector_types(HPKT &hpkt)
{
   int i;
   for (i=0; collectortypes[i].type_name; i++) {
      if (*(int32_t *)(hpkt.ritem->value) == collectortypes[i].coll_type) {
         bjson_sendit(hpkt, "\n    \"%s\": %s", hpkt.ritem->name,
            quote_string(hpkt.edbuf, collectortypes[i].type_name));
         return;
      }
   }
}
