#!/bin/sh
#
#   Bacula® - The Network Backup Solution
#
# Copyright (C) 2000-2020 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#
#   Bacula(R) is a registered trademark of Kern Sibbald.
#
# This is a Bacula archive tool for backup/restore files on Docker volumes.
# Author: Radosław Korzeniewski, radekk@inteos.pl, Inteos Sp. z o.o.
#
echo "This script will build a custom static BaculaTar archive!"
echo
echo "To doing so it needs a full C building toolchain, upx executable compression library"
echo "and full Internet access for downloading required source code and dependencies."
echo "You should provide it for successful build."
echo
echo "When ready - hit enter -"
if [ "x$1" != "xyes" ]
then
   read a
fi
rm -rf archbuild
git clone https://github.com/ebl/tar-static.git archbuild
cd archbuild
./build.sh
cd ..
cp archbuild/releases/tar .
rm -rf archbuild
#D=`date +%d%b%y`
D=`grep DOCKER_TAR_IMAGE ../../../../version.h | awk '{print $3}' | sed 's/"//g'`
docker build -t baculatar:$D .
docker tag baculatar:$D baculatar:latest
docker save -o baculatar-$D.docker.tar baculatar:latest
