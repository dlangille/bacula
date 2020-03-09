/*
   Bacula® - The Network Backup Solution

   Copyright (C) 2000-2020 Bacula Systems SA
   All rights reserved.

   The main author of Bacula is Kern Sibbald, with contributions from many
   others, a complete list can be found in the file AUTHORS.

   Licensees holding a valid Bacula Systems SA license may use this file
   and others of this release in accordance with the proprietary license
   agreement provided in the LICENSE file.  Redistribution of any part of
   this release is not permitted.

   Bacula® is a registered trademark of Kern Sibbald.
*/

#include "bacula.h"
#include "../lib/unittests.h"

int main(int argc, char **argv)
{
   Unittests u("t");
   char buf[512] = {0};
   BPIPE *bp;
   FILE *fp;
   fp = fopen("toto.sh", "w");
   fprintf(fp, "#!/bin/sh\necho hello\nread a\necho $a\n");
   fclose(fp);
   chmod("toto.sh", 0755);
   
   bp = open_bpipe("./toto.sh", 0, "rw");
   ok(bp != NULL, "open_bpipe");

   fgets(buf, sizeof(buf), bp->rfd);
   ok(strcmp(buf, "hello\n") == 0, "fgets on bpipe");
   printf("%s", buf);

   fprintf(bp->wfd, "titi\n");
   fflush(bp->wfd);

   fgets(buf, sizeof(buf), bp->rfd);
   ok(strcmp(buf, "titi\n") == 0, "fgets on bpipe");
   printf("%s", buf);
   
   ok(close_bpipe(bp) == 0, "closing bpipe");
   return report();
}
