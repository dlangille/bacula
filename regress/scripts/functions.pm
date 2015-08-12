################################################################
use strict;

=head1 LICENSE

   Bacula® - The Network Backup Solution

   Copyright (C) 2008-2014 Bacula Systems SA

   The main author of Bacula is Kern Sibbald, with contributions from
   many others, a complete list can be found in the file AUTHORS.

   Licensees holding a valid Bacula Systems SA license may use this file
   and others of this release in accordance with the proprietary license
   agreement provided in the LICENSE file.  Redistribution of any part of
   this release is not permitted.

   Bacula® is a registered trademark of Kern Sibbald.

=cut

package scripts::functions;
use File::Basename qw/basename/;
# Export all functions needed to be used by a simple 
# perl -Mscripts::functions -e '' script
use Exporter;
our @ISA = qw(Exporter);

our @EXPORT = qw(update_some_files create_many_files check_multiple_copies
                  update_client $HOST $BASEPORT add_to_backup_list
                  run_bconsole run_bacula start_test end_test create_bconcmds
                  create_many_dirs cleanup start_bacula
                  get_dirname
                  stop_bacula get_resource set_maximum_concurrent_jobs get_time
                  add_attribute check_prune_list check_min_volume_size
                  init_delta update_delta check_max_backup_size comment_out
                  create_many_files_size $plugins debug p
                  check_max_volume_size $estat $bstat $rstat $zstat $cwd $bin
                  $scripts $conf $rscripts $tmp $working $dstat extract_resource
                  $db_name $db_user $db_password $src $tmpsrc $out $CLIENT docmd
                  set_global_maximum_concurrent_jobs check_volumes update_some_files_rep
                  remote_init remote_config remote_stop remote_diff remote_check
                  get_field_size get_field_ratio create_binfile );


use File::Copy qw/copy/;

our ($cwd, $bin, $scripts, $conf, $rscripts, $tmp, $working, $estat, $dstat,
     $plugins, $bstat, $zstat, $rstat, $debug, $out, $TestName,
     $REMOTE_CLIENT, $REMOTE_ADDR, $REMOTE_FILE, $REMOTE_PORT, $REMOTE_PASSWORD,
     $REMOTE_STORE_ADDR, $REGRESS_DEBUG, $REMOTE_USER, $start_time, $end_time,
     $db_name, $db_user, $db_password, $src, $tmpsrc, $HOST, $BASEPORT, $CLIENT);

END {
    if ($estat || $rstat || $zstat || $bstat || $dstat) {
        exit 1;
    }
}

BEGIN {
    # start by loading the ./config file
    my ($envar, $enval);
    if (! -f "./config") {
        die "Could not find ./config file\n";
    }
    # load the ./config file in a subshell doesn't allow to use "env" to display all variable
    open(IN, ". ./config; set |") or die "Could not run shell: $!\n";
    while ( my $l = <IN> ) {
        chomp ($l);
        if ($l =~ /^([\w\d]+)='?([^']+)'?/) {
            next if ($1 eq 'SHELLOPTS'); # is in read-only
            ($envar,$enval) = ($1, $2);
            $ENV{$envar} = $enval;
        }
    }
    close(IN);
    $cwd = `pwd`; 
    chomp($cwd);

    # set internal variable name and update environment variable
    $ENV{db_name}     = $db_name     = $ENV{db_name}     || 'regress';
    $ENV{db_user}     = $db_user     = $ENV{db_user}     || 'regress';
    $ENV{db_password} = $db_password = $ENV{db_password} || '';

    $ENV{bin}      = $bin      =  $ENV{bin}      || "$cwd/bin";
    $ENV{tmp}      = $tmp      =  $ENV{tmp}      || "$cwd/tmp";
    $ENV{src}      = $src      =  $ENV{src}      || "$cwd/src";
    $ENV{conf}     = $conf     =  $ENV{conf}     || $bin;
    $ENV{scripts}  = $scripts  =  $ENV{scripts}  || $bin;
    $ENV{plugins}  = $plugins  =  $ENV{plugins}  || "$bin/plugins";
    $ENV{tmpsrc}   = $tmpsrc   =  $ENV{tmpsrc}   || "$cwd/tmp/build";
    $ENV{working}  = $working  =  $ENV{working}  || "$cwd/working";    
    $ENV{rscripts} = $rscripts =  $ENV{rscripts} || "$cwd/scripts";
    $ENV{HOST}     = $HOST     =  $ENV{HOST}     || "localhost";
    $ENV{BASEPORT} = $BASEPORT =  $ENV{BASEPORT} || "8101";
    $ENV{REGRESS_DEBUG} = $debug         = $ENV{REGRESS_DEBUG} || 0;
    $ENV{REMOTE_CLIENT} = $REMOTE_CLIENT = $ENV{REMOTE_CLIENT} || 'remote-fd';
    $ENV{REMOTE_ADDR}   = $REMOTE_ADDR   = $ENV{REMOTE_ADDR}   || undef;
    $ENV{REMOTE_FILE}   = $REMOTE_FILE   = $ENV{REMOTE_FILE}   || "/tmp";
    $ENV{REMOTE_PORT}   = $REMOTE_PORT   = $ENV{REMOTE_PORT}   || 9102;
    $ENV{REMOTE_PASSWORD} = $REMOTE_PASSWORD = $ENV{REMOTE_PASSWORD} || "xxx";
    $ENV{REMOTE_STORE_ADDR}=$REMOTE_STORE_ADDR=$ENV{REMOTE_STORE_ADDR} || undef;
    $ENV{REMOTE_USER}   = $REMOTE_USER   = $ENV{REMOTE_USER}   || undef;
    $ENV{CLIENT}        = $CLIENT        = $ENV{CLIENT}        || "$HOST-fd";
    $ENV{LANG} = 'C';
    $out = ($debug) ? '@tee' : '@out';

    $TestName = basename($0);

    $dstat = $estat = $rstat = $bstat = $zstat = 0;
}

