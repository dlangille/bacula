var Units = {
	units: {
		size: [
			{short: '',  long: 'B', value: 1},
			{short: 'kb', long: 'kB', value: 1000},
			{short: 'k', long: 'KiB', value: 1024},
			{short: 'mb', long: 'MB', value: 1000},
			{short: 'm', long: 'MiB', value: 1024},
			{short: 'gb', long: 'GB', value: 1000},
			{short: 'g', long: 'GiB', value: 1024},
			{short: 'tb', long: 'TB', value: 1000},
			{short: 't', long: 'TiB', value: 1024},
			{short: 'pb', long: 'PB', value: 1000},
			{short: 'p', long: 'PiB', value: 1024}
		],
		speed: [
			{short: '',  long: 'B/s', value: 1},
			{short: 'kb/s', long: 'kB/s', value: 1000},
			{short: 'k/s', long: 'KiB/s', value: 1024},
			{short: 'mb/s', long: 'MB/s', value: 1000},
			{short: 'm/s', long: 'MiB/s', value: 1024}
		],
		time: [
			{long: 'second', value: 1},
			{long: 'minute', value: 60},
			{long: 'hour', value: 60},
			{long: 'day', value: 24}
		]
	},
	get_size: function(size, unit_value) {
		var dec_size;
		var units = [];
		for (var u in Units.units.size) {
			if ([1, unit_value].indexOf(Units.units.size[u].value) !== -1) {
				units.push(Units.units.size[u].long);
			}
		}
		if (size === null) {
			size = 0;
		}

		var size_pattern = new RegExp('^[\\d\\.]+(' + units.join('|') + ')$');
		if (size_pattern.test(size.toString())) {
			// size is already formatted
			dec_size = size;
		} else {
			units.shift(); // remove byte unit
			size = parseInt(size, 10);
			var unit;
			dec_size = size.toString();
			while(size >= unit_value) {
				size /= unit_value;
				unit = units.shift();
			}
			if (unit) {
				dec_size = (Math.floor(size * 10) / 10).toFixed(1);
				dec_size += unit;
			} else if (size > 0) {
				dec_size += 'B';
			}
		}
		return dec_size;
	},
	get_decimal_size: function(size) {
		return this.get_size(size, 1000);
	},
	get_binary_size: function(size) {
		return this.get_size(size, 1024);
	},
	get_formatted_size: function(size) {
		var value = '';
		if (typeof(SIZE_VALUES_UNIT) === 'string') {
			if (SIZE_VALUES_UNIT === 'decimal') {
				value = this.get_decimal_size(size);
			} else if (SIZE_VALUES_UNIT === 'binary') {
				value = this.get_binary_size(size);
			}
		}
		return value;
	},
	format_size: function(size_bytes, format) {
		var reminder;
		var f = this.units.size[0].long;
		for (var i = 0; i < this.units.size.length; i++) {
			if (this.units.size[i].long != format && size_bytes) {
				reminder = size_bytes % this.units.size[i].value
				if (reminder === 0) {
					size_bytes /= this.units.size[i].value;
					f = this.units.size[i].long;
					continue;
				}
				break;
			}
		}
		var ret = {value: size_bytes, format: f};
		return ret;
	},
	format_time_period: function(time_seconds, format, float_val) {
		var reminder;
		var f = this.units.time[0].long;
		for (var i = 0; i < this.units.time.length; i++) {
			if (this.units.time[i].long != format && time_seconds) {
				reminder = time_seconds % this.units.time[i].value;
				if (reminder === 0 || (float_val && time_seconds >= this.units.time[i].value)) {
					time_seconds /= this.units.time[i].value;
					f = this.units.time[i].long;
					continue;
				}
				break;
			}
		}
		var ret = {value: time_seconds, format: f};
		return ret;
	},
	format_date: function(timestamp) {
		if (typeof(timestamp) === 'string') {
			timestamp = parseInt(timestamp, 10);
		}
		if (timestamp < 9999999999) {
			timestamp *= 1000;
		}
		var d = new Date(timestamp);
		var dt = DATE_TIME_FORMAT;
		if (dt.indexOf('Y') !== -1) { // full 4 digits year, ex. 2021
			dt = dt.replace(/Y/g, d.getFullYear());
		}
		if (dt.indexOf('y') !== -1) { // 2 digits year, ex, 21
			dt = dt.replace(/y/g, ('0' + d.getFullYear()).slice(-2));
		}
		if (dt.indexOf('M') !== -1) { // 2 digits month 01..12
			dt = dt.replace(/M/g, ('0' + (d.getMonth() + 1)).slice(-2));
		}
		if (dt.indexOf('m') !== -1) { // 1-2 digits month 1..12
			dt = dt.replace(/m/g, (d.getMonth() + 1));
		}
		if (dt.indexOf('D') !== -1) { // 2 digits day 01..31
			dt = dt.replace(/D/g, ('0' + d.getDate()).slice(-2));
		}
		if (dt.indexOf('d') !== -1) { // 1-2 digits day 1..31
			dt = dt.replace(/d/g, d.getDate());
		}
		if (dt.indexOf('H') !== -1) { // 2 digits 24-hour format hour 00..23
			dt = dt.replace(/H/g, ('0' + d.getHours()).slice(-2));
		}
		if (dt.indexOf('h') !== -1) { // 1-2 digits 24-hour format hour 0..23
			dt = dt.replace(/h/g, d.getHours());
		}
		if (dt.indexOf('G') !== -1) { // 2 digits 12-hour format hour value 01..12
			var hours = d.getHours() % 12;
			hours = hours ? hours : 12;
			dt = dt.replace(/G/g, ('0' + hours).slice(-2));
		}
		if (dt.indexOf('g') !== -1) { // 1-2 digits 12-hour format hour value 1..12
			var hours = d.getHours() % 12;
			hours = hours ? hours : 12;
			dt = dt.replace(/g/g, hours);
		}
		if (dt.indexOf('I') !== -1) { // 2 digits minutes 00..59
			dt = dt.replace(/I/g, ('0' + d.getMinutes()).slice(-2));
		}
		if (dt.indexOf('i') !== -1) { // 1-2 digits minutes 0..59
			dt = dt.replace(/i/g, d.getMinutes());
		}
		if (dt.indexOf('S') !== -1) { // 2 digits seconds 00..23
			dt = dt.replace(/S/g, ('0' + d.getSeconds()).slice(-2));
		}
		if (dt.indexOf('s') !== -1) { // 1-2 digits seconds 0..23
			dt = dt.replace(/s/g, d.getSeconds());
		}
		if (dt.indexOf('p') !== -1) { // AM/PM value
			var am_pm = d.getHours() >= 12 ? 'PM' : 'AM';
			dt = dt.replace(/p/g, am_pm);
		}
		if (dt.indexOf('R') !== -1) { // 24-hours format time value 17:22:41
			var minutes = ('0' + d.getMinutes()).slice(-2);
			var seconds = ('0' + d.getSeconds()).slice(-2);
			dt = dt.replace(/R/g, d.getHours() + ':' + minutes + ':' + seconds);
		}
		if (dt.indexOf('r') !== -1) { // time in digits 12-hours format 11:05:12 AM
			var am_pm = d.getHours() >= 12 ? 'PM' : 'AM';
			var hours = d.getHours() % 12;
			hours = hours ? hours : 12;
			var minutes = ('0' + d.getMinutes()).slice(-2);
			var seconds = ('0' + d.getSeconds()).slice(-2);
			dt = dt.replace(/r/g, hours + ':' + minutes + ':' + seconds + ' ' + am_pm);
		}
		return dt;
	},
	format_date_str: function(date) {
		var d = date;
		if (/^\d{4}-\d{2}-\d{2} \d{1,2}:\d{2}:\d{2}$/.test(d)) {
			var t = date_time_to_ts(d);
			d = Units.format_date(t);
		}
		return d;
	},
	format_speed: function(speed_bytes, format, float_val, decimal) {
		var reminder;
		var f = this.units.speed[0].long;
		for (var i = 0; i < this.units.speed.length; i++) {
			if (this.units.speed[i].long != format && speed_bytes) {
				if (decimal && [1, 1000].indexOf(this.units.speed[i].value) == -1) {
					continue;
				}
				reminder = speed_bytes % this.units.speed[i].value;
				if (reminder === 0 || (float_val && speed_bytes >= this.units.speed[i].value)) {
					speed_bytes /= this.units.speed[i].value;
					f = this.units.speed[i].long;
					continue;
				}
				break;
			}
		}
		return {value: speed_bytes, format: f};
	},
	get_short_unit_by_long: function(unit_type, unit_long) {
		var unit_short = '';
		for (var i = 0; i < this.units[unit_type].length; i++) {
			if (this.units[unit_type][i].long === unit_long) {
				unit_short = this.units[unit_type][i].short;
				break;
			}
		}
		return unit_short;
	}
}

