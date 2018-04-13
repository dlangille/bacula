var BaculaConfigClass = jQuery.klass({
	show_item: function(element, show, after_finish) {
		var el = $(element);
		var duration;
		if (show === true && el.is(':visible') === false) {
			el.slideDown({duration: 400, complete: after_finish});
		} else if (show === false && el.is(':visible') === true) {
			el.slideUp({duration: 500, complete: after_finish});
		}
	},
	show_items: function(selector, show, callback) {
		$(selector).each(function(index, el) {
			var cb = function() {
				callback(el);
			}
			this.show_item(el, show, cb);
		}.bind(this));
	},
	show_new_config: function(sender, component_type, component_name, resource_type) {
		var container = $('div.config_directives[rel="' + sender + '"]');
		var h2 = container.find('h2');
		var text = h2[0].getAttribute('rel');
		text = text.replace('%component_type', component_type);
		text = text.replace('%component_name', component_name);
		text = text.replace('%resource_type', resource_type);
		h2[0].textContent = text;
		container.find('div.config_directives').show();
		this.show_item(container, true);
		this.scroll_to_element(container);
	},
	scroll_to_element: function(selector) {
		$('html,body').animate({scrollTop: $(selector).offset().top}, 'slow');
	},
	get_child_container: function(sender) {
		var child_container = $('#' + sender).closest('table').next('div');
		return child_container;
	},
	get_element_sender: function(el) {
		var element_sender = el.closest('table').prev('a');
		return element_sender;
	},
	set_config_items: function(id) {
		var child_container = this.get_child_container(id);
		var show = !child_container.is(':visible');
		this.show_item(child_container, show);
		this.loader_stop(id);
	},
	unset_config_items: function(id) {
		var child_container = this.get_child_container(id);
		var selector = [child_container[0].nodeName.toLowerCase(), child_container[0].className].join('.');
		var callback = function(el) {
			if (el.style.display === 'none' && !el.classList.contains('new_resource')) {
				// element closed
				var divs = el.getElementsByTagName('DIV');
				if (divs.length > 0) {
					var fc = divs[0];
					// clear by removing elements
					while (fc.firstChild) {
						fc.removeChild(fc.firstChild);
					}
				}
			}
		}
		this.show_items(selector, false, callback);
		/**
		 * Send request about items only if items are invisible to avoid
		 * asking about items when they are visible already.
		 */
		var send = !child_container.is(':visible');
		return send;
	},
	get_loader: function(id) {
		var loader = $('#' + id).next('img');
		return loader;
	},
	loader_start: function(id) {
		var loader = this.get_loader(id);
		$(loader).show();
	},
	loader_stop: function(id) {
		var loader = this.get_loader(id);
		$(loader).hide();
	}
});

var BaculaConfigOptionsClass = jQuery.klass({
	css: {
		options_container: 'directive_setting'
	},
	options_id: null,
	action_obj: null,
	initialize: function(opts) {
		if (opts.hasOwnProperty('options_id')) {
			this.options_id = opts.options_id;
		}
		if (opts.hasOwnProperty('action_obj')) {
			this.action_obj = opts.action_obj;
		}
		this.set_events();
	},
	set_events: function() {
		var element = document.getElementById(this.options_id);
		var opts = element.getElementsByTagName('LI');
		for (var i = 0; i < opts.length; i++) {
			opts[i].addEventListener('click', function(e) {
				var el = e.srcElement || e.target;
				var action = el.getAttribute('rel');
				this.do_action(action);
			}.bind(this));
		}
	},
	get_options: function() {
		;
	},
	do_action: function(param) {
		if (typeof(this.action_obj) === "object") {
			this.action_obj.setCallbackParameter(param);
			this.action_obj.dispatch();
		}
	}
});

var BaculaConfig = new BaculaConfigClass();