# execute bconsole session
sub run_bconsole
{
    my $script = shift || "$tmp/bconcmds";
    return docmd("cat $script | $bin/bconsole -c $conf/bconsole.conf");
}

# create a file-list for many tests using
# <$cwd/tmp/file-list as fileset
sub add_to_backup_list
{
    open(FP, ">$tmp/file-list") or die "ERROR: Unable to open $tmp/file-list $@";
    foreach my $l (@_) {
        if ($l =~ /\n$/) {
            print FP $l;
        } else {
            print FP $l, "\n";
        }
    }
    close(FP);
}

sub cleanup
{
    system("$rscripts/cleanup");
    return $? == 0;
}

sub start_test
{
    $start_time = time();
    my $d = strftime('%R:%S', localtime($start_time));
    print "\n\n === Starting $TestName at $d ===\n";
}

sub end_test
{
    $end_time = time();
    my $t = strftime('%R:%S', localtime($end_time));
    my $d = strftime('%H:%M:%S', gmtime($end_time - $start_time));

    if ( -f "$tmp/err.log") {
        system("cat $tmp/err.log");
    }

    if ($estat != 0 || $zstat != 0 || $dstat != 0 || $bstat != 0 ) {
        print "
       !!!!! $TestName failed!!! $t $d !!!!!
         Status: estat=$estat zombie=$zstat backup=$bstat restore=$rstat diff=$dstat\n";

        if ($bstat != 0 || $rstat != 0) {
            print "     !!! Bad termination status       !!!\n";
        } else {
            print "     !!! Restored files differ        !!!\n";
        }
        print "     Status: backup=$bstat restore=$rstat diff=$dstat\n";
        print "     Test owner of $ENV{SITE_NAME} is $ENV{EMAIL}\n";
    } else {
        print "\n\n    === Ending $TestName at $t ($d) ===\n\n";
    }
}

# create a console command file, can handle a list
sub create_bconcmds
{
    open(FP, ">$tmp/bconcmds");
    map { print FP "$_\n"; } @_;
    close(FP);
}

# run a command
sub docmd
{
    my $cmd = shift;
    system("sh -c '$cmd " . (($debug)?"":" >/dev/null") . "'");
    return $? == 0;
}

sub start_bacula
{
    my $ret;
    $ret = docmd("$bin/bacula start");

    # cleanup bweb stuff
    create_bconcmds('@out /dev/null',
                    'sql',
                    'truncate client_group;',
                    'truncate client_group_member;',
                    'update Media set LocationId=0;',
                    'truncate location;',
                    '');
    run_bconsole();
    return $ret;
}

sub stop_bacula
{
    return docmd("$bin/bacula stop");
}

sub get_dirname
{
    my $ret = `$bin/bdirjson -c $conf/bacula-dir.conf -l Name -r Director`;
    if ($ret =~ /"Name": "(.+?)"/) {
        print "$1\n";
    }
}

sub get_resource
{
    my ($file, $type, $name) = @_;
    my $ret;
    open(FP, $file) or die "Can't open $file";
    my $content = join("", <FP>);
    
    if ($content =~ m/(^$type \{[^}]+?Name\s*=\s*"?$name"?[^}]+?^\})/ms) {
        $ret = $1;
    }

    close(FP);
    return $ret;
}