var Strings = {
	limits: {
		label: 19
	},
	get_short_label: function(txt) {
		var short_txt = txt;
		var cut = ((this.limits.label - 2) / 2);
		if (txt.length > this.limits.label) {
			short_txt = txt.substr(0, cut) + '..' + txt.substr(-cut);
		}
		return short_txt;
	}
}

var PieGraph  = {
	pie_label_formatter: function (total, value) {
		var percents =  (100 * value / total).toFixed(1);
		if (percents >= 1) {
			percents = percents.toString() + '%';
		} else {
			percents = '';
		}
		return percents;
	},
	pie_track_formatter: function(e) {
		return e.series.label;
	}
}

var Formatters = {
	formatter: [
		{css_class: 'size', format_func: function(val) { return Units.get_formatted_size(val); }},
		{css_class: 'time', format_func: function(val) { return Units.format_time_period(val); }},
		{css_class: 'datetime', format_func: function(val) { return Units.format_date_str(val); }},
		{css_class: 'udatetime', format_func: function(val) { return Units.format_date(val); }}
	],
	set_formatters: function() {
		var elements, formatter, txt, val;
		for (var i = 0; i < Formatters.formatter.length; i++) {
			elements = document.getElementsByClassName(Formatters.formatter[i].css_class);
			formatter = Formatters.formatter[i].format_func;
			for (var j = 0; j < elements.length; j++) {
				txt = elements[j].firstChild;
				if (txt && txt.nodeType === 3) {
					val = formatter(txt.nodeValue);
					if (typeof(val) === 'object' && val.hasOwnProperty('value') && val.hasOwnProperty('format')) {
						txt.nodeValue = val.value + ' ' + val.format + ((val.value > 0) ? 's' : '');
					} else {
						txt.nodeValue = val;
					}
				}
			}
		}
	}
}

