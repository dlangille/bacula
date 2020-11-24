#!/bin/sh

export REGRESS_DEBUG=1
. scripts/functions

TestName="cdp-client-tests.sh"
Path=${cwd}
CdpClientPath=$Path/build/src/tools/cdp-client/cdp-client
TmpDir="$Path/tmp"
SpoolDir=$TmpDir/"spool-dir"
JournalFile=$TmpDir/".bcdp-cli.journal"
WatchedDir1=$TmpDir/"folder1"
WatchedDir2=$TmpDir/"folder2"

make -C $Path/build/src/tools/cdp-client/ install > /dev/null

start_test

run_cdp_client()
{
Args="-d 999 -s $SpoolDir -j $TmpDir -f $WatchedDir1 -f $WatchedDir2"
   if [ -f "$CdpClientPath" ]
   then
       $CdpClientPath $Args > $TmpDir/"cdp.log" &
       cdppid=$!
       trap "kill $cdppid" EXIT TERM INT
   else
      print_debug "CDP Client not found: $CdpClientPath. Please, make the binary."
   fi
sleep 0.4 
}

mkfile_and_wait()
{
echo 'abcde' > $1
sleep 0.4
}

mkdir_and_wait()
{
mkdir -p $1
sleep 0.4
}

touch_and_wait()
{
touch $1
sleep 0.4
}

cp_and_wait()
{
cp $1 $2
sleep 0.4
}

mv_and_wait()
{
mv $1 $2
sleep 0.4
}

verify_spooldir_contains_file()
{
print_debug "Verifying if Spool Dir contains: $1"
FoundFile=$(find $SpoolDir | grep -o "$1" | wc -l)
if [ "$FoundFile" -eq "1" ]; then
   print_debug "OK"
else
    print_debug "FAILED"
    estat=1
fi
}

verify_journal_contains_record()
{
print_debug "Verifying if Journal File contains File Record"
JournalHasFile=$(cat $JournalFile | grep "name=$1" | wc -l)
if [ "$JournalHasFile" -eq "1" ]; then
   print_debug "OK"
else
   print_debug "FAILED"
   estat=1
fi
}

rm -rf $TmpDir
mkdir $TmpDir
mkdir $WatchedDir1
mkdir $WatchedDir2
SedTestFile=$WatchedDir2/"sed_test.txt"
mkfile_and_wait $SedTestFile
TouchTestFile=$WatchedDir2/"touch_test.txt"

run_cdp_client

print_debug "TEST 1: CDP Client should detect creation of a new file:"

TestFile1=$WatchedDir1/"test1.txt"
print_debug "EXAMPLE 1: Creation of file $TestFile1"
mkfile_and_wait $TestFile1
verify_spooldir_contains_file "[0-9]\{10\}_test1.txt"
verify_journal_contains_record "$TestFile1"

TestFile2=$WatchedDir1/"test2.txt"
print_debug "EXAMPLE 2: Creation of file $TestFile2"
mkfile_and_wait $TestFile2
verify_spooldir_contains_file "[0-9]\{10\}_test2.txt"
verify_journal_contains_record "$TestFile2"

NewPath=$WatchedDir2/"my/new/path"
TestFile3=$NewPath/"test3.txt"
print_debug "EXAMPLE 3: Creation of file $TestFile3 after creation of new path $NewPath"

mkdir_and_wait $NewPath
mkfile_and_wait $TestFile3
verify_spooldir_contains_file "[0-9]\{10\}_test3.txt"
verify_journal_contains_record "$TestFile3"

TestFileCopy=$WatchedDir2/"test1_copy.txt"
print_debug "EXAMPLE 4: Copy of file $TestFile1 into $TestFileCopy"
cp_and_wait $TestFile1 $TestFileCopy
verify_spooldir_contains_file "[0-9]\{10\}_test1_copy.txt"
verify_journal_contains_record $TestFileCopy

print_debug "---------------------------------------------------------------------------"
print_debug "TEST 2: CDP Client should detect file changes:"

TestFile1=$WatchedDir2/"test_mv.gif"
TestFileMove=$WatchedDir1/"test_mv_renamed.gif"
print_debug "EXAMPLE 1: Move file $TestFile1 into $TestFileMove"
mkfile_and_wait $TestFile1
mv_and_wait $TestFile1 $TestFileMove
verify_spooldir_contains_file "[0-9]\{10\}_test_mv_renamed.gif"
verify_journal_contains_record $TestFileMove

print_debug "EXAMPLE 2: Sed file $SedTestFile"
sed -i "$ a #newline" $SedTestFile
sleep 0.4
verify_spooldir_contains_file "[0-9]\{10\}_sed_test.txt"
verify_journal_contains_record $SedTestFile

print_debug "EXAMPLE 3: Touch on file $TouchTestFile"
mkfile_and_wait $TouchTestFile
touch_and_wait $TouchTestFile
verify_spooldir_contains_file "[0-9]\{10\}_touch_test.txt"
verify_journal_contains_record $TouchTestFile 

end_test