sub extract_resource
{
    my $ret = get_resource(@_);
    if ($ret) {
        print $ret, "\n";
    }
}

sub get_field_size
{
    my ($file, $field) = @_;
    my $size=0;

    my $pattern=$field."\\s*([\\d,]+)";
    open(FP, $file) or die "ERROR: Can't open $file";
    
    while (<FP>) {
        if (/$pattern/) { 
            $size=$1;
        }
    }

    close(FP);

    $size =~ s/,//g;

    print $size."\n";
}

sub get_field_ratio
{
    my ($file, $field) = @_;
    my $ret=0;
    my $ratio=0;

    my $pattern=$field."\\s*[\\d.]+%\\s+([\\d]+)\.[\\d]*:1"; # stop at the '.'
    my $pattern2=$field."\\s*None";
    open(FP, $file) or die "ERROR: Can't open $file";
    
    while (<FP>) {
        if (/$pattern/) { 
            $ratio=$1;
        }
        if (/$pattern2/) { 
            $ratio="None";
        }
    }

    close(FP);

    $ratio =~ s/,//g;

    print $ratio."\n";
}



sub check_max_backup_size
{
    my ($file, $size) = @_;
    my $ret=0;
    my $s=0;

    open(FP, $file) or die "ERROR: Can't open $file";
    
    while (<FP>) {

        if (/FD Bytes Written: +([\d,]+)/) { 
            $s=$1;
        }
    }

    close(FP);

    $size =~ s/,//g;

    if ($s > $size) { 
        print "ERROR: backup too big ($s > $size)\n";  
        $ret++;
    } else {
        print "OK\n";
    }
    return $ret;
}

sub check_min_volume_size
{
    my ($size, @vol) = @_;
    my $ret=0;

    foreach my $v (@vol) {
        if (! -f "$tmp/$v") {
            print "ERR: $tmp/$v not accessible\n";
            $ret++;
            next;
        }
        if (-s "$tmp/$v" < $size) {
            print "ERR: $tmp/$v too small\n";
            $ret++;
        }
    }
    $estat+=$ret;
    return $ret;
}

# check_volumes("tmp/log1.out", "tmp/log2.out", ...)
sub check_volumes
{
    my @files = @_;
    my %done;
    unlink("$tmp/check_volumes.out");

    foreach my $f (@files) {
        open(FP, $f) or next;
        while (my $f = <FP>)
        {
            if ($f =~ /Wrote label to prelabeled Volume "(.+?)" on file device "(.+?)" \((.+?)\)/) {
                if (!$done{$1}) {
                    $done{$1} = 1;
                    if (-f "$3/$1") {
                        system("$bin/bls -c $conf/bacula-sd.conf -j -E -V \"$1\" \"$2\" &>> $tmp/check_volumes.out");
                        if ($? != 0) {
                            debug("Found problems for $1, traces are in $tmp/check_volumes.out");
                            $estat = 1;
                        }
                    }
                }
            }
        }
        close(FP);
    }
    return $estat;
}

# check if a volume is too big
# check_max_backup_size(10000, "vol1", "vol3");
sub check_max_volume_size
{
    my ($size, @vol) = @_;
    my $ret=0;

    foreach my $v (@vol) {
        if (! -f "$tmp/$v") {
            print "ERR: $tmp/$v not accessible\n";
            $ret++;
            next;
        }
        if (-s "$tmp/$v" > $size) {
            print "ERR: $tmp/$v too big\n";
            $ret++;
        }
    }
    $estat+=$ret;
    return $ret;
}

# update client definition for the current test
# it permits to test remote client
sub update_client
{
    my ($new_passwd, $new_address, $new_port) = @_;
    my $in_client=0;

    open(FP, "$conf/bacula-dir.conf") or die "can't open source $!";
    open(NEW, ">$tmp/bacula-dir.conf.$$") or die "can't open dest $!";
    while (my $l = <FP>) {
        if (!$in_client && $l =~ /^Client \{/) {
            $in_client=1;
        }
        
        if ($in_client && $l =~ /Address/i) {
            $l = "Address = $new_address\n";
        }

        if ($in_client && $l =~ /FDPort/i) {
            $l = "FDPort = $new_port\n";
        }

        if ($in_client && $l =~ /Password/i) {
            $l = "Password = \"$new_passwd\"\n";
        }

        if ($in_client && $l =~ /^\}/) {
            $in_client=0;
        }
        print NEW $l;
    }
    close(FP);
    close(NEW);
    my $ret = copy("$tmp/bacula-dir.conf.$$", "$conf/bacula-dir.conf");
    unlink("$tmp/bacula-dir.conf.$$");
    return $ret;
}