function date_time_to_ts(datetime) {
	var d = datetime;
	if (/^\d{4}-\d{2}-\d{2} \d{1,2}:\d{2}:\d{2}$/.test(d)) {
		var dati = datetime.split(' ');
		var da = dati[0].split('-');
		var ti = dati[1].split(':');
		d = (new Date(da[0], (da[1] - 1), da[2], ti[0], ti[1], ti[2], 0)).getTime();
	}
	return d;
}

/** Data tables formatters **/

function render_date(data, type, row) {
	var t = data;
	if (t) {
		var d = date_time_to_ts(t);
		if (type == 'display') {
			t = Units.format_date(d);
		} else {
			t = d;
		}
	}
	return t;
}

function render_date_ts(data, type, row) {
	var t;
	if (type == 'display' || type == 'filter') {
		t = Units.format_date(data)
	} else {
		t = data;
	}
	return t;
}

function render_date_ex(data, type, row) {
	var t = data;
	if (t && t != 'no date') {
		var d = (new Date(t)).getTime();
		if (type == 'display') {
			t = Units.format_date(d);
		} else {
			t = d;
		}
	}
	return t;
}

function render_jobstatus(data, type, row) {
	var ret;
	if (type == 'display') {
		ret = JobStatus.get_icon(data).outerHTML;
	} else {
		ret = data;
	}
	return ret;
}

function render_bytes(data, type, row) {
	var s;
	if (type == 'display') {
		s = Units.get_formatted_size(data)
	} else {
		s = data;
	}
	return s;
}
function render_level(data, type, row) {
	var ret;
	if (!data) {
		ret = '-';
	} else {
		ret = JobLevel.get_level(data);
	}
	return ret;
}

function render_time_period(data, type, row) {
	var ret;
	if (type == 'display' || type == 'filter') {
		var time = Units.format_time_period(data);
		ret = time.value + ' ' + time.format + ((time.value > 0) ? 's': '');
	} else {
		ret = data;
	}
	return ret;
}

function render_string_short(data, type, row) {
	ret = data;
	if (type == 'display') {
		var span = document.createElement('SPAN');
		span.title = data;
		if (data.length > 40) {
			span.textContent = data.substring(0, 40) + '...';
		} else {
			span.textContent = data;
		}
		ret = span.outerHTML;
	} else {
		ret = data;
	}
	return ret;
}

function set_formatters() {
	Formatters.set_formatters();
}

var Cookies = {
	default_exipration_time: 31536000000, // 1 year in miliseconds
	set_cookie: function(name, value, expiration) {
		var date = new Date();
		date.setTime(date.getTime() + this.default_exipration_time);
		var expires = 'expires=' + date.toUTCString();
		document.cookie = name + '=' + value + '; ' + expires;
	},
	get_cookie: function(name) {
		name += '=';
		var values = document.cookie.split(';');
		var cookie_val = null;
		var value;
		for (var i = 0; i < values.length; i++) {
			value = values[i];
			while (value.charAt(0) == ' ') {
				value = value.substr(1);
			}
			if (value.indexOf(name) == 0) {
				cookie_val = value.substring(name.length, value.length);
				break;
			}
		}
		return cookie_val;
	}
}

var JobStatus = {
	st: {
		ok: ['T', 'D'],
		warning: ['W'],
		error: ['E', 'e', 'f', 'I'],
		cancel: ['A'],
		running: ['C', 'R']
	},

	is_ok: function(s) {
		return (this.st.ok.indexOf(s) !== -1);
	},
	is_warning: function(s) {
		return (this.st.warning.indexOf(s) !== -1);
	},
	is_error: function(s) {
		return (this.st.error.indexOf(s) !== -1);
	},
	is_cancel: function(s) {
		return (this.st.cancel.indexOf(s) !== -1);
	},
	is_running: function(s) {
		return (this.st.running.indexOf(s) !== -1);
	},
	get_icon: function(s) {
		var css = 'fa ';
		if (this.is_ok(s)) {
			css += 'fa-check-square w3-text-green';
		} else if (this.is_error(s)) {
			css += 'fa-exclamation-circle w3-text-red';
		} else if (this.is_running(s)) {
			css += 'fa-cogs w3-text-blue';
		} else if (this.is_cancel(s)) {
			css += 'fa-minus-square w3-text-yellow';
		} else if (this.is_warning(s)) {
			css += 'fa-exclamation-triangle w3-text-orange';
		} else {
			css += 'fa-question-circle w3-text-red';
		}
		css += ' w3-large';
		var ret = document.createElement('I');
		ret.className = css;
		ret.title = this.get_desc(s);
		return ret;
	},
	get_desc: function(s) {
		var desc;
		if (s == 'C') {
			desc = 'Created but not yet running';
		} else if (s == 'R') {
			desc = 'Running';
		} else if (s == 'B') {
			desc = 'Blocked';
		} else if (s == 'T') {
			desc = 'Terminated normally';
		} else if (s == 'W') {
			desc = 'Terminated normally with warnings';
		} else if (s == 'E') {
			desc = 'Terminated in Error';
		} else if (s == 'e') {
			desc = 'Non-fatal error';
		} else if (s == 'f') {
			desc = 'Fatal error';
		} else if (s == 'D') {
			desc = 'Verify Differences';
		} else if (s == 'A') {
			desc = 'Canceled by the user';
		} else if (s == 'I') {
			desc = 'Incomplete Job';

		/*
		 * Some statuses are used only internally by Bacula and
		 * they are not exposed to end interface.
		 */
		} else if (s == 'F') {
			desc = 'Waiting on the File daemon';
		} else if (s == 'S') {
			desc = 'Waiting on the Storage daemon';
		} else if (s == 'm') {
			desc = 'Waiting for a new Volume to be mounted';
		} else if (s == 'M') {
			desc = 'Waiting for a Mount';
		} else if (s == 's') {
			desc = 'Waiting for Storage resource';
		} else if (s == 'j') {
			desc = 'Waiting for Job resource';
		} else if (s == 'c') {
			desc = 'Waiting for Client resource';
		} else if (s == 'd') {
			desc = 'Wating for Maximum jobs';
		} else if (s == 't') {
			desc = 'Waiting for Start Time';
		} else if (s == 'p') {
			desc = 'Waiting for higher priority job to finish';
		} else if (s == 'i') {
			desc = 'Doing batch insert file records';
		} else if (s == 'a') {
			desc = 'SD despooling attributes';
		} else if (s == 'l') {
			desc = 'Doing data despooling';
		} else if (s == 'L') {
			desc = 'Committing data (last despool)';
		} else {
			desc = 'Unknown status';
		}
		return desc;
	},
	get_states: function() {
		var states = {};
		var keys = Object.keys(this.st);
		for (var i = 0; i < keys.length; i++) {
			for (var j = 0; j < this.st[keys[i]].length; j++) {
				states[this.st[keys[i]][j]] = {
					type: keys[i],
					value: this.get_desc(this.st[keys[i]][j])
				};
			}
		}
		return states;
	}
};

