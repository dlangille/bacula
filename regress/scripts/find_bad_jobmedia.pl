#!/usr/bin/perl -w
use strict;

=head1 NAME

    find_bad_jobmedia.pl -- Bacula tool to detect bad JobMedia records

=head1 SYNOPIS

    find_bad_jobmedia.pl

                --bin=/opt/bacula/bin 
                --conf=/opt/bacula/etc/bacula-dir.conf

                -U|--user|-u      user
                -D|--database|-d  database
                -h|--host         hostname
                -T|--type         database type (Pg|mysql)
                -P|--port         database port

                --logs            display logs of jobs

=head2 VERSION

   Copyright (C) 2000-2020 Kern Sibbald
   License: BSD 2-Clause; see file LICENSE-FOSS

   Written by Eric Bollengier

   1.1

=cut

use DBI;
use Getopt::Long qw/:config no_ignore_case/;
use Data::Dumper;
use Pod::Usage;
use POSIX qw(strftime);

my $bin_path = "/opt/bacula/bin";
my $dir_conf = "/opt/bacula/etc/bacula-dir.conf";

my $user='';
my $pass='';
my $host='';
my $dbname='';
my $dbport='';
my $dbtype='postgresql';
my $dsp_logs=0;

GetOptions('bin=s'    => \$bin_path,
           'conf=s'   => \$dir_conf,
           'U=s'      => \$user,
           'user|u=s' => \$user,
           'host|h=s' => \$host,
           'd|database|D=s' => \$dbname,
           'type|T=s' => \$dbtype,
           'port|P=i' => \$dbport,
           'logs'     => \$dsp_logs,
    ) || Pod::Usage::pod2usage(-exitval => 2, -verbose => 2) ;

my %dbi;

sub p
{
    print "\n", strftime("%H:%M:%S ", localtime);
    print @_;
}

sub connect_db
{
    my ($dbi, $user, $password) = @_;

    if (!$dbi) {
        ($dbi, $user, $password) = get_bacula_db();
    }

    my $dbh;

    if ($dbi !~ /Pg|mysql|SQLite/) {
        die("ERROR: It looks you have a case problem with your DBI string.\n" . 
            "$dbi doesn't contain Pg, mysql or SQLite");
    }
    
    $dbh = DBI->connect($dbi, $user, $password);

    die("ERROR: Can't connect to your database:\n$DBI::errstr\n\n" .
        "You can try to run this script as 'postgres'\n" .
        "For example: $0 -d bacula\n"
        ) unless ($dbh);

    $dbh->{FetchHashKeyName} = 'NAME_lc';
    $dbh->{PrintError} = 1;

    return $dbh;
}

sub get_bacula_db
{
    if ($user || $host || $dbname || $dbport) {
        $dbi{db_user}    = $user;
        $dbi{db_address} = $host;
        $dbi{db_name}    = $dbname;
        $dbi{db_port}    = $dbport;
        $dbi{db_type}    = $dbtype;

        die "Unable to find database name in argument"
            unless $dbi{db_name};

    } else {

        die "ERROR: Unable to find or access $bin_path/dbcheck"
            unless (-x "$bin_path/dbcheck");
        
        die "ERROR: Unable to find or access $dir_conf"
            unless (-r $dir_conf);
        
        my @conf = `$bin_path/dbcheck -B -c $dir_conf`;
        if ($? != 0) {
            die "ERROR: dbcheck returned an error\n@conf";
        }
        
        my $found=0;
        
        foreach my $l (@conf) {
            chomp($l);
            if ($l !~ /=/) {
                print "Strange line on dbcheck output $l\n";
                next;
            }
            if ($l =~ /catalog=/) {
                if ($found++ > 0) {
                    print "Will choose the first catalog, discarding $l\n";
                    print "To migrate $l, use command line parameters\n";
                    last;
            }
            }
            my ($k, $v) = split(/=/, $l);
            $dbi{$k} = $v;
        }

        die "ERROR: Unable to get information from dbcheck"
            unless ($dbi{db_type} and $dbi{db_name});
    }

    my $dbi_string;
    if (lc($dbi{db_type}) eq 'postgresql') {
        $dbi_string = "DBI:Pg";

    } elsif (lc($dbi{db_type}) eq 'mysql') {
        $dbi_string = "DBI:mysql";

    } else {
        die "Database type $dbi{db_type} is not supported by this script";
    }

    # Is present!
    if ($dbi{db_name}) {
        $dbi_string .= ":database=$dbi{db_name}";
    }

    if ($dbi{db_address}) {
        $dbi_string .= ";host=$dbi{db_address}";
    }

    if ($dbi{db_port}) {
        $dbi_string .= ";port=$dbi{db_port}";
    }

    if ($dbi{db_socket}) {
        $dbi_string .= ";host=$dbi{db_socket}";
    }

    return ($dbi_string, $dbi{db_user}, $dbi{db_password});
}

