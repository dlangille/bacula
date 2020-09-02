#!/usr/bin/env python3
#
# Copyright (C) 2000-2020 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#
# This test create lots of small volumes using MaxVolBytes, 
# start multiple backups in //
# and sometime some backups are restored with errors
import sys
import os
import re
import hashlib
import logging
import random
import shutil
import codecs
import threading
import time

from regress import blab

lab=blab.Lab(testname='crazy-volumes-test', profile='dedup-simple', shell=True)

lab.StartTest()

numbackup=20
if lab.GetVar('FORCE_DEDUP') in ( 'yes', '1', 'on'):
    jobsize=500
    jobstart=100
    vol_count=60
    maxvolbytes=2*1024**2
else:
    jobsize=20
    jobstart=10
    vol_count=220
    maxvolbytes=2*1024**2

# create some data sample
for i in range(numbackup):
    sample=os.path.join(lab.vars.tmp, 'stream%d.dedup' % (i,))
    open(sample, 'wt').write("""\
global_size=10G
chunk_min_size=4K
chunk_max_size=6K
deviation=10
seed=1234
size=%dM
start=%dM
""" % (jobsize, jobstart*i))


#
# Create 200 small volumes
#
for voli in range(1, vol_count):
    volname='TestVolume%03d' % (voli, )
    lab.BconsoleScript('label storage={STORAGE} volume=%s\n' % (volname,))
    lab.BconsoleScript('update volume=%s MaxVolBytes=%d\n' % (volname, maxvolbytes, ))
# and a normal one
volname='TestVolume%03d' % (vol_count, )
lab.BconsoleScript('label storage={STORAGE} volume=%s\n' % (volname,))

lab.ShellOut('$tmp/select-cfg.sh 0')
#
# Do some backups
#
script="""\
@output /dev/null
messages
@{out} {cwd}/tmp/log1.out
@#setdebug level=100 client={CLIENT}
@#setdebug level=50 storage={STORAGE} tags=asx,dde
@#
"""
for i in range(numbackup):
    script+="""\
run job=DedupPluginTest level=Full storage={STORAGE} yes
@exec "{tmp}/select-cfg.sh next"
"""
script+="""\
wait
message
quit
"""
lab.BconsoleScript(script)

lab.BconsoleScript('list media\nquit\n')

#
# try some restore
#
script="""\
@output /dev/null
messages
@{out} {cwd}/tmp/log2.out
setdebug level=4 storage={STORAGE}
"""
for i in range(numbackup):
    script+="restore jobid=%d where={tmp}/bacula-restores all storage={STORAGE} done yes\n" % ( i+1, )
script+="""\
wait
message
quit
"""

lab.BconsoleScript(script)

# Vacuum
# lab.BconsoleScript('dedup vacuum storage={STORAGE} checkindex checkvolumes\n')

lab.Shell('check_for_zombie_jobs storage=${STORAGE}')
lab.Shell('check_two_logs')
lab.EndTest()