var JobLevel = {
	level: {
		'F': 'Full',
		'I': 'Incremental',
		'D': 'Differential',
		'B': 'Base',
		'f': 'VirtualFull',
		'V': 'InitCatalog',
		'C': 'Catalog',
		'O': 'VolumeToCatalog',
		'd': 'DiskToCatalog',
		'A': 'Data',
		' ': '-' // using by jobs without job level (ex. Admin job)
	},
	get_level: function(l) {
		var level;
		if (this.level.hasOwnProperty(l)) {
			level = this.level[l];
		} else {
			level = 'Unknown';
		}
		return level;
	}
};

var JobType = {
	type: {
		'B': 'Backup',
		'M': 'Migrated',
		'V': 'Verify',
		'R': 'Restore',
		'I': 'Internal',
		'D': 'Admin',
		'A': 'Archive',
		'C': 'Copy',
		'c': 'Copy Job',
		'g': 'Migration'
	},
	get_type: function(t) {
		var type;
		if (this.type.hasOwnProperty(t)) {
			type = this.type[t];
		} else {
			type = 'Unknown';
		}
		return type;
	}
};

var oLastJobsList = {
	last_jobs_table: null,
	ids: {
		last_jobs_list: 'last_jobs_list',
		last_jobs_list_body: 'lats_jobs_list_body'
	},
	init: function(data) {
		this.destroy();
		this.set_table(data);
	},
	destroy: function() {
		if (this.last_jobs_table) {
			this.last_jobs_table.destroy();
		}
	},
	set_table: function(data) {
		this.last_jobs_table = $('#' + this.ids.last_jobs_list).DataTable({
			data: data,
			bInfo: false,
			paging: false,
			searching: false,
			deferRender: true,
			columns: [
				{
					className: 'details-control',
					orderable: false,
					data: null,
					defaultContent: '<button type="button" class="w3-button w3-blue"><i class="fa fa-angle-down"></i></button>'
				},
				{
					data: 'jobid',
					responsivePriority: 1
				},
				{
					data: 'name',
					responsivePriority: 2
				},
				{
					data: 'level',
					render: function(data, type, row) {
						return (['R', 'D'].indexOf(row.type) === -1 ? JobLevel.get_level(data) : '-');
					},
					responsivePriority: 3
				},
				{
					data: 'starttime',
					render: render_date,
					responsivePriority: 5
				},
				{
					data: 'jobstatus',
					render: render_jobstatus,
					responsivePriority: 4
				}
			],
			responsive: {
				details: {
					type: 'column'
				}
			},
			columnDefs: [{
				className: 'control',
				orderable: false,
				targets: 0
			},
			{
				className: "dt-center",
				targets: [ 1, 3, 4, 5 ]
			}],
			order: [1, 'desc']
		});
		this.set_events();
	},
	set_events: function() {
		var self = this;
		$('#' + this.ids.last_jobs_list + ' tbody').on('click', 'tr', function (e) {
			var data = self.last_jobs_table.row(this).data();
			document.location.href = '/web/job/history/' + data.jobid + '/'
		});
	}
};

