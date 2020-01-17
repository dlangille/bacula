#!/bin/bash
#
#    Bacula® - The Network Backup Solution
#
#    Copyright (C) 2007-2017 Bacula Systems SA
#    All rights reserved.
#
#    The main author of Bacula is Kern Sibbald, with contributions from many
#    others, a complete list can be found in the file AUTHORS.
#
#    Licensees holding a valid Bacula Systems SA license may use this file
#    and others of this release in accordance with the proprietary license
#    agreement provided in the LICENSE file.  Redistribution of any part of
#    this release is not permitted.
#
#    Bacula® is a registered trademark of Kern Sibbald.
#
#
# This is an Inteos regression tests support utilities.
# Author: Radoslaw Korzeniewski, radekk@inteos.pl, Inteos Sp. z o.o.
#

#
# the support procedures requires a following variables for proper usage:
#   $JobName - a Jobname prefix to execute
#   $tmp - directory for temporary files - set by scripts/functions
#   $CLIENT - test backup client name - set by scripts/functions
#   $JOBID - a sequential number corresponding to executed job
#   $FileSetName - a FileSet prefix for restore job to execute
#

# now check if we are running under a proper shell
if test "x$SHELL" != "x/bin/bash"
then
	echo "Regression script must use BASH for this utilities!"
	exit 1
fi

#
# setup LPLUG variable used in listing tests
#
setup_plugin_param()
{
LPLUG=$1
if [ "x$debug" != "x" ]
then
	LPLUG="$LPLUG debug=1"
fi
export LPLUG
}

#
# common check if test pass by checking input param
#
# in:
# $1 - if variable is equal to zero ("0") then test pass else fail
#
regress_test_result()
{
if [ $1 -ne 0 ]
then
   echo "failed"
else
   echo "ok"
fi
}

#
# simplify unittest execution
#
# in:
# $1 - test name to execute - the unittest binary name
# $2 - test binary directory location
#
do_regress_unittest()
{
. scripts/functions
tname=$1
tdirloc=$2
make -C ${src}/${tdirloc} ${tname}
${src}/${tdirloc}/${tname}
exit $?
}

#
# do simple backup job test
#   generate ${tmp}/blog${ltest}.out job output messages logfile
#
# in:
#	$1 - a test number to perform which means it execute a job=${JobName}${1}
#
do_regress_backup_test()
{
ltest=$1
blevel="full"
if [ "x$2" != "x" ]
then
	blevel=$2
fi
printf "     backup test${ltest} ... "
cat << END_OF_DATA >${tmp}/bconcmds
@output /dev/null
messages
@$out ${tmp}/blog${ltest}.out
status client=${CLIENT}
setdebug level=500 client=${CLIENT} trace=1
run job=${JobName}${ltest} level=${blevel} storage=File1 yes
wait
status client=${CLIENT}
messages
setdebug level=0 trace=0 client=${CLIENT}
llist jobid=${JOBID}
list files jobid=${JOBID}
quit
END_OF_DATA
run_bconsole
((JOBID++))
}

#
# do simpe estimation listing test
#   generate ${tmp}/elog${ltest}.out job output messages logfile
#
# in:
# $1 - a test number to perform which means it execute a job=${JobName}${1}
#
do_regress_estimate_test()
{
ltest=$1
printf "     estimate test${ltest} ... "
cat << END_OF_DATA > ${tmp}/bconcmds
#@output /dev/null
messages
@$out ${tmp}/elog${ltest}.out
setdebug level=150 client=${CLIENT} trace=1
estimate listing job=$JobName fileset=${FileSetName}${ltest} level=Full
messages
setdebug level=50 trace=0 client=${CLIENT}
quit
END_OF_DATA
run_bconsole
}