# if you want to run this function more than 100 times, please, update this number
my $last_update = 100;

# open a directory and update all files
sub update_some_files_rep
{
    my ($dest, $nbupdate)=@_;
    my $t=rand();
    my $f;
    my $nb=0;
    my $nbdel=0;
    my $total=0;

    if ($nbupdate) {
        $last_update = $nbupdate;
        unlink("$tmp/last_update");

    } elsif (-f "$tmp/last_update") {
        $last_update = `cat $tmp/last_update`;
        chomp($last_update);
        $last_update--;
        if ($last_update == 0) {
            $last_update = 100;
        }
    }
    my $base = chr($last_update % 26 + 65); # We use a base directory A-Z

    system("sh -c 'echo $last_update > $tmp/last_update'");
    print "Update files in $dest\n";
    opendir(DIR, "$dest/$base") || die "$!";
    map {
        $f = "$dest/$base/$_";
        if (($total++ % $last_update) == 0) {
            if (-f $f) {
                # We delete some of them, and we replace them later
                if ((($nb + $nbdel) % 11) == 0) {
                    unlink($f);
                    $nbdel++;

                    open(FP, ">$dest/$base/$last_update-$nbdel.txt") or die "$f $!";
                    seek(FP, $last_update * 4000, 0);
                    print FP "$t update $f\n";
                    close(FP);

                } else {
                    open(FP, ">>$f") or die "$f $!";
                    print FP "$t update $f\n";
                    close(FP);
                    $nb++;
                }
            }
        }
    } sort readdir(DIR);
    closedir DIR;
    print "$nb files updated, $nbdel deleted/created\n";
}

# open a directory and update all files
sub update_some_files
{
    my ($dest)=@_;
    my $t=rand();
    my $f;
    my $nb=0;
    print "Update files in $dest\n";
    opendir(DIR, $dest) || die "$!";
    map {
        $f = "$dest/$_";
        if (-f $f) {
            open(FP, ">$f") or die "$f $!";
            print FP "$t update $f\n";
            close(FP);
            $nb++;
        }
    } readdir(DIR);
    closedir DIR;
    print "$nb files updated\n";
}

# create big number of files in a given directory
# Inputs: dest  destination directory
#         nb    number of file to create
# Example:
# perl -Mscripts::functions -e 'create_many_files("$cwd/files", 100000)'
# perl -Mscripts::functions -e 'create_many_files("$cwd/files", 100000, 32000)'
sub create_many_files
{
    my ($dest, $nb, $sparse_size) = @_;
    my $base;
    my $dir=$dest;
    $nb = $nb / 2;              # We create 2 files per loop
    $nb = $nb || 750000;
    $sparse_size = $sparse_size | 0;
    mkdir $dest;
    $base = chr($nb % 26 + 65); # We use a base directory A-Z

    # already done
    if (-f "$dest/$base/a${base}a${nb}aaa${base}") {
        debug("Files already created\n");
        return;
    }

    # auto flush stdout for dots
    $| = 1;
    print "Create ", $nb * 2, " files into $dest\n";
    for(my $i=0; $i < 26; $i++) {
        $base = chr($i + 65);
        mkdir("$dest/$base") if (! -d "$dest/$base");
    }
    for(my $i=0; $i<=$nb; $i++) {
        $base = chr($i % 26 + 65);
        open(FP, ">$dest/$base/a${base}a${i}aaa$base") or die "$dest/$base $!";
        if ($sparse_size) {
            seek(FP, $sparse_size + $i, 0);
        }
        print FP "$i\n";
        close(FP);
        
        open(FP, ">>$dir/b${base}a${i}csq$base") or die "$dir $!";
        print FP "$base $i\n";
        close(FP);
        
        if (!($i % 100)) {
            $dir = "$dest/$base/$base$i$base";
            mkdir $dir;
        }
        print "." if (!($i % 10000));
    }
    print "\n";
}

# BEEF
# create big number of files in a given directory
# Inputs: dest  destination directory
#         nb    number of file to create
# Example:
# perl -Mscripts::functions -e 'create_many_files_size("$cwd/files", 100000)'
sub create_many_files_size
{
    my ($dest, $nb) = @_;
    my $base;
    my $dir=$dest;
    $nb = $nb || 750000;
    mkdir $dest;
    $base = chr($nb % 26 + 65); # We use a base directory A-Z

    # already done
    if (-f "$dest/$base/a${base}a${nb}aaa${base}") {
        debug("Files already created\n");
        return;
    }

    # auto flush stdout for dots
    $| = 1;
    print "Create $nb files into $dest\n";
    for(my $i=0; $i < 26; $i++) {
        $base = chr($i + 65);
        mkdir("$dest/$base") if (! -d "$dest/$base");
    }
    for(my $i=0; $i<=$nb; $i++) {
        $base = chr($i % 26 + 65);
        open(FP, ">$dest/$base/a${base}a${i}aaa$base") or die "$dest/$base $!";
        print FP "$base" x $i;
        close(FP);
        
        print "." if (!($i % 10000));
    }
    print "\n";
}

