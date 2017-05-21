var ConfigurationWindowClass = jQuery.klass({
	objects: {},

	initialize: function(id) {
		if(typeof(id) == "undefined") {
			return;
		}
		var prefix = id.replace('Window', 'Configuration');
		this.window_id = prefix + 'configuration';
		this.progress_id = 'configuration-progress';
		this.lock = false;
	},

	show: function() {
		this.hideAll();
		this.initTabs();
		document.getElementById(this.window_id).style.display = 'block';
		$('div[id=' + this.window_id + '] input[type="submit"]').each(function(index, el) {
			$(el).on('click', function() {
				this.progress(true);
			}.bind(this));
		}.bind(this));
	},

	objectExists: function(key) {
		return this.objects.hasOwnProperty(key);
	},

	registerObj: function(key, obj) {
		if(this.objectExists(key) === false) {
			this.objects[key] = obj;
		}
	},

	getObj: function(key) {
		var obj = null;
		if(this.objectExists(key) === true) {
			obj = this.objects[key];
		}
		return obj;
	},

	hide: function() {
		document.getElementById(this.window_id).style.display = 'none';
	},

	hideAll: function() {
		$('div.configuration').css({'display' : 'none'});
	},

	progress: function(show) {
		if(show) {
			document.getElementById(this.progress_id).style.display = 'block';
		} else {
			document.getElementById(this.progress_id).style.display = 'none';
		}
	},

	is_progress: function() {
		return (document.getElementById(this.progress_id).style.display == 'block');
	},

	initTabs: function() {
		var show_elements = [];
		var element;
		var tabs = $('div[id=' + this.window_id + '] span.tab');
		tabs.each(function(index, el) {
			element = $(el).attr('rel');
			show_elements.push($('#' + element));
			$(el).on('click', function() {
				for (var i = 0; i < show_elements.length; i++) {
					show_elements[i].hide();
				}
				tabs.removeClass('tab_active');
				$(el).addClass('tab_active');
				var show_el = $('#' + el.getAttribute('rel'));
				show_el.show();
			}.bind(this));
		}.bind(this));
	},

	switchTab: function(tab_rel) {
		var tabs = $('div[id=' + this.window_id + '] span.tab');
		tabs.each(function(index, el) {
			element = $(el).attr('rel');
			if (element == tab_rel) {
				$(el).addClass('tab_active');
			} else {
				$(el).removeClass('tab_active');
			}
			$('#' + element).hide();
		});
		$('#' + tab_rel).show();
	},

	switchTabByNo: function(tab_no) {
		var tab_rel;
		var tabs = $('div[id=' + this.window_id + '] span.tab');
		for (var i = 0, j = 1; i < tabs.length; i++, j++) {
			if (tab_no === j) {
				tab_rel = tabs[i].getAttribute('rel');
				break;
			}
		}

		if (tab_rel) {
			this.switchTab(tab_rel);
		}
	},

	openConfigurationWindow: function(slideWindowObj) {
		if(this.is_progress() === false) {
			this.progress(true);
			if(slideWindowObj.isFullSize() === true) {
				slideWindowObj.resetSize();
			}
		}
	}
});

var ConfigurationWindow = new ConfigurationWindowClass();
