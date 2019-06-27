
This is a brief description of a new Statistics collector functionality.

0. Requirements and Assumptions

>>>Eric Bollengier <eric.bollengier@baculasystems.com> at 12/04/2018
>>>
The main idea would be to generate statistics from our daemon (mainly the
storage daemon, but FD and Dir can be interested too) and send this data to a
Graphite daemon.

Graphite is a bit like RRDTool, it is in charge to collect the data and render
the information.

https://graphiteapp.org/

I think that the solution at the Bacula level should be able to use different
"drivers" (csv file on disk with a configurable format for example, native
graphite tcp connection, etc...).

At the Bacula level, we probably need a new Resource in the configuration file
to configure that (or only new directives I don't know). Once you know what you
need, Kern will review the name of the Resources and/or Directives.

Each job should be able to send data to the Bacula collector (ie from multiple
threads). Ideally, a job should not be blocked because the Graphite socket is
hanging for example.

We need to define interesting metrics that can be reported to the statistic
collector, few examples: 
- nb of jobs at a given time 
- device statistics (nb read, nb write, nb job...) 
- total network throughput 
- per job network throughput 
- disk throughput 
- current memory usage 
- system information (cpu, load average, swap usage) 
- number of files read for a job 
- (basically what the status command reports)

It might be interesting to let the user choose the metrics they want to see, and
have a directive such as the destination message.

  Metrics = NbJob, NetworkThroughput, DiskThroughput, MemoryUsage

(this is just an idea).

We can start with few basic statistics and enrich the program later.

1. Statistics Collector Architecture

The most important requirement is to not block a job thread because external
communication stalled or introduced unexpected latency which can negatively
affect a running job. This requirement lead to the strict separation in
Collector functionality and behavior.

The collector was designed as a two separate entities:
- an internal collector class: bstatcollect
- an interface collector thread: COLLECTOR
The first one functions as a metrics cache and the second one is responsible for 
sending collected metrics into an external data target (i.e. Graphite).

1.1. Statistics Collection flow

Any Bacula's metrics are collected in a push architecture where an object code 
is responsible for preparing/generating a metrics and "push" it to internal 
collector for later usage. This "push" operation should be extremely fast which
allows to avoid unexpected latency in the job.

To save a metrics in internal collector requires two step process: metric
registration and metrics update. Metrics registration could take some time
(relatively speaking) with O(n) and return a metrics index used in metrics update
process which is very fast with O(1).

You should register metrics at the very beginning of the job or daemon start when
additional latency is not a problem at all. The update process using the metric
index from registration will be very fast, so it should be no problem setting it
even at the time critical part of the code, i.e.

metric_index = statcollector->registration_int64("bacula.jobs.all",
				METRIC_UNIT_JOB, value, "The number of all jobs");

(...)

while (true)

(...)

	statcollector->set_value_int64(metric_index, newvalue);

(...)

statcollector->unregistration(metric_index);

The only latency introduced by a metrics update process is lock/unlock resolution
used for internal collector synchronization access.

The metrics should be unregistered when no needed any more. After unregistration 
process the metrics index becomes invalid and should not be used to address this 
metric. As the metrics index is a regular integer number it will be reused upon 
next registration of any new metric.

You can get any or all metrics values when required. The return will be always
the full copy of the metrics so you can process it as you wish.

1.2. Statistics Collector backend thread

The collector background thread (COLLECTOR resource) is responsible for getting 
the copy of the current metrics list and save them to configured destination. 
The save/send process could be a time consuming, i.e. could involve a network 
communication like for a Graphite collector. As this collector thread operates 
on a copy of metrics list it doesn't affect standard job operations. Collector 
thread saves metrics at regular intervals.
	Note: the current implementation introduced a two backends: CSV file and 
	Graphite which are build in backends.

1.3. Statistics Collector update thread

We have a two metrics types to collect: an easy to count/update and a hard one 
to count/update. The easy to count/update metrics are all statistics which 
corresponds to already available (in memory) counter/variable, so we can update 
metrics every time the counter/variable is updated. We can achieve a perfect 
accuracy here without a problem. On the other hand the hard to count/update 
metrics are all metrics which depends on external data (i.e. almost all 
permanent metrics based on catalog data), metrics which are not directly 
controllable by Bacula (i.e. the size of the heap) or metrics where frequent 
update will bring a huge performance impact (i.e. sm_pool_memory size). For this 
kind of metrics we've develop a dedicated mechanism for updating these metrics. 

The main assumption here is that these metrics won't be updated until necessary, 
so as long as nobody would check what is the value we won't update it. We agreed 
that the real value of the metrics (i.e. a number of error jobs) could change a 
dozens of times in the mean time, but we want a perfect value at the time of 
sampling, so i.e. saving/sending to external backend.

For this purpose we run a dedicated collector update thread which will start 
only when any of collector backend threads are started. So, if no collectors 
defined for the daemon, no update thread will be necessary. The collector update 
thread executes a dedicated function to every daemon as every daemon would has a 
different set of hard to count/update metrics. The collector update thread 
updates required metrics as frequent as the minimal interval parameter set at 
defined collector resources. So, for two collector resources which would have 
Interval=5min and Interval=30sec, the update thread should get a 30secs 
interval. Additionally the "collect" command, which display all available 
metrics at any time, is executing the same update function as for update thread 
to get up to date metrics.

2. Architecture implementation and code

2.1. The metrics class

This is a basic building block for metrics collection:

class bstatmetrics : public SMARTALLOC {
public:
    char *name;                             /* this is a metrics name */
    metric_type_t type;                     /* this is a metrics type */
    metric_unit_t unit;                     /* this is a metrics unit */
    metric_value_t value;                   /* this is a metrics value */
    char *description;                      /* this is a metrics description */
    bstatmetric();
    bstatmetric(char *mname, metric_type_t mtype, metric_unit_t munit, 
    	char *descr);
    bstatmetric(char *mname, metric_unit_t munit, bool mvalue, char *descr);
    bstatmetric(char *mname, metric_unit_t munit, int64_t mvalue, char *descr);
    bstatmetric(char *mname, metric_unit_t munit, float mvalue, char *descr);
    ~bstatmetric();
    bstatmetric& operator=(const bstatmetric& orig);
    void render_metric_value(POOLMEM **buf, bool bstr=false);
    void render_metric_value(POOL_MEM &buf, bool bstr=false);
    const char *metric_type_str();
    const char *metric_unit_str();
    void dump();
};

You can have a three (technically four) types of metrics in bstatmetric:
- METRIC_UNDEF - when bstatmetrics is uninitialized
- METRIC_INT - the metrics stores an integer values (int64_t)
- METRIC_BOOL - the metrics stores a boolean values (so, True/False)
- METRIC_FLOAT - the metrics stores a float values

You can define a metrics unit, which shows what entity a value represent, i.e.
METRIC_UNIT_BYTE, METRIC_UNIT_BYTESEC, METRIC_UNIT_JOB, METRIC_UNIT_CLIENT, etc.
When a value have no unit and it is a just a value/number you should use:  
METRIC_UNIT_EMPTY.

2.2. The internal collector class

The metrics objects collection is an internal collector class:

class bstatcollect : public SMARTALLOC {
public:
    bstatcollect();
    /* registration return a metrics index */
    int registration(char *metric, metric_type_t type, metric_unit_t unit, 
    	char *descr);
    int registration_bool(char *metric, metric_unit_t unit, bool value, 
    	char *descr);
    int registration_int64(char *metric, metric_unit_t unit, int64_t value, 
    	char *descr);
    int registration_float(char *metric, metric_unit_t unit, float value, 
    	char *descr);
    /* unregistration */
    void unregistration(int metric);
    /* update/set the metrics value */
    int set_value_bool(int metric, bool value);
    int set_value_int64(int metric, int64_t value);
    int add_value_int64(int metric, int64_t value);
    int add2_value_int64(int metric1, int64_t value1, int metric2, 
    	int64_t value2);
    int sub_value_int64(int metric, int64_t value);
    int set_value_float(int metric, float value);
    int inc_value_int64(int metric);
    int dec_value_int64(int metric);
    int dec_inc_values_int64(int metricd, int metrici);
    /* get data */
    bool get_bool(int metric);
    int64_t get_int(int metric);
    float get_float(int metric);
    alist *get_all();
    bstatmetrics *get_metric(char *metric);
    /* utility */
    void dump();
};

You can register a metrics of particular type and unit with 
bstatcollect::registration() method. In this case the value will be set 
initially to zero. Using other registration_*() methods you can set other 
initial value.

When you make a next metrics registration in the time when a particular metrics 
already exist in bstatcollect object you will always get the same metrics index. 
When you unregister a metrics and register it later again you could get a 
different metrics index.

Any metrics could have a description string which could be useful to users. You 
can set metrics description at first registration only. Any subsequent metrics 
registration does not update it.

2.2.1. Updating the metrics value

Any metrics value update should be performed as an atomic operation, so the 
internal collector class has a number of a such methods:
- set_value_*() - will set the metrics value into method argument "value", old 
	metrics value will be overwritten
- inc_value_int64()/dec_value_int64() - will increment or decrement the metrics 
	value accordingly as a single atomic operation
- dec_inc_values_int64() - will decrement a first metrics value and increment 
	a second metrics value as a single atomic operation, used to update a related 
	metrics in a single step
- add_value_int64() - will add the numeric argument to the metrics value as a 
	single atomic operation
- add2_value_int64() - will add the two numeric arguments to the two metrics 
	values as a single atomic operation, used to update more metrics in a single 
	step

The inc_value_int64()/dec_value_int64()/add_value_int64()/etc. should be used 
when managing a "shared" metrics updated from a different or separate threads.

2.3. Supported utilities

There are a few supported utilities you can use when processing metrics or 
metrics list:
- bstatmetric::render_metric_value() - render a metrics value as a string into 
	a buffer.
- bstatmetric::metric_type_str() - return a metrics type as string.
- bstatmetric::metric_unit_str() - return a metrics unit as string.
- free_metric_alist() - releases memory for a list of metrics returned from 
	bstatcollect::get_all().

3. Statistics resource configuration

The Statistics resource defines the attributes of the Statistics collector threads
running on any daemon. You can define any number of Collector resources and every
single Statistics resource will spawn a single collector thread. This resource can be 
defined for any Daemon (Dir, SD and FD). Resource directives:

Name = <name> The collector name used by the system administrator. This 
directive is required.

Description = <text> The text field contains a description of the Collector that 
will be displayed in the graphical user interface. This directive is optional.

Interval = <time-interval> This directive instruct Collector thread how long it 
should sleep between every collection iteration. This directive is optional and 
when not specified a value 300 seconds will be used instead.

Type = <backend> The Type directive specifies the Collector backend, which may 
be one of the following: CSV or Graphite. This directive is required.

-> CSV is a simple file level backend which saves all required metrics with the 
following format to the file: <time>, <metric>, <value>\n

--> where: <time> is a standard Unix time (a number of seconds from 1/01/1970) 
with local timezone as returned by a system call time(). <metric> is a Bacula 
metrics string and <vale> is a metrics value which could be in numeric format 
(int/float) or a string "True" or "False" for boolean variable. The CSV backend 
require a File=... parameter.

-> Graphite is a network backend which will send all required metrics to 
Graphite server. This backend requires Host=... and Port=... directives to be 
set.

Metrics = <metricspec> This directive allow metrics filtering and <metricspec> is 
a filter which uses "*" and "?" characters to match required metrics name in the 
same way as found in shell wildcard resolution. You can exclude filtered metrics 
with '!' prefix. You can define any number of filters for a single Collector. 
Metrics filter is executed in order as found in configuration. This directive is 
optional and if not used all available metrics will be saved by this collector 
backend. Example: 
- include any metrics started with bacula.jobs:
	Metrics = "bacula.jobs.*"
- exclude any metrics started with bacula.jobs
	Metrics = "!bacula.jobs.*"

Prefix = <prefix> This directive allows to alter the metrics name saved by 
collector to distinguish between different installations or daemons. The 
<prefix> string will be added to metrics name as: <prefix>.<metric_name>
This directive is optional. If not set no metrics name will be altered.

File = <filename> This directive is used for CSV collector backend only and 
point to the full path and filename of the file where metrics will be saved. 
In this case the directive is required. The collector thread has to have a 
permissions to write to selected file or create a new file if it won't exist.
If collector will be unable to write to the file or create a new one then 
a collection terminate and error message will be generated. The file is open 
during saving only and closed otherwise. Statistics file rotation could be 
executed by a simple mv ... shell command. 

Host = <IP-Address> This directive is used for Graphite backend only and 
specify the IP address or hostname of the Graphite server. In this case the 
directive is required.

Port = <port-number> This directive is used for Graphite backend only and 
specify the port number for Graphite server connection. In this case the 
directive is required.

MangleMetric = <yes|no> This directive will replace all dots in your resource
metric with '%32'. It is especially useful when using Graphite collector 
backend and Bacula's resource names with dots the Graphite application will
incorrectly split metrics tree based on resource name. It could be useful if
user wants another level of metrics separation but it could be annoying when
used unintentionally. This string could be demangled in GUI - i.e. BWeb.

The currently developed are a two Collector backends: CSV and Graphite. Other 
backends could be developed if required. The Statistics resource is a build-in 
feature.

3.1. Metrics spooling

Metrics spooling is a feature build in some backends which allows to postpone 
metrics delivery to external database (i.e. Graphite) in case of any delivery 
problems. The postpone delivery uses regular file located in a Working Directory 
of the daemon as a spool file. The metrics spooling to file starts automatically 
and run until delivery destination will become available again. In this case 
spooled metrics will be despooled automatically and spooling function suspend 
until the next delivery problem. This feature is available now for Graphite 
backend only. When metrics spooling cannot be performed (i.e. cannot create the 
spooling file) the spooling will be disabled until daemon restart or 
configuration reload. The spooling file is not deleted on daemon restart, so any 
spooled metrics will be saved until destination become available (despooling).

4. Bconsole

You can manually display Bacula metrics with bconsole command: collect.

statistics all | <metric> [ simple | full | json ] This command display all Bacula 
metrics when all keyword is set or a single particular metrics with <metric> 
parameter. You can change the output format with simple, full or json keywords. 
When no format keyword is set then a simple format will be used.

- Simple format example:

*statistics bacula.jobs.all
bacula.jobs.all=276

- Full format example:

*statistics bacula.jobs.all full
name="bacula.jobs.all" value=276 type=Integer unit=Jobs descr="Number of all 
jobs."

- JSON format example:

*statistics bacula.jobs.all json
[
  {
    "name": "bacula.jobs.all",
    "value": 164,
    "type": "Integer",
    "unit": "Jobs",
    "description": "Number of all jobs."
  }
]

By default you will query a Director daemon with statistics command. If you wish to 
query a Storage Daemon or File Daemon then you have to add a storage=<sd> or 
client=<fd> parameters to your statistics command, i.e.

*statistics bacula.storage.File1.config.devices storage=File1
Connecting to Storage daemon File1 at bacula-devel:9103
bacula.storage.File1.config.devices=4

*statistics bacula.storage.bacula-devel-sd.config.devices storage=File1 json
Connecting to Storage daemon File1 at 192.168.0.224:9103
[
  {
    "name": "bacula.storage.bacula-devel-sd.config.devices",
    "value": 4,
    "type": "Integer",
    "unit": "Device",
    "description": "The number of defined devices in Storage."
  }
]

This feature does not require any Statistics backend resource to be setup.

You can display any Statistics resource configuration using the bconsole command: 
show collector, i.e.

*show statistics
Statistics: name=CSV collector
            type=1 interval=300 secs
            prefix=
            file=/tmp/stat-dir.csv
Statistics: name=Graphite
            type=2 interval=60 secs
            prefix=backup
            host=192.168.0.229 port=2003

You can display a status of any running Statistics backend with a special 
".status <daemon> statistics[=<name>]" command. You have to give a proper daemon 
resource name, i.e. "dir", "storage[=<name>]", "client[=<name>]" to point which 
daemon you want to examine. You can limit a ".status" command to display
information about a single Statistics backend with "statistics=<name>" parameter. 
When no statistics name will be set then information about all Statistics backends 
and Statistics update thread will be displayed, i.e.

*.status dir statistics
Statistics backend: CSV collector is running
 type=1 lasttimestamp=24-Jul-18 10:51
 interval=300 secs
 spooling=disabled

Statistics backend: Graphite is running
 type=2 lasttimestamp=24-Jul-18 10:51
 interval=60 secs
 spooling=enabled

Update Statistics: running interval=60 secs lastupdate=24-Jul-18 10:51

*.status client=bacula-devel-fd statistics=Graphite
Connecting to Client bacula-devel-fd at bacula-devel:9102
Statistics backend: Graphite is running
 type=2 lasttimestamp=24-Jul-18 10:52
 interval=30 secs
 spooling=enabled

A ".status statistics" command support APIv2 display, i.e.

*.api 2
*.status dir statistics
collector_backends:

name=CSV1
status=running
interval=60
lasttimestamp_epoch=1561643144
lasttimestamp=2019-06-27 15:45:44
spooling=disabled
lasterror=


name=CSV2
status=running
interval=60
lasttimestamp_epoch=1561643144
lasttimestamp=2019-06-27 15:45:44
spooling=disabled
lasterror=


name=Graphite1
status=running
interval=60
lasttimestamp_epoch=1561643147
lasttimestamp=2019-06-27 15:45:47
spooling=in progress
lasterror=Could not connect to localhost:9223 Err=Connection refused


collector_update:

status=running
interval=60
lasttimestamp_epoch=1561643144
lasttimestamp=2019-06-27 15:45:44