# create big number of dirs in a given directory
# Inputs: dest  destination directory
#         nb    number of dirs to create
# Example:
# perl -Mscripts::functions -e 'create_many_dirs("$cwd/files", 100000)'
sub create_many_dirs
{
    my ($dest, $nb) = @_;
    my ($base, $base2);
    my $dir=$dest;
    $nb = $nb || 750000;
    mkdir $dest;
    $base = chr($nb % 26 + 65); # We use a base directory A-Z
    $base2 = chr(($nb+10) % 26 + 65);
    # already done
    if (-d "$dest/$base/$base2/$base/a${base}a${nb}aaa${base}") {
        debug("Files already created\n");
        return;
    }

    # auto flush stdout for dots
    $| = 1;
    print "Create $nb dirs into $dest\n";
    for(my $i=0; $i < 26; $i++) {
        $base = chr($i + 65);
        $base2 = chr(($i+10) % 26 + 65);
        mkdir("$dest/$base");
        mkdir("$dest/$base/$base2");
        mkdir("$dest/$base/$base2/$base$base2");
        mkdir("$dest/$base/$base2/$base$base2/$base$base2");
        mkdir("$dest/$base/$base2/$base$base2/$base$base2/$base2$base");
    }
    for(my $i=0; $i<=$nb; $i++) {
        $base = chr($i % 26 + 65);
        $base2 = chr(($i+10) % 26 + 65);
        mkdir("$dest/$base/$base2/$base$base2/$base$base2/$base2$base/a${base}a${i}aaa$base");  
        print "." if (!($i % 10000));
    }
    print "\n";
}

sub check_encoding
{
    if (grep {/Wanted SQL_ASCII, got UTF8/} 
        `${bin}/bacula-dir -d50 -t -c ${conf}/bacula-dir.conf 2>&1`)
    {
        print "Found database encoding problem, please modify the ",
              "database encoding (SQL_ASCII)\n";
        exit 1;
    }
}

sub set_global_maximum_concurrent_jobs
{
    my ($nb) = @_;
    add_attribute("$conf/bacula-dir.conf", "MaximumConcurrentJobs", $nb, "Job");
    add_attribute("$conf/bacula-dir.conf", "MaximumConcurrentJobs", $nb, "Client");
    add_attribute("$conf/bacula-dir.conf", "MaximumConcurrentJobs", $nb, "Director");
    add_attribute("$conf/bacula-dir.conf", "MaximumConcurrentJobs", $nb, "Storage");
    add_attribute("$conf/bacula-sd.conf", "MaximumConcurrentJobs", $nb, "Storage");
    add_attribute("$conf/bacula-sd.conf", "MaximumConcurrentJobs", $nb, "Device");
    add_attribute("$conf/bacula-fd.conf", "MaximumConcurrentJobs", $nb, "FileDaemon");
}

# You can change the maximum concurrent jobs for any config file
# If specified, you can change only one Resource or one type of
# resource at the time (optional)
#  set_maximum_concurrent_jobs('$conf/bacula-dir.conf', 100);
#  set_maximum_concurrent_jobs('$conf/bacula-dir.conf', 100, 'Director');
#  set_maximum_concurrent_jobs('$conf/bacula-dir.conf', 100, 'Device', 'Drive-0');
sub set_maximum_concurrent_jobs
{
    my ($file, $nb, $obj, $name) = @_;

    die "Can't get new maximumconcurrentjobs" 
        unless ($nb);

    add_attribute($file, "Maximum Concurrent Jobs", $nb, $obj, $name);
}

