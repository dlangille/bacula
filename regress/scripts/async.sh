#!/bin/sh

set -e

. ./config

die()
{
  echo $1
  exit 1
}

check_bin()
{
   bin="$1"
   [ -f ${bin}/bacula-fd ] && [ -f ${bin}/bacula-sd ] && [ -f ${bin}/bacula-dir ]
}

build()
{
   rm -f bin/bacula-fd build/src/filed/bacula-fd bin/bacula-sd build/src/filed/bacula-sd bin/bacula-dir build/src/filed/bacula-dir
   cd build
   make ${MAKEOPT} || exit 1
   make ${MAKEOPT} install || exit 1
   if [ x$FORCE_DEDUP = xyes ]; then
      make -C src/stored install-dedup || exit 1
   fi
   [ -f ../bin/bacula-fd ] || die "bacula-fd missing"
   [ -f ../bin/bacula-sd ] || die "bacula-sd missing"
   [ -f ../bin/bacula-dir ] || die "bacula-dir missing"

   # wait # bin/bacula stop
   # get all tools -- especially testls
   cd src/tools
   make installall || die "make installall tools failed"


   cd ${cwd}
   if [ x${SMTP_HOST} = x -o x${SMTP_HOST} = xdummy ]; then
      # Turn off email
      cp scripts/dummy_bsmtp bin/bsmtp
      chmod 755 bin/bsmtp
   fi
}

cwd=`pwd`
[ -d ${BACULA_SOURCE} ] || die "The BACULA_SOURCE environment variable must be a Bacula release directory, but is not."

MAKEOPTS=${MAKEOPT:-"-j3"}

if [ -x bin/bacula ] ; then
   bin/bacula stop -KILL &
fi

#rsync -av ${BACULA_SOURCE}/ build/ --exclude examples/ --exclude patches/ --exclude src/win32 || die "rsync failed"
rsync -av --stats --exclude examples/ --exclude patches/ --exclude src/win32 --exclude src/config.h --exclude src/hosts.h --exclude src/version.h --include '*/' --include "*.c" --include "*.cc" --include '*.cpp' --include '*.h' --exclude '*' ${BACULA_SOURCE}/ build/ > tmp/rsync.log 2>&1 || die "rsync failed"

if grep "Number of files transferred: 0" tmp/rsync.log > /dev/null && check_bin bin; then
   echo "No Changes in sources"
else 
   build
   echo "Done"
fi
# wait for end of bacula stop above
wait 

exit 0
