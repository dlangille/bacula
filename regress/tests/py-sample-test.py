#!/usr/bin/env python3
from regress import blab

lab=blab.Lab(testname='py-sample-test', shell=True)
lab.Shell('scripts/copy-confs')
lab.Shell('start_test')
lab.StartBacula()

lab.BconsoleScript("""\
@{out} /dev/null
messages
@{out} {tmp}/log1.out
setdebug level=4 storage=File1
label volume=TestVolume001 storage=File1 pool=File slot=1 drive=0
show job=BackupClient1
run job=BackupClient1 yes
@sleep 1
status storage=File1
@sleep 1
status storage=File1
wait
messages
@#
@# now do a restore
@#
@{out} {tmp}/log2.out
setdebug level=4 storage=File1
restore where={tmp}/bacula-restores select all done
yes
wait
messages
quit
""")

lab.Shell('check_for_zombie_jobs storage=File1')
lab.Shell('stop_bacula')

lab.Shell('check_two_logs')
lab.Shell('check_restore_diff')
lab.Shell('end_test')
lab.shell.Close()