# You can comment out a directive
#  comment_out('$conf/bacula-dir.conf', 'FDTimeout', 'Job', 'test');
#  comment_out('$conf/bacula-dir.conf', 'FDTimeout');
sub comment_out
{
    my ($file, $attr, $obj, $name) = @_;
    my ($cur_obj, $cur_name, $done);

    open(FP, ">$tmp/1.$$") or die "Can't write to $tmp/1.$$";
    open(SRC, $file) or die "Can't open $file";
    while (my $l = <SRC>)
    {
        if ($l =~ /^#/) {
            print FP $l;
            next;
        }

        if ($l =~ /^(\w+) \{/) {
            $cur_obj = $1;
            $done=0;
        }

        if ($l =~ /^\s*\Q$attr\E/i) {
            if (!$obj || $cur_obj eq $obj) {
                if (!$name || $cur_name eq $name) {
                    $l =~ s/^/##/;
                    $done=1
                }
            }
        }

        if ($l =~ /^\s*Name\s*=\s*"?([\w\d\.-]+)"?/i) {
            $cur_name = $1;
        }
        print FP $l;
    }
    close(SRC);
    close(FP);
    copy("$tmp/1.$$", $file) or die "Can't copy $tmp/1.$$ to $file";
}

# You can add option to a resource
#  add_attribute('$conf/bacula-dir.conf', 'FDTimeout', 1600, 'Director');
#  add_attribute('$conf/bacula-dir.conf', 'FDTimeout', 1600, 'Storage', 'FileStorage');
sub add_attribute
{
    my ($file, $attr, $value, $obj, $name) = @_;
    my ($cur_obj, $cur_name, $done);

    my $is_options = $obj && $obj eq 'Options';
    open(FP, ">$tmp/1.$$") or die "Can't write to $tmp/1.$$";
    open(SRC, $file) or die "Can't open $file";
    while (my $l = <SRC>)
    {
        if ($l =~ /^#/) {
            print FP $l;
            next;
        }

        if ($l =~ /^(\w+) \{/  || ($is_options && $l =~ /\s+(Options)\s*\{/)) {
            $cur_obj = $1;
            $done=0;
        }

        if ($l =~ /^\s*\Q$attr\E/i) {
            if (!$obj || $cur_obj eq $obj) {
                if (!$name || $cur_name eq $name) {
                    $l =~ s/\Q$attr\E\s*=\s*.+/$attr = $value/ig;
                    $done=1
                }
            }
        }

        if ($l =~ /^\s*Name\s*=\s*"?([\w\d\.-]+)"?/i) {
            $cur_name = $1;
        }

        my $add_missing = 0;
        if ($is_options) {
            if ($l =~ /\}/) {
                $add_missing = 1;
            }
        } elsif ($l =~ /^\}/) {
            $add_missing = 1;
        }
    
        if ($add_missing) {
            if (!$done) {
                if ($cur_obj && $cur_obj eq $obj) {
                    if (!$name || $cur_name eq $name) {
                        $l =~ s/\}/\n  $attr = $value\n\}/;
                    }
                }
            }
            $cur_name = $cur_obj = undef;
        }
        print FP $l;
    }
    close(SRC);
    close(FP);
    copy("$tmp/1.$$", $file) or die "Can't copy $tmp/1.$$ to $file";
}

# This test the list jobs output to check differences
# Input: read file argument
#        check if all jobids in argument are present in the first
#        'list jobs' and not present in the second
# Output: exit(1) if something goes wrong and print error
sub check_prune_list
{
    my $f = shift;
    my %to_check = map { $_ => 1} @_;
    my %seen;
    my $in_list_jobs=0;
    my $nb_list_job=0;
    my $nb = scalar(@_);
    open(FP, $f) or die "Can't open $f $!";
    while (my $l = <FP>)          # read all files to check
    {
        if ($l =~ /list jobs/) {
            $in_list_jobs=1;
            $nb_list_job++;
            
            if ($nb_list_job == 2) {
                foreach my $jobid (keys %to_check) {
                    if (!$seen{$jobid}) {
                        print "ERROR: in $f, can't find JobId=$jobid in first 'list jobs'\n";
                        exit 1;
                    }
                }
            }
            next;
        }
        if ($nb_list_job == 0) {
            next;
        }
        if ($l =~ /Pruned (\d+) Job for client/) {
            if ($1 != $nb) {
                print "ERROR: in $f, Prune command returns $1 jobs, want $nb\n";
                exit 1;
            }
        }

        if ($l =~ /No Jobs found to prune/) {
           if ($nb != 0) {
                print "ERROR: in $f, Prune command returns 0 job, want $nb\n";
                exit 1;
            }            
        }

        # list jobs ouput:
        # | 1 | NightlySave | 2010-06-16 22:43:05 | B | F | 27 | 4173577 | T |
        if ($l =~ /^\|\s+(\d+)/) {
            if ($nb_list_job == 1) {
                $seen{$1}=1;
            } else {
                delete $seen{$1};
            }
        }
    }
    close(FP);
    foreach my $jobid (keys %to_check) {
        if (!$seen{$jobid}) {
            print "******** listing of $f *********\n";
            system("cat $f");
            print "******** end listing of $f *********\n";
            print "ERROR: in $f, JobId=$jobid should not be, but is still present in the 2nd 'list jobs'\n";
            exit 1;
        }
    }
    if ($nb_list_job != 2) {
        print "ERROR: in $f, not enough 'list jobs'\n";
        exit 1;
    }
    exit 0;
}