var Dashboard = {
	stats: null,
	txt: null,
	pie: null,
	noval: '-',
	ids: {
		clients: {
			no: 'client_no',
			most: 'client_most',
			jobs: 'client_jobs'
		},
		jobs: {
			no: 'job_no',
			most: 'job_most',
			most_count: 'job_most_count'
		},
		jobtotals: {
			total_bytes: 'jobs_total_bytes',
			total_files: 'jobs_total_files'
		},
		database: {
			type: 'database_type',
			size: 'database_size'
		},
		pools: {
			no: 'pool_no',
			most: 'pool_most',
			jobs: 'pool_jobs'
		},
		pie_summary: 'jobs_summary_graph'
	},
	last_jobs_table: null,
	dbtype: {
		pgsql: 'PostgreSQL',
		mysql: 'MySQL',
		sqlite: 'SQLite'
	},
	update_all: function(statistics) {
		this.stats = statistics;
		this.update_pie_jobstatus();
		this.update_clients();
		this.update_jobs();
		this.update_job_access();
		this.update_pools();
		this.update_jobtotals();
		this.update_database();
	},
	update_clients: function() {
		var clients = this.stats.clients_occupancy;
		var most_occuped_client = this.noval;
		var occupancy = -1;
		for (client in clients) {
			if (occupancy < clients[client]) {
				most_occuped_client = client;
				occupancy = clients[client];
			}
		}

		if (occupancy === -1) {
			occupancy = 0;
		}

		document.getElementById(this.ids.clients.no).textContent = Object.keys(this.stats.clients).length;
		document.getElementById(this.ids.clients.most).setAttribute('title', most_occuped_client);
		document.getElementById(this.ids.clients.most).textContent = Strings.get_short_label(most_occuped_client);
		document.getElementById(this.ids.clients.jobs).textContent = occupancy;
	},
	update_job_access: function() {
		// get last 10 jobs
		var data = this.stats.jobs.slice(0, 10);
		oLastJobsList.init(data);
	},
	update_jobs: function() {
		var jobs = this.stats.jobs_occupancy;
		var most_occuped_job = this.noval;
		var occupancy = -1;
		for (job in jobs) {
			if (occupancy < jobs[job]) {
				most_occuped_job = job;
				occupancy = jobs[job];
			}
		}

		if (occupancy === -1) {
			occupancy = 0;
		}

		document.getElementById(this.ids.jobs.no).textContent = Object.keys(this.stats.jobs).length;
		document.getElementById(this.ids.jobs.most).setAttribute('title',most_occuped_job);
		document.getElementById(this.ids.jobs.most).textContent = Strings.get_short_label(most_occuped_job);
		document.getElementById(this.ids.jobs.most_count).textContent = occupancy;
	},
	update_jobtotals: function() {
		document.getElementById(this.ids.jobtotals.total_bytes).textContent = Units.get_formatted_size(this.stats.jobtotals.bytes);
		document.getElementById(this.ids.jobtotals.total_files).textContent = this.stats.jobtotals.files || 0;
	},
	update_database: function() {
		if (this.stats.dbsize.dbsize) {
			document.getElementById(this.ids.database.type).textContent = this.dbtype[this.stats.dbsize.dbtype];
			document.getElementById(this.ids.database.size).textContent = Units.get_formatted_size(this.stats.dbsize.dbsize);
		}
	},
	update_pools: function() {
		var pools = this.stats.pools_occupancy;
		var most_occuped_pool = this.noval;
		var occupancy = -1;
		for (pool in pools) {
			if (occupancy < pools[pool]) {
				most_occuped_pool = pool;
				occupancy = pools[pool];
			}
		}

		if (occupancy === -1) {
			occupancy = 0;
		}

		document.getElementById(this.ids.pools.no).textContent = Object.keys(this.stats.pools).length;
		document.getElementById(this.ids.pools.most).setAttribute('title', most_occuped_pool);
		document.getElementById(this.ids.pools.most).textContent = Strings.get_short_label(most_occuped_pool);
		document.getElementById(this.ids.pools.jobs).textContent = occupancy;
	},
	update_pie_jobstatus: function() {
		if (this.pie != null) {
			this.pie.pie.destroy();
		}
		this.pie = new GraphPieClass({
			jobs: this.stats.jobs_summary,
			container_id: this.ids.pie_summary
		});
	}
};

var MsgEnvelope = {
	ids: {
		envelope: 'msg_envelope',
		modal: 'msg_envelope_modal',
		container: 'msg_envelope_container',
		content: 'msg_envelope_content'
	},
	issue_regex: { // @TODO: add more regexes
		warning: [
			/Cannot find any appendable volumes/g,
			/Please mount read Volume/g,
			/Please mount append Volume/g
		],
		error: [
			/ERR=/g,
			/error: /ig
		]
	},
	init: function() {
		this.set_events();
		this.set_actions();
	},
	set_events: function() {
		document.getElementById(this.ids.envelope).addEventListener('click', function(e) {
			this.open();
			var container = document.getElementById(this.ids.container);
			// set scroll to the bottom
			container.scrollTop = container.scrollHeight;
		}.bind(this));
	},
	set_actions: function() {
		var monitor_func = function() {
			var is_bottom = false;
			var container = document.getElementById(this.ids.container);

			// detect if before adding content, scroll is at the bottom
			if (container.scrollTop === (container.scrollHeight - container.offsetHeight)) {
				is_bottom = true
			}

			// add logs
			var logs = oData.messages;
			MsgEnvelope.set_logs(logs);

			// set scroll to the bottom
			if (is_bottom) {
				container.scrollTop = container.scrollHeight;
			}
		}.bind(this);
		MonitorCallsInterval.push(monitor_func);
	},
	open: function() {
		document.getElementById(this.ids.modal).style.display = 'block';
	},
	close: function() {
		document.getElementById(this.ids.modal).style.display = 'none';
	},
	set_logs: function(logs) {
		this.find_issues(logs);
		document.getElementById(this.ids.content).innerHTML = logs.join("\n");
	},
	mark_envelope_error: function() {
		var envelope = document.getElementById(this.ids.envelope);
		if (envelope.classList.contains('w3-green')) {
			envelope.classList.replace('w3-green', 'w3-red');
		}
		if (envelope.classList.contains('w3-orange')) {
			envelope.classList.replace('w3-orange', 'w3-red');
		}
		envelope.querySelector('I').classList.add('blink');
	},
	mark_envelope_warning: function() {
		var envelope = document.getElementById(this.ids.envelope);
		if (envelope.classList.contains('w3-green')) {
			envelope.classList.replace('w3-green', 'w3-orange');
		}
		envelope.querySelector('I').classList.add('blink');
	},
	mark_envelope_ok: function() {
		var envelope = document.getElementById(this.ids.envelope);
		if (envelope.classList.contains('w3-red')) {
			envelope.classList.replace('w3-red', 'w3-green');
		}
		if (envelope.classList.contains('w3-orange')) {
			envelope.classList.replace('w3-orange', 'w3-green');
		}
		envelope.querySelector('I').classList.remove('blink');
	},
	find_issues: function(logs) {
		var error = warning = false;
		var logs_len = logs.length;
		for (var i = 0; i < logs_len; i++) {
			for (var j = 0; j < this.issue_regex.warning.length; j++) {
				if (this.issue_regex.warning[j].test(logs[i])) {
					logs[i] = '<span class="w3-orange">' + logs[i] + '</span>';
					warning = true;
					break;
				}
			}
			for (var j = 0; j < this.issue_regex.error.length; j++) {
				if (this.issue_regex.error[j].test(logs[i])) {
					logs[i] = '<span class="w3-red">' + logs[i] + '</span>';
					error = true;
					break;
				}
			}
		}

		if (error) {
			this.mark_envelope_error();
		} else if (warning) {
			this.mark_envelope_warning();
		}
	}
};