my $ret;
my $dbh = connect_db();

p "INFO: Connexion to the catalog OK\n";

################################################################

my @suspectfields = qw(JobMediaId JobId MediaId FirstIndex
                       LastIndex StartFile EndFile StartBlock
                       EndBlock VolIndex);

my $suspect = $dbh->selectall_arrayref("
SELECT " . join(",", @suspectfields) . "
  FROM JobMedia
 WHERE EndFile <= StartFile AND EndBlock < StartBlock
 ORDER BY JobId, VolIndex");

if (scalar(@$suspect) == 0) {
    p "INFO: No problem detected\n";
    exit 0;
}

print "\n================================================================\n";
p "INFO: Found " . scalar(@$suspect) . " suspect records\n";
print join("\t", @suspectfields), "\n";
foreach my $row (@$suspect) {
    print join("\t", @$row), "\n";
}

my $jobids = join(",", map { $_->[1] } @$suspect);

print "\n================================================================\n";
p "INFO: Dumping JobMedia for suspicious jobs\n";

my @jobmediafields = qw(JobMediaId JobId MediaId FirstIndex
                        LastIndex StartFile EndFile StartBlock
                        EndBlock VolIndex);
my $jobmedia = $dbh->selectall_arrayref("
SELECT " . join(",", @jobmediafields) . "
  FROM JobMedia
 WHERE JobId IN ($jobids)
 ORDER BY JobId, VolIndex
");

print join("\t", @jobmediafields), "\n";
foreach my $row2 (@$jobmedia) {
    print join("\t", @$row2), "\n";
}

print "\n================================================================\n";
p "INFO: Dumping Jobs information\n";

my @jobfields = qw(JobId Job Name Type ClientId StartTime EndTime JobFiles
                   JobBytes JobStatus);

my $jobs = $dbh->selectall_arrayref("
SELECT " . join(",", @jobfields) . "
 FROM Job
 WHERE JobId IN ($jobids)
 ORDER BY JobId
");

my $nbjob = scalar(@$jobs);

print join("\t", @jobfields), "\n";
foreach my $row2 (@$jobs) {
    print join("\t", @$row2), "\n";
}

print "\n================================================================\n";
p "INFO: Dumping info about Media\n";

my @mediaidfields = qw(MediaId VolumeName LastWritten VolBlocks VolBytes 
                       VolFiles VolJobs VolRetention VolStatus);

my $mediaids = join(",", map { $_->[2] } @$suspect);

my $media = $dbh->selectall_arrayref("
SELECT " . join(",", @mediaidfields) . "
  FROM Media
 WHERE MediaId IN ($mediaids)
 ORDER BY MediaId
");

print join("\t", @mediaidfields), "\n";
foreach my $row2 (@$media) {
    print join("\t", map { defined $_ ? $_ : 'null' } @$row2), "\n";
}

# We display logs if requested or if we don't have too much jobs involved
if ($dsp_logs || $nbjob < 15) {
    print "\n================================================================\n";
    p "INFO: Dumping logs information for all jobs between start and end time\n";
    my @logfields = qw/Time LogText/;
    foreach my $job (@$jobs) {
        p "INFO: Dumping for JobId $job->[0]\n";
        print join("\t", @logfields), "\n";
        my $log = $dbh->selectall_arrayref("
SELECT " . join(",", @logfields) . "
  FROM Log
 WHERE Time >= '$job->[5]' and Time <= '$job->[6]'
 ORDER BY LogId
");
        # If we display lines automatically, ensure
        # we don't have too much lines
        if ($dsp_logs || scalar(@$log) < 1000) {
            foreach my $row2 (@$log) {
                print join("\t", @$row2);
            }
        } else {
            print "Too much lines... ", scalar(@$log), "\n";
        }
    }
}

################################################################

exit 0;

