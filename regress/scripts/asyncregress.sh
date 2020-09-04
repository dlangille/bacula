#!/bin/sh

# when working with multiple regress directory, 
# sync the local directory using ${BACULA_SOURCE}/../regress

set -e

. ./config

if [ -d ../bacula/src ] ; then
   echo "You are not in a remote regress directory"
   exit 1
fi

regress_dir="${BACULA_SOURCE}/../regress"
for d in scripts tests win32 ; do
   src="${regress_dir}/../regress/${d}"
   if [ ! -d ${src} ] ; then
      echo "source directory not found, skip: ${src}"
      continue
   fi
   if [ ! -d ${d} ] ; then
      echo "target directory not found, skip: ${d}"
      continue
   fi
   rsync -av ${src}/ ${d}/ 
done
# rsync the top of regress/ directory
rsync -dlptgoDv --exclude config ${regress_dir}/* .