var W3SideBar = {
	ids: {
		sidebar: 'sidebar',
		overlay_bg: 'overlay_bg'
	},
	css: {
		page_main: '.page_main_el'
	},
	cookies: {
		side_bar_hide: 'baculum_side_bar_hide'
	},
	init: function() {
		this.sidebar = document.getElementById(this.ids.sidebar);
		if (!this.sidebar) {
			// don't initialize for pages without sidebar
			return;
		}
		this.overlay_bg = document.getElementById(this.ids.overlay_bg);
		this.page_main = $(this.css.page_main);
		var hide = Cookies.get_cookie(this.cookies.side_bar_hide);
		if (hide == 1) {
			this.close();
		}
		if (!this.sidebar) {
			// on pages without sidebar always show page main elements with 100% width.
			this.page_main.css({'margin-left': '0', 'width': '100%'});
		}
		this.set_events();
	},
	set_events: function() {
		if (this.sidebar) {
			this.sidebar.addEventListener('touchstart', handle_touch_start);
			this.sidebar.addEventListener('touchmove', function(e) {
				handle_touch_move(e, {
					'swipe_left': function() {
						this.close();
					}.bind(this)
				});
			}.bind(this));
		}
	},
	open: function() {
		if (this.sidebar.style.display === 'block' || this.sidebar.style.display === '') {
			this.close();
		} else {
			Cookies.set_cookie('baculum_side_bar_hide', 0);
			this.sidebar.style.display = 'block';
			this.overlay_bg.style.display = 'block';
			this.page_main.css({'margin-left': '250px', 'width': 'calc(100% - 250px)'});
		}
	},
	close: function() {
		Cookies.set_cookie('baculum_side_bar_hide', 1);
		this.sidebar.style.display = 'none';
		this.overlay_bg.style.display = 'none';
		this.page_main.css({'margin-left': '0', 'width': '100%'});
	}
};

W3TabsCommon = {
	open: function(btn_id, item_id) {
		var tab_items = document.getElementsByClassName(this.css.tab_item);
		for (var i = 0; i < tab_items.length; i++) {
			if (tab_items[i].id === item_id) {
				tab_items[i].style.display = 'block';
			} else {
				tab_items[i].style.display = 'none';
			}
		}
		var tab_btns = document.getElementsByClassName(this.css.tab_btn);
		for (var i = 0; i < tab_btns.length; i++) {
			if (tab_btns[i].id === btn_id && !tab_btns[i].classList.contains(this.css.tab_item_hover)) {
				tab_btns[i].classList.add(this.css.tab_item_hover);
			} else if (tab_btns[i].id !== btn_id && tab_btns[i].classList.contains(this.css.tab_item_hover)) {
				tab_btns[i].classList.remove(this.css.tab_item_hover);
			}
		}
	},
	is_open: function(item_id) {
		var display = document.getElementById(item_id).style.display;
		return (display === 'block' || display === '');
	}
};

W3Tabs = {
	css: {
		tab_btn: 'tab_btn',
		tab_item: 'tab_item',
		tab_item_hover: 'w3-grey'
	},
	open: function(btn_id, item_id) {
		W3TabsCommon.open.call(this, btn_id, item_id);
	},
	is_open: function(item_id) {
		return W3TabsCommon.is_open(item_id);
	}
};

W3SubTabs = {
	css: {
		tab_btn: 'subtab_btn',
		tab_item: 'subtab_item',
		tab_item_hover: 'w3-border-red'
	},
	open: function(btn_id, item_id) {
		W3TabsCommon.open.call(this, btn_id, item_id);
	}
};
var OAuth2Scopes = [
	'console',
	'jobs',
	'directors',
	'clients',
	'storages',
	'volumes',
	'pools',
	'bvfs',
	'joblog',
	'filesets',
	'schedules',
	'config',
	'status',
	'actions',
	'oauth2'
];
var set_scopes = function(field_id) {
	document.getElementById(field_id).value = OAuth2Scopes.join(' ');
}

