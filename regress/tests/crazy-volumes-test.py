#!/usr/bin/env python3
#
# Copyright (C) 2000-2020 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#
# This test create crazy volumes and most backups cannot be restored
# Create 2 volumes with a fixed MaxVolBytes
# Start a lot of backup in //
# When both volumes are full increate the size of the 1st one and
# change its volstatus to "Append", when full to the same with the other
# and then back to the first one until all backups are done
#
# I think bacula is not creating all required jobmedia to switch to the
# appropriate volume when required for the restore
# for example, you have volumes V1 & V2
# I will enumerate backup with a letter and block number with a number.
# A1 is the first block of backup A and B2 the second block of backup B ...
#
# Sometime we got something like this
#
# V1 V2
# ----------
# A1 B1
# B2 A2
# A3 B3
#
# The restore of backup A, will read A1 and A3 from
# the first volume before A2 in the second one.
# If A1 and A3 can fit together, bacula will even not
# notice the problem and the restored data are corrupted
# If A1 ends with a split record
# that don't fit with A3 then bacula returns an error :
# 127.0.0.1-sd JobId 22: Fatal error: read.c:175 Bad reference, size=64

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

numbackup=10
maxvolbytes=2*1024**2

lab=blab.Lab(testname='crazy-volumes-test', profile='dedup-simple', shell=True)

lab.StartTest()

if lab.GetVar('FORCE_DEDUP') in ( 'yes', '1', 'on'):
    jobsize=500
    jobstart=100
else:
    jobsize=10
    jobstart=10

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
# Create 2 volumes
#
volname1='TestVolume001'
volname2='TestVolume002'
for volname in [volname1, volname2 ]:
    # create a volume and make a backup of it (to replace it later)
    lab.BconsoleScript('label storage={STORAGE} volume=%s\n' % (volname,))
    lab.BconsoleScript('update volume=%s MaxVolBytes=%d\n' % (volname, maxvolbytes, ))

ctx=[ False, volname1, volname2, maxvolbytes, maxvolbytes ]

def EnlargeVolume(ctx):
    """Automatically enlarge volumes when needed"""
    lab.Log(logging.INFO, 'thread EnlargeVolume Started %r', ctx)
    _stop, volname1, volname2, size, inc_size=ctx
    # volume2 is the last used one, must write again on volume1
    vol1=lab.GetVolume(volname1)
    vol2=lab.GetVolume(volname2)
    state="waiting_msg"
    while not ctx[0]:
        if state=="waiting_msg":
            returncode, out, err=lab.BconsoleScriptOut('status storage={STORAGE}\n')
            if 'Device is BLOCKED waiting to create a volume' in out:
                if vol1['name']<vol2['name']:
                    # increment only ones over two
                    size+=inc_size
                vol_size=os.path.getsize(vol1['path'])
                lab.Log(logging.INFO, '******* Device is BLOCKED vol=%s sz=%d', vol1['name'], size)
                lab.BconsoleScript('update volume=%s MaxVolBytes=%d\n' % (vol1['name'], size))
                lab.BconsoleScript('update volume=%s volstatus=Append\n' % (vol1['name'],))
                lab.BconsoleScript('mount storage={STORAGE} drive=0 slot=0\n')
                # swap vol1 & vol2
                vol1, vol2=vol2, vol1
                state="waiting_vol_grow"
            else:
                time.sleep(5)
            continue

        if state=="waiting_vol_grow":
            new_size=os.path.getsize(vol2['path'])
            if vol_size!=new_size:
                lab.Log(logging.INFO, '******* Volsize changed vol=%s oldsize=%d newsize=%d', vol2['name'], vol_size, new_size)
                vol_size=new_size
                state="waiting_vol_stable"
                # I could go directly to state="waiting_msg" but prefere to wait
                # to have a stable volsize, to avoid to much call to bconsole
            else:
                time.sleep(0.1)
            continue

        if state=="waiting_vol_stable":
            time.sleep(1.0)
            new_size=os.path.getsize(vol2['path'])
            if vol_size==new_size:
                lab.Log(logging.INFO, '******* Volsize stable vol=%s size=%d', vol2['name'], new_size)
                state="waiting_msg"
                time.sleep(3.0) # let bacula know, try to avoid bconsole use
            else:
                vol_size=new_size
                time.sleep(0.1)
            continue

    lab.Log(logging.INFO, 'thread EnlargeVolume ended')

# Start the thread that enlorge volume
thread_secondrun=threading.Thread(target=EnlargeVolume, args=(ctx, ))
thread_secondrun.start()

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

# stop and wait for the thread
ctx[0]=True
thread_secondrun.join()

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
lab.Log(logging.ERROR, 'This test is not supposed to succeed, until bacula can restore from "tortured" volumes')
lab.EndTest()
