var ConfigurationWindowClass = new Class.create({
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
		$(this.window_id).setStyle({'display' : 'block'});
		$$('div[id=' + this.window_id + '] input[type="submit"]').each(function(el) {
			el.observe('click', function() {
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
		$(this.window_id).setStyle({'display' : 'none'});
	},

	hideAll: function() {
		$$('div.configuration').each(function(el){
			el.setStyle({'display' : 'none'});
		});
	},

	progress: function(show) {
		if(show) {
			$(this.progress_id).setStyle({'display' : 'block'});
		} else {
			$(this.progress_id).setStyle({'display' : 'none'});
		}
	},

	is_progress: function() {
		return $(this.progress_id).getStyle('display') == 'block';
	},

	initTabs: function() {
		var show_elements = [];
		var element;
		var tabs = $$('div[id=' + this.window_id + '] span.tab');
		tabs.each(function(el) {
			element = el.readAttribute('rel');
			show_elements.push($(element));
			el.observe('click', function() {
				show_elements.invoke('hide');
				tabs.invoke('removeClassName', 'tab_active');
				el.addClassName('tab_active');
				var show_el = $(el.readAttribute('rel'));
				$(show_el).show();
			}.bind(this));
		}.bind(this));
	},

	switchTab: function(tab_rel) {
		var tabs = $$('div[id=' + this.window_id + '] span.tab');
		tabs.each(function(el) {
			element = el.readAttribute('rel');
			if (element == tab_rel) {
				el.addClassName('tab_active');
			} else {
				el.removeClassName('tab_active');
			}
			$(element).hide();
		});
		$(tab_rel).show();
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