function estimate_job(jobs, job, level) {
	var bytes = 0;
	var files = 0;
	var time = 0;
	var bytes_xy = 0;
	var files_xy = 0;
	var x2 = 0;
	var counter = 0;
	for (var i = 0; i < jobs.length; i++) {
		if (jobs[i].name === job && jobs[i].level === level) {
			if (jobs[i].jobbytes === 0 || jobs[i].jobfies === 0 || jobs[i].jobstatus !== 'T') {
				continue;
			}
			if (counter === 20) {
				break;
			}
			time += jobs[i].jobtdate;
			bytes += jobs[i].jobbytes;
			files += jobs[i].jobfiles;
			bytes_xy += jobs[i].jobtdate * jobs[i].jobbytes;
			files_xy += jobs[i].jobtdate * jobs[i].jobfiles;
			x2 += Math.pow(jobs[i].jobtdate, 2);
			counter++;
		}
	}
	var est;
	if (counter < 2) {
		est = {
			est_bytes: bytes,
			est_files: files
		};
	} else if (counter === 2) {
		est = {
			est_bytes: (bytes / 2),
			est_files: (files / 2)
		}
	} else {
		var divisor = (counter * x2 - Math.pow(time, 2));
		var bytes_slope = ((counter * bytes_xy) - (time * bytes)) / divisor;
		var files_slope = ((counter * files_xy) - (time * files)) / divisor;
		var bytes_intercept = (bytes / counter) - (bytes_slope * (time / counter));
		var files_intercept = (files / counter) - (files_slope * (time / counter));
		var est_bytes = bytes_intercept + (bytes_slope * parseInt((new Date).getTime() / 1000, 10));
		var est_files = files_intercept + (files_slope * parseInt((new Date).getTime() / 1000, 10));
		est = {
			est_bytes: est_bytes,
			est_files: est_files
		};
	}
	return est;
};

function get_url_param (name) {
	var url = window.location.href;
	var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)");
        var results = regex.exec(url);
	var ret;
	if (!results) {
		ret = null;
	} else if (!results[2]) {
		ret = '';
	} else {
		ret = results[2].replace(/\+/g, " ");
		ret = decodeURIComponent(ret);
	}
	return ret;
}

function openElementOnCursor(e, element, offsetX, offsetY) {
	if (!offsetX) {
		offsetX = 0;
	}
	if (!offsetY) {
		offsetY = 0;
	}
	var x = (e.clientX + offsetX).toString();
	var y = (e.clientY + offsetY + window.pageYOffset).toString();
	$('#' + element).css({
		position: 'absolute',
		left: x + 'px',
		top: y + 'px',
		zIndex: 1000
	});
	$('#' + element).show();
}


function get_random_string(allowed, len) {
	var random_string = "";
	for(var i = 0; i < len; i++) {
		random_string += allowed.charAt(Math.floor(Math.random() * allowed.length));
	}
	return random_string;
}

function clear_node(selector) {
	var node = $(selector);
	for (var i = 0; i < node.length; i++) {
		while (node[i].firstChild) {
			node[i].removeChild(node[i].firstChild);
		}
	}
}

function show_element(element, show) {
	if (show === true) {
		$(element).show('fast');
	} else {
		$(element).hide('fast');
	}
}

/**
 * Set compatibility with Send Bacula Backup Report tool.
 * @see https://giunchi.net/send-bacula-backup-report
 */
function set_sbbr_compatibility() {
	var open = get_url_param('open');
	var id = get_url_param('id');
	if (open && id) {
		open = open.toLowerCase();
		if (open === 'job') {
			open = 'job/history';
		}
		var url = '/web/%open/%id/'.replace('%open',  open.toLowerCase()).replace('%id', id);
		document.location.href = url;
	}
}

function set_icon_css() {
	/**
	 * Problem with shaking web font icons on Firefox.
	 * Workaround to solve shaking effect in spinning elements on Firefox.
	 * Note, both using w3-spin and fa-spin causes shaking, but only disabling
	 * for a micro time this effect (css) solves this issue.
	 */
	$('.w3-spin').removeClass('w3-spin').addClass('fa-spin');
}

var touch_start_x = null;
var touch_start_y = null;

function handle_touch_start(e) {
	// browser API or jQuery
	var first_touch =  e.touches || e.originalEvent.touches
	touch_start_x = first_touch[0].clientX;
	touch_start_y = first_touch[0].clientY;
}

function handle_touch_move(e, callbacks) {
	if (!touch_start_x || !touch_start_y || typeof(callbacks) !== 'object') {
		// no touch type event or no callbacks
		return;
	}

	var touch_end_x = e.touches[0].clientX;
	var touch_end_y = e.touches[0].clientY;

	var touch_diff_x = touch_start_x - touch_end_x;
	var touch_diff_y = touch_start_y - touch_end_y;

	if (Math.abs(touch_diff_x) > Math.abs(touch_diff_y)) {
		if (touch_diff_x > 0 && callbacks.hasOwnProperty('swipe_left')) {
			// left swipe
			callbacks.swipe_left();
		} else if (callbacks.hasOwnProperty('swipe_right')) {
			// right swipe
			callbacks.swipe_right();
		}
	} else {
		if (touch_diff_y > 0 && callbacks.hasOwnProperty('swipe_up')) {
			// up swipe
			callbacks.swipe_up()
		} else if (callbacks.hasOwnProperty('swipe_down')) {
			// down swipe
			callbacks.swipe_down()
		}
	}

	// reset values
	touch_start_x = null;
	touch_start_y = null;
}

