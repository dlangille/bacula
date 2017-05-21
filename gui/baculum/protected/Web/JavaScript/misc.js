var Units = {
	formats: [
		{format: 'second', value: 1},
		{format: 'minute', value: 60},
		{format: 'hour', value: 60},
		{format: 'day', value: 24}
	],
	units: ['K', 'M', 'G', 'T', 'P'],
	get_decimal_size: function(size) {
		var dec_size;
		var size_unit = 'B';
		var units = Units.units.slice(0);

		if (size === null) {
			size = 0;
		}

		var size_pattern = new RegExp('^[\\d\\.]+(' + units.join('|') + ')?' + size_unit + '$');

		if (size_pattern.test(size.toString())) {
			// size is already formatted
			dec_size = size;
		} else {
			size = parseInt(size, 10);
			var unit;
			dec_size = size.toString() + ((size > 0 ) ? size_unit : '');
			while(size >= 1000) {
				size /= 1000;
				unit = units.shift(units);
			}
			if (unit) {
				dec_size = (Math.floor(size * 10) / 10).toFixed(1);
				dec_size += unit + size_unit;
			}
		}
		return dec_size;
	},
	format_time_period: function(time_seconds, format) {
		var reminder;
		var f;
		for (var i = 0; i < this.formats.length; i++) {
			if (this.formats[i].format != format) {
				reminder = time_seconds % this.formats[i].value;
				if (reminder === 0) {
					time_seconds /= this.formats[i].value;
					f = this.formats[i].format;
					continue;
				}
				break;
			}
		}
		var ret = {value: time_seconds, format: f};
		return ret;
	}
}

var Strings = {
	limits: {
		label: 15
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
		{css_class: 'size', format_func: Units.get_decimal_size}
	],
	set_formatters: function() {
		var elements, formatter, txt;
		for (var i = 0; i < this.formatter.length; i++) {
			elements = document.getElementsByClassName(this.formatter[i].css_class);
			formatter = this.formatter[i].format_func;
			for (var i = 0; i < elements.length; i++) {
				txt = elements[i].firstChild;
				if (txt.nodeType === 3) {
					txt.nodeValue = formatter(txt.nodeValue);
				}
			}
		}
	}
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

var Dashboard = {
	stats: null,
	txt: null,
	pie: null,
	noval: '-',
	ids: {
		clients: {
			no: 'clients_no',
			most: 'clients_most',
			jobs: 'clients_jobs'
		},
		jobs: {
			to_view: 'jobs_to_view',
			most: 'jobs_most',
			most_count: 'jobs_most_count'
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
			no: 'pools_no',
			most: 'pools_most',
			jobs: 'pools_jobs'
		},
		pie_summary: 'jobs_summary_graph'
	},
	dbtype: {
		pgsql: 'PostgreSQL',
		mysql: 'MySQL',
		sqlite: 'SQLite'
	},
	update_all: function(statistics, txt) {
		this.stats = statistics;
		this.txt = txt;
		this.update_pie_jobstatus();
		this.update_clients();
		this.update_job_access();
		this.update_jobs();
		this.update_jobtotals();
		this.update_database();
		this.update_pools();
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
		var jobs_combobox= document.getElementById(this.ids.jobs.to_view);
		jobs_combobox.innerHTML = '';
		var last_jobs = this.stats.jobs.slice(0, 100);
		for (var i = 0; i < last_jobs.length; i++) {
			var opt = document.createElement('OPTION');
			var txt = '[' + last_jobs[i].jobid + '] ' + last_jobs[i].name + ' (' + this.txt.level + ': ' + last_jobs[i].level + ' ' + this.txt.status + ': ' + last_jobs[i].jobstatus + ' ' + this.txt.starttime + ': ' + last_jobs[i].starttime + ')';
			var label = document.createTextNode(txt);
			opt.value = last_jobs[i].jobid;
			opt.appendChild(label);
			jobs_combobox.appendChild(opt);
		}
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

		document.getElementById(this.ids.jobs.most).setAttribute('title',most_occuped_job);
		document.getElementById(this.ids.jobs.most).textContent = Strings.get_short_label(most_occuped_job);
		document.getElementById(this.ids.jobs.most_count).textContent = occupancy;
	},
	update_jobtotals: function() {
		document.getElementById(this.ids.jobtotals.total_bytes).textContent = Units.get_decimal_size(this.stats.jobtotals.bytes);
		document.getElementById(this.ids.jobtotals.total_files).textContent = this.stats.jobtotals.files || 0;
	},
	update_database: function() {
		document.getElementById(this.ids.database.type).textContent = this.dbtype[this.stats.dbsize.dbtype];
		document.getElementById(this.ids.database.size).textContent = Units.get_decimal_size(this.stats.dbsize.dbsize);
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
		if (PanelWindow.currentWindowId === 'dashboard') {
			if (this.pie != null) {
				this.pie.pie.destroy();
			}
			this.pie = new GraphPieClass(this.stats.jobs_summary, this.ids.pie_summary);
		}
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
		}
	},
	validators: {
		user_pattern: null
	},
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
				$(target.parentNode.getElementsByTagName('A')[0]).click();
			} else if (e.keyCode == 27) {
				this.cancelAddUser();
			}
			return false;
		}.bind(this));
		document.getElementById(this.ids.create_user.newpwd).addEventListener('keydown', function(e) {
			var target = e.target || e.srcElement;
			if (e.keyCode == 13) {
				$(target.nextElementSibling).click();
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
		var user = document.getElementById(this.ids.create_user.newuser).value;
		var pwd = document.getElementById(this.ids.create_user.newpwd).value;
		if (this.userValidator(user) === false) {
			return false;
		}
		if (this.pwdValidator(pwd) === false) {
			return false;
		}

		$('#' + this.ids.create_user.add_user).hide();
		this.action_callback('newuser', user, pwd);
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
		$(el.nextElementSibling).select('input')[0].focus();
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
	cancelAddUser: function() {
		$('#' + this.ids.create_user.add_user).hide();
	},
	cancelChangePwd: function(el) {
		$(el.parentNode).hide();
		$(el.parentNode.previousElementSibling).show();
	}

};

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
