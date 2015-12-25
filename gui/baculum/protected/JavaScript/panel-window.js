var PanelWindowClass = Class.create({

	currentWindowId: null,
	windowIds: ['dashboard', 'container', 'graphs'],
	onShow: null,

	initialize: function() {
		this.currentWindowId = this.windowIds[0];
	},

	hideOthers: function() {
		var hide_panel_by_id = function(id) {
			var el = $(id);
			if(el.visible() === true && id != this.currentWindowId) {
				Effect.toggle(el, 'slide', {
					duration: 0.3,
					afterFinish: function() {
						el.hide();
					}.bind(el)
				});
			}
		}
		for (var i = 0, j = 1; i < this.windowIds.length; i++, j++) {
			hide_panel_by_id(this.windowIds[i]);
		}
	},

	show: function(id) {
		if($(id).visible() === true) {
			return;
		}

		this.currentWindowId = id;
		Effect.toggle(id, 'slide', {
			duration: 0.3,
			beforeStart: function() {
				this.hideOthers();
			}.bind(this),
			afterFinish: function() {
				if (this.onShow) {
					this.onShow();
				}
				setContentWidth();
			}.bind(this)
		});
	}
});

var PanelWindow;
document.observe("dom:loaded", function() {
	PanelWindow = new PanelWindowClass();
});