function sort_natural(a, b) {
	a = a.toString();
	b = b.toString();
	return a.localeCompare(b, undefined, {numeric: true});
}

function update_job_table(table_obj, new_data) {

	var current_page = table_obj.page();

	var rows = table_obj.rows();
	var old_jobs = {};
	table_obj.data().toArray().forEach(function(job) {
		old_jobs[job.jobid] = job;
	});
	var new_jobs = {};
	new_data.forEach(function(job) {
		new_jobs[job.jobid] = job;
	});

	var job_add_mod = {};
	for (var jobid in new_jobs) {
		if (!old_jobs.hasOwnProperty(jobid) || new_jobs[jobid].jobstatus != old_jobs[jobid].jobstatus) {
			job_add_mod[jobid] = new_jobs[jobid];
		}
	}
	var job_rm = {};
	for (var jobid in old_jobs) {
		if (!new_jobs.hasOwnProperty(jobid)) {
			job_rm[jobid] = old_jobs[jobid];
		}
	}

	var rows_rm_idxs = [];
	var rows_list = rows.toArray();
	var jobid;
	for (var i = 0; i < rows_list[0].length; i++) {
		row = rows_list[0][i];
		jobid = table_obj.row(row).data().jobid
		if (job_add_mod.hasOwnProperty(jobid)) {
			// update modified row
			table_obj.row(row).data(job_add_mod[jobid]).draw();
			// remove modified jobs from table
			delete job_add_mod[jobid];
			continue;
		}
		if (job_rm.hasOwnProperty(jobid)) {
			// get rows to remove
			rows_rm_idxs.push(row);
			continue;
		}
	};

	// remove old rows
	if (rows_rm_idxs.length > 0) {
		table_obj.rows(rows_rm_idxs).remove().draw();
	}

	// add new rows
	for (var jobid in job_add_mod) {
		table_obj.row.add(job_add_mod[jobid]).draw();
	}

	table_obj.page(current_page).draw(false);
}

/**
 * Used to escape values before putting them into regular expression.
 * Dedicated to use in table values.
 */
dtEscapeRegex = function(value) {
	if (typeof(value) != 'string' && typeof(value.toString) == 'function') {
		value = value.toString();
	}
	return $.fn.dataTable.util.escapeRegex(value);
};

/**
 * Do validation comma separated list basing on regular expression
 * for particular values.
 */
function validate_comma_separated_list(value, regex) {
	var valid = true;
	var vals = value.split(',');
	var val;
	for (var i = 0; i < vals.length; i++) {
		val = vals[i].trim();
		if (!val || (regex && !regex.test(val))) {
			valid = false;
			break;
		}
	}
	return valid;
}

function get_table_toolbar(table, actions, txt) {
	var table_toolbar = document.querySelector('#' + table.table().node().id + '_wrapper div.table_toolbar');
	table_toolbar.className += ' dt-buttons';
	table_toolbar.style.display = 'none';
	var title = document.createTextNode(txt.actions);
	var select = document.createElement('SELECT');
	var option = document.createElement('OPTION');
	option.value = '';
	select.appendChild(option);
	var acts = {};
	for (var i = 0; i < actions.length; i++) {
		option = document.createElement('OPTION');
		option.value = actions[i].action,
		label = document.createTextNode(actions[i].label);
		option.appendChild(label);
		select.appendChild(option);
		acts[actions[i].action] = actions[i];
	}
	if (actions.length == 1) {
		select.selectedIndex = 1;
	}
	var btn = document.createElement('BUTTON');
	btn.type = 'button';
	btn.className = 'dt-button';
	btn.style.verticalAlign = 'top';
	label = document.createTextNode(txt.ok);
	btn.appendChild(label);
	btn.addEventListener('click', function(e) {
		if (!select.value) {
			// no value, no action
			return
		}
		var selected = [];
		var sel_data = table.rows({selected: true}).data();
		sel_data.each(function(v, k) {
			selected.push(v[acts[select.value].value]);
		});

		// call validation if defined
		if (acts[select.value].hasOwnProperty('validate') && typeof(acts[select.value].validate) == 'function') {
			if (acts[select.value].validate(sel_data) === false) {
				// validation error
				return false;
			}
		}
		// call pre-action before calling bulk action
		if (acts[select.value].hasOwnProperty('before') && typeof(acts[select.value].before) == 'function') {
			acts[select.value].before();
		}
		selected = selected.join('|');
		acts[select.value].callback.options.RequestTimeOut = 60000; // Timeout set to 1 minute
		acts[select.value].callback.setCallbackParameter(selected);
		acts[select.value].callback.dispatch();
	});
	table_toolbar.appendChild(title);
	table_toolbar.appendChild(select);
	table_toolbar.appendChild(btn);
	return table_toolbar;
}

function set_global_listeners() {
	document.addEventListener('keydown', function(e) {
		var key_code = e.keyCode || e.which;
		switch (key_code) {
			case 27: { // escape
				$('.w3-modal').filter(':visible').hide(); // hide modals
				break;
			}
		}
	});
}

$(function() {
	W3SideBar.init();
	set_sbbr_compatibility();
	set_icon_css();
	set_global_listeners();
});
