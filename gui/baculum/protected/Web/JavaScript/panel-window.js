var PanelWindowClass = jQuery.klass({

	currentWindowId: null,
	windowIds: ['dashboard', 'container', 'graphs', 'config', 'users'],
	onShow: null,

	initialize: function() {
		this.currentWindowId = this.windowIds[0];
	},

	hideOthers: function() {
		var hide_panel_by_id = function(id) {
			var el = $('#' + id);

			if(el.is(':visible') === true && id != this.currentWindowId) {
				el.slideToggle({
					duration: 300,
					done: function() {
						el.hide();
					}.bind(el)
				});
			}
		}.bind(this);
		for (var i = 0; i < this.windowIds.length; i++) {
			hide_panel_by_id(this.windowIds[i]);
		}
	},

	show: function(id) {
		if($('#' + id).is(':visible') === true) {
			return;
		}

		this.currentWindowId = id;
		$('#' + id).slideToggle({
			duration: 400,
			start: function() {
				this.hideOthers();
			}.bind(this),
			done: function() {
				if (this.onShow) {
					this.onShow();
				}
				setContentWidth();
			}.bind(this)
		});
	},

	setPanelHeight: function() {
		for (var i = 0; i < this.windowIds.length; i++) {
			var y = window.innerHeight - 130;
			document.getElementById(this.windowIds[i]).style.height = y.toString() + 'px';
		}
		// set slide window content height
		document.getElementById('content').style.height = (y-17) + 'px';
	}
});

var PanelWindow;
$(function() {
	PanelWindow = new PanelWindowClass();
	PanelWindow.setPanelHeight();
});

$(window).resize(function() {
	PanelWindow.setPanelHeight();
});
