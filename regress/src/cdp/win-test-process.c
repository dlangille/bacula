#include "bacula.h"

static BOOL
file_size (HANDLE h, DWORD * lower, DWORD * upper)
{
  *lower = GetFileSize (h, upper);
  return 1;
}

/* LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems. */
# ifndef LOCKFILE_FAIL_IMMEDIATELY
#  define LOCKFILE_FAIL_IMMEDIATELY 1
# endif

/* Acquire a lock. */
static BOOL
do_lock (HANDLE h, int non_blocking, int exclusive)
{
  BOOL res;
  DWORD size_lower, size_upper;
  OVERLAPPED ovlp;
  int flags = 0;

  /* We're going to lock the whole file, so get the file size. */
  res = file_size (h, &size_lower, &size_upper);
  if (!res)
    return 0;

  /* Start offset is 0, and also zero the remaining members of this struct. */
  memset (&ovlp, 0, sizeof ovlp);

  if (non_blocking)
    flags |= LOCKFILE_FAIL_IMMEDIATELY;
  if (exclusive)
    flags |= LOCKFILE_EXCLUSIVE_LOCK;

  return LockFileEx (h, flags, 0, size_lower, size_upper, &ovlp);
}

/* Unlock reader or exclusive lock. */
static BOOL
do_unlock (HANDLE h)
{
  int res;
  DWORD size_lower, size_upper;

  res = file_size (h, &size_lower, &size_upper);
  if (!res)
    return 0;

  return UnlockFile (h, 0, 0, size_lower, size_upper);
}

int main(int argc, char *argv[])
{
   printf("Created Child Process\n");
   FILE *fp = bfopen(argv[1], "r");

   if (!fp) {
      printf("ERROR: could not open file %s\n", argv[1]);
      exit(-1);
   }

   int fd = fileno(fp);
   HANDLE handle = (HANDLE) _get_osfhandle (fd);
   
   if (handle == INVALID_HANDLE_VALUE) {
      printf("ERROR: could not get File Handle: %s\n", argv[1]);
      exit(-1);
   }

   if (!do_lock(handle, 1, 1)) {
      printf("ERROR: could not lock file: %s\n", argv[1]);
      exit(-1);
   }

   printf("CHILD  - got lock for Journal File\n");
   sleep(3);

   if (!do_unlock(handle)) {
      printf("ERROR: could not lock file: %s\n", argv[1]);
      exit(-1);
   }

   printf("CHILD  - released lock for Journal File\n");
   fclose(fp);
   return 0;
}