#
# do plugin listing test
#   generate ${tmp}/llog${ltest}.out output listing logfile
#
# in:
# $1 - a test number to perform
# $2 - a path=... parameter for listing command
# $3 - custom $LPLUG plugin reference - optional, uses exported LPLUG variable
#
do_regress_listing_test()
{
ltest=$1
lpath=$2
lplug=$LPLUG
if [ "x$3" != "x" ]
then
	lplug=$3
fi
printf "     listing test${ltest} ... "
cat << END_OF_DATA >${tmp}/bconcmds
#@output /dev/null
messages
@$out ${tmp}/llog${ltest}.out
setdebug level=150 client=${CLIENT} tracee=1
.ls client=${CLIENT} plugin="$lplug" path=${lpath}
setdebug level=50 trace=0 client=${CLIENT}
quit
END_OF_DATA
run_bconsole
}

#
# do standard restore test
#   generate ${tmp}/rlog${ltest}.out job output messages logfile
#
# in:
# $1 - a test number to perform
# $2 - a FileSet number to use for restore test as ${FileSetName}${fs}
# $3 - a where=... restore parameter
# $4 - a restore selection command, optional. you can use strings like:
#     "file=..." or "select all"
#
do_regress_restore_test()
{
ltest=$1
fs=$2
where=$3
file=$4
cmd="restore fileset=${FileSetName}${fs} where=$3"
if [ "x$file" != "x" ]
then
   cmd="${cmd} $file"
fi
cmd="${cmd} storage=File1 done"
printf "     restore test${ltest} ... "
cat << END_OF_DATA >${tmp}/bconcmds
messages
@$out ${tmp}/rlog${ltest}.out
setdebug level=500 client=${CLIENT} trace=1
${cmd}
yes
wait
setdebug level=0 client=${CLIENT} trace=0
messages
llist jobid=${JOBID}
quit
END_OF_DATA
run_bconsole
((JOBID++))
}

#
# check the expected backup job execution status based on logfile
#   check for status Successful
#
# in:
# $1 - a test number to examine which means we will check blog${ltest}.out logfile
#
check_regress_backup_statusT()
{
ltest=$1
RET=`grep "jobstatus: " ${tmp}/blog${ltest}.out | awk '{print $2}'`
ERRS=$((`grep "joberrors: " ${tmp}/blog${ltest}.out | awk '{print $2}'`+0))
if [ "x$RET" != "xT" -o $ERRS -ne 0 ]
then
   ((bstat++))
   return 1
else
   return 0
fi
}

#
# check the expected backup job execution status based on logfile
#   check for status Warning
#
# in:
# $1 - a test number to examine which means we will check blog${ltest}.out logfile
#
check_regress_backup_statusW()
{
ltest=$1
RET=`grep "jobstatus: " ${tmp}/blog${ltest}.out | awk '{print $2}'`
ERRS=$((`grep "joberrors: " ${tmp}/blog${ltest}.out | awk '{print $2}'`+0))
if [ "x$RET" != "xT" -o $ERRS -eq 0 ]
then
   ((bstat++))
   return 1
else
   return 0
fi
}

#
# check the expected backup job execution status based on logfile
#   check for status Error or Fatal error
#
# in:
# $1 - a test number to examine which means we will check blog${ltest}.out logfile
#
check_regress_backup_statusE()
{
ltest=$1
RET=`grep "jobstatus: " ${tmp}/blog${ltest}.out | awk '{print $2}'`
if [ "x$RET" != "xf" -a "x$RET" != "xE" ]
then
   ((bstat++))
   return 1
else
   return 0
fi
}

#
# check the expected restore job execution status based on logfile
#   check for status Successful
#
# in:
# $1 - a test number to examine which means we will check rlog${ltest}.out logfile
#
check_regress_restore_statusT()
{
ltest=$1
RET=`grep "jobstatus: " ${tmp}/rlog${ltest}.out | awk '{print $2}'`
ERRS=$((`grep "joberrors: " ${tmp}/rlog${ltest}.out | awk '{print $2}'`+0))
if [ "x$RET" != "xT" -o $ERRS -ne 0 ]
then
   ((rstat++))
   return 1
else
   return 0
fi
}