# This test ensure that 'list copies' displays only each copy one time
#
# Input: read stream from stdin or with file list argument
#        check the number of copies with the ARGV[1]
# Output: exit(1) if something goes wrong and print error
sub check_multiple_copies
{
    my ($nb_to_found) = @_;

    my $in_list_copies=0;       # are we or not in a list copies block
    my $nb_found=0;             # count the number of copies found
    my $ret = 0;
    my %seen;

    while (my $l = <>)          # read all files to check
    {
        if ($l =~ /list copies/) {
            $in_list_copies=1;
            %seen = ();
            next;
        }

        # not in a list copies anymore
        if ($in_list_copies && $l =~ /^ /) {
            $in_list_copies=0;
            next;
        }

        # list copies ouput:
        # |     3 | Backup.2009-09-28 |  9 | DiskChangerMedia |
        if ($in_list_copies && $l =~ /^\|\s+\d+/) {
            my (undef, $jobid, undef, $copyid, undef) = split(/\s*\|\s*/, $l);
            if (exists $seen{$jobid}) {
                print "ERROR: $jobid/$copyid already known as $seen{$jobid}\n";
                $ret = 1;
            } else {
                $seen{$jobid}=$copyid;
                $nb_found++;
            }
        }
    }
    
    # test the number of copies against the given arg
    if ($nb_to_found && ($nb_to_found != $nb_found)) {
        print "ERROR: Found wrong number of copies ",
              "($nb_to_found != $nb_found)\n";
        exit 1;
    }

    exit $ret;
}

use POSIX qw/strftime/;
sub get_time
{
    my ($sec) = @_;
    print strftime('%F %T', localtime(time+$sec)), "\n";
}

sub debug
{
    if ($debug) {
        print join("\n", @_), "\n";
    }
}

sub p
{
    debug("\n################################################################",
          @_,
          "################################################################\n");
}

# check if binaries are OK
sub remote_check
{
    my $ret = 0;
    my $path = "/opt/bacula/bin";
    print "INFO: check binaries\n";
    foreach my $b (qw/bacula-fd bacula-dir bconsole bdirjson bsdjson
                      bfdjson bbconsjson bacula-sd/)
    {
        if (-x "$path/$b") {
            my $out = `$path/$b -? 2>&1`;
            if ($out !~ /Version:/g) {
                print "ERROR: with $b -?\n";
                system("$path/$b -?");
                $ret++;
            }
        }
    }
    foreach my $b (qw/bacula-sd/)
    {
        if (-r "$path/$b") {
            my $libs = `ldd $path/$b`;
            if ($libs !~ /tokyocabinet/g) {
                print "ERROR: unable to find tokyocabinet for $b\n";
                print $libs;
                $ret++;
            }
        }
    }

    return $ret;
}

