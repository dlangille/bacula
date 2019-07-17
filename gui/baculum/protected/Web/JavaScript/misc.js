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
			{short: 'kb/s', long: 'KB/s', value: 1000},
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
		var date = [d.getFullYear(), ('0' + (d.getMonth()+1)).slice(-2), ('0' + d.getDate()).slice(-2)].join('-');
		var time = [d.getHours(), ('0' + d.getMinutes()).slice(-2), ('0' + d.getSeconds()).slice(-2)].join(':');
		return (date + ' ' + time);
	},
	format_speed: function(speed_bytes, format, float_val, decimal) {
		var reminder;
		var f = this.units.speed[0].long;
		for (var i = 0; i < this.units.speed.length; i++) {
			if (this.units.speed[i].long != format && speed_bytes) {
				if (decimal && [1, 1000].indexOf(this.units.speed[i].value) == -1) {
					continue;
				}
				reminder = speed_bytes % this.units.speed[i].value
				if (reminder === 0 || (float_val && speed_bytes >= this.units.speed[i].value)) {
					speed_bytes /= this.units.speed[i].value;
					f = this.units.speed[i].long;
					continue;
				}
				break;
			}
		}
		var ret = {value: speed_bytes, format: f};
		return ret;
	},
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
		{css_class: 'time', format_func: function(val) { return Units.format_time_period(val); }}
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
		'A': 'Data'
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
						return JobLevel.get_level(data);
					},
					responsivePriority: 3
				},
				{
					data: 'starttime',
					responsivePriority: 5
				},
				{
					data: 'jobstatus',
					render: function (data, type, row) {
						var ret;
						if (type == 'display') {
							ret = JobStatus.get_icon(data).outerHTML;
						} else {
							ret = data;
						}
						return ret;
					},
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
			var node_name = e.target.nodeName.toUpperCase();
			if (node_name === 'BUTTON' || node_name === 'SVG' || node_name === 'PATH') {
				// clicking on button doesn't cause directing to job details
				return;
			}
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
		this.pie = new GraphPieClass(this.stats.jobs_summary, this.ids.pie_summary);
	}
}

var Users = {
	ids: {
		create_user: {
			add_user: 'add_user',
			add_user_btn: 'add_user_btn',
			newuser: 'newuser',
			newpwd: 'newpwd'
		},
		change_pwd: {
			rel_chpwd: 'chpwd',
			rel_chpwd_btn: 'chpwd_btn'
		},
		set_host: {
			rel_user_host: 'user_host_img'
		}
	},
	validators: {
		user_pattern: null
	},
	current_action: null,
	init: function() {
		this.setEvents();
	},
	setEvents: function() {
		document.getElementById(this.ids.create_user.add_user_btn).addEventListener('click', function(e) {
			$('#' + this.ids.create_user.add_user).show();
			$('#' + this.ids.create_user.newuser).focus();
		}.bind(this));
		document.getElementById(this.ids.create_user.newuser).addEventListener('keydown', function(e) {
			var target = e.target || e.srcElement;
			if (e.keyCode == 13) {
				this.addUser();
			} else if (e.keyCode == 27) {
				this.cancelAddUser();
			}
			return false;
		}.bind(this));
		document.getElementById(this.ids.create_user.newpwd).addEventListener('keydown', function(e) {
			var target = e.target || e.srcElement;
			if (e.keyCode == 13) {
				this.addUser();
			} else if (e.keyCode == 27) {
				this.cancelAddUser();
			}
			return false;
		}.bind(this));
	},
	userValidator: function(user) {
		user = user.replace(/\s/g, '');
		if (user == '') {
			alert(this.txt.enter_login);
			return false;
		}
		var valid = this.validators.user_pattern.test(user);
		if (valid === false) {
			alert(this.txt.invalid_login);
			return false;
		}
		return true;
	},
	pwdValidator: function(pwd) {
		var valid = pwd.length > 4;
		if (valid === false) {
			alert(this.txt.invalid_pwd);
		}
		return valid;
	},
	addUser: function() {
		var user = document.getElementById(this.ids.create_user.newuser);
		var pwd = document.getElementById(this.ids.create_user.newpwd);
		if (this.userValidator(user.value) === false) {
			return false;
		}
		if (this.pwdValidator(pwd.value) === false) {
			return false;
		}

		$('#' + this.ids.create_user.add_user).hide();
		this.action_callback('newuser', user.value, pwd.value);
		user.value = '';
		pwd.value = '';
		return true;
	},
	rmUser: function(user) {
		this.action_callback('rmuser', user);
	},
	showChangePwd: function(el) {
		$('a[rel=\'' + this.ids.change_pwd.rel_chpwd_btn + '\']').show();
		$('#' + el).hide();
		$('span[rel=\'' + this.ids.change_pwd.rel_chpwd + '\']').hide();
		$(el.nextElementSibling).show();
		$(el.nextElementSibling).find('input')[0].focus();
	},
	changePwd: function(el, user) {
		var pwd = el.value;

		if (this.pwdValidator(pwd) === false) {
			return false;
		}

		$(el.parentNode).hide();
		$(el.parentNode.previousElementSibling).show();
		this.action_callback('chpwd', user, pwd);
		return true;
	},
	set_host: function(user, select) {
		select.nextElementSibling.style.visibility = 'visible';
		this.action_callback('set_host', user, select.value);
	},
	hide_loader: function() {
		setTimeout(function() {
			if (this.current_action === 'set_host') {
				$('svg[rel=\'' + this.ids.set_host.rel_user_host + '\']').css({visibility: 'hidden'});
			}
		}.bind(this), 300);

	},
	cancelAddUser: function() {
		$('#' + this.ids.create_user.add_user).hide();
	},
	cancelChangePwd: function(el) {
		$(el.parentNode).hide();
		$(el.parentNode.previousElementSibling).show();
	}

};

var W3SideBar = {
	ids: {
		sidebar: 'sidebar',
		overlay_bg: 'overlay_bg',
		page_main: 'page_main'
	},
	cookies: {
		side_bar_hide: 'baculum_side_bar_hide'
	},
	init: function() {
		this.sidebar = document.getElementById(this.ids.sidebar);
		this.overlay_bg = document.getElementById(this.ids.overlay_bg);
		this.page_main = document.getElementById(this.ids.page_main);
		var hide = Cookies.get_cookie(this.cookies.side_bar_hide);
		if (hide == 1) {
			this.close();
		}
	},
	open: function() {
		if (this.sidebar.style.display === 'block' || this.sidebar.style.display === '') {
			Cookies.set_cookie('baculum_side_bar_hide', 1);
			this.sidebar.style.display = 'none';
			this.overlay_bg.style.display = 'none';
			this.page_main.style.marginLeft = 0;
		} else {
			Cookies.set_cookie('baculum_side_bar_hide', 0);
			this.sidebar.style.display = 'block';
			this.overlay_bg.style.display = 'block';
			this.page_main.style.marginLeft = '300px';
		}
	},
	close: function() {
		Cookies.set_cookie('baculum_side_bar_hide', 1);
		this.sidebar.style.display = 'none';
		this.overlay_bg.style.display = 'none';
		this.page_main.style.marginLeft = 0;
	}
};

W3Tabs = {
	css: {
		tab_btn: 'tab_btn',
		tab_item: 'tab_item'
	},
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
			tab_btns[i].className = 'w3-bar-item w3-button ' + this.css.tab_btn;
			if (tab_btns[i].id === btn_id) {
				tab_btns[i].className += ' w3-gray';
			}
		}
	}
}

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

$(function() {
	W3SideBar.init();
	set_sbbr_compatibility();
});