sub remote_config
{
    open(FP, ">$REMOTE_FILE/bacula-fd.conf") or 
        die "ERROR: Can't open $REMOTE_FILE/bacula-fd.conf $!";

    my $plugins = '/opt/bacula/bin';
    if (-d '/opt/bacula/plugins') {
        $plugins = '/opt/bacula/plugins';
    }

    print FP "
Director {
  Name = $HOST-dir
  Password = \"$REMOTE_PASSWORD\"
}
FileDaemon {
  Name = remote-fd
  FDport = $REMOTE_PORT
  WorkingDirectory = $REMOTE_FILE/working
  Pid Directory = $REMOTE_FILE/working
  Plugin Directory = $plugins
  Maximum Concurrent Jobs = 20
}
Messages {
  Name = Standard
  director = $HOST-dir = all, !skipped, !restored
}
";  
    close(FP);
    system("mkdir -p '$REMOTE_FILE/working' '$REMOTE_FILE/save'");
    system("rm -rf '$REMOTE_FILE/restore'");
    my $pid = fork();
    if (!$pid) {
        close(STDIN);  open(STDIN, "/dev/null");
        close(STDOUT); open(STDOUT, ">/dev/null");
        close(STDERR); open(STDERR, ">/dev/null");        
        exec("/opt/bacula/bin/bacula-fd -c $REMOTE_FILE/bacula-fd.conf");
        exit 1;
    }
    sleep(2);
    $pid = `cat $REMOTE_FILE/working/bacula-fd.$REMOTE_PORT.pid`;
    chomp($pid);

    # create files and tweak rights
    create_many_files("$REMOTE_FILE/save", 5000);
    chdir("$REMOTE_FILE/save");
    my $d = 'A';
    my $r = 0700;
    for my $g ( split(' ', $( )) {
        chmod $r++, $d;
        chown $<, $g, $d++;
    }
    
    # create a sparse file of 2MB
    init_delta("$REMOTE_FILE/save", 2000000);

    # create a simple script to execute
    open(FP, ">test.sh") or die "Can't open test.sh $!";
    print FP "#!/bin/sh\n";
    print FP "echo this is a script";
    close(FP);
    chmod 0755, "test.sh";

    # create a hardlink
    link("test.sh", "link-test.sh");

    # create long filename
    mkdir("b" x 255) or print "can't create long dir $!\n";
    copy("test.sh", ("b" x 255) . '/' . ("a" x 255)) or print "can't create long dir $!\n";

    # play with some symlinks
    symlink("test.sh", "sym-test.sh");
    symlink("$REMOTE_FILE/save/test.sh", "sym-abs-test.sh");

    if ($pid) {
        system("ps $pid");
        $estat = ($? != 0);
    } else {
        $estat = 1;
    }
}

sub remote_diff
{
    debug("Doing diff between save and restore");
    system("ssh $REMOTE_USER$REMOTE_ADDR " . 
     "$REMOTE_FILE/scripts/diff.pl -s $REMOTE_FILE/save -d $REMOTE_FILE/restore/$REMOTE_FILE/save");
    $dstat = ($? != 0);
}

sub remote_stop
{
    debug("Kill remote bacula-fd $REMOTE_ADDR");
    system("ssh $REMOTE_USER$REMOTE_ADDR " . 
             "'test -f $REMOTE_FILE/working/bacula-fd.$REMOTE_PORT.pid && " . 
              "kill `cat $REMOTE_FILE/working/bacula-fd.$REMOTE_PORT.pid`'");
}

sub remote_init
{
    system("ssh $REMOTE_USER$REMOTE_ADDR mkdir -p '$REMOTE_FILE/scripts/'");
    system("scp -q scripts/functions.pm scripts/diff.pl $REMOTE_USER$REMOTE_ADDR:$REMOTE_FILE/scripts/");
    system("scp -q config $REMOTE_USER$REMOTE_ADDR:$REMOTE_FILE/");
    debug("INFO: Configuring remote client");
    system("ssh $REMOTE_USER$REMOTE_ADDR 'cd $REMOTE_FILE && perl -Mscripts::functions -e remote_config'");
    system("ssh $REMOTE_USER$REMOTE_ADDR 'cd $REMOTE_FILE && perl -Mscripts::functions -e remote_check'");
}

sub create_binfile
{
    my ($file, $nb) = @_;
    $nb |= 10;

    if (!open(FP, ">$file")) {
        print "ERR\nCan't create txt $file $@\n";
        exit 1;
    }
    for (my $i = 0; $i < $nb ; $i++) {
        foreach my $c ('a'..'z') {
            my $l = ($c x 1024);
            print FP $l;
        }
    }
    close(FP);
}

my $c = "a";

sub init_delta
{
    my ($source, $sparse_size) = @_;

    $sparse_size = $sparse_size || 100000000;

    # Create $source if needed
    system("mkdir -p '$source'");

    if (!chdir($source)) {        
        print "ERR\nCan't access to $source $!\n";
        exit 1;
    }
 
    open(FP, ">text.txt") or return "ERR\nCan't create txt file $@\n";
    my $l = ($c x 80) . "\n";
    print FP $l x 40000;
    close(FP);

    open(FP, ">prev");
    print FP $c, "\n";
    close(FP);

    open(FP, ">sparse.dat") or return "ERR\nCan't create sparse $@\n";
    seek(FP, $sparse_size, 0);
    print FP $l;
    close(FP);
}

sub update_delta
{
    my ($source) = shift;

    if (!chdir($source)) {        
        return "ERR\nCan't access to $source $!\n";
    }

    $c = `cat prev`;
    chomp($c);

    open(FP, "+<sparse.dat") or return "ERR\nCan't update the sparse file $@\n";
    seek(FP, int(rand(-s "sparse.dat")), 0);
    print FP $c x 400;
    seek(FP, 0, 2);
    print FP $c x 4000;
    close(FP);


    open(FP, ">>text.txt") or return "ERR\nCan't update txt file $@\n";    
    $c++;
    my $l = ($c x 80) . "\n";
    print FP $l x 40000;
    close(FP);

    open(FP, ">prev");
    print FP $c, "\n";
    close(FP);

    return "OK\n";
}

1;
