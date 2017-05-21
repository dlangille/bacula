var SlideWindowClass = jQuery.klass({

	windowId: null,
	window: null,
	showEl: null,
	hideEl: null,
	fullSizeEl : null,
	search: null,
	toolbar: null,
	tools: null,
	titlebar: null,
	options: null,
	configurationObj: null,
	loadRequest : null,
	actionsRequest: null,
	repeaterEl: null,
	gridEl: null,
	checked: [],
	objects: {},
	windowSize: null,
	initElementId: null,

	size: {
		widthNormal : '53%',
		heightNormal : '325px',
		widthHalf : '53%',
		heightHalf : '586px',
		widthFull : '100%',
		heightFull : '586px',
		menuWidth: '75px'
	},

	elements : {
		content: 'div.slide-window-content',
		containerSuffix: '-slide-window-container',
		containerProgressSuffix: '-slide-window-progress',
		configurationWindows: 'div.configuration',
		configurationProgress: 'div.configuration-progress',
		contentItems: 'slide-window-element',
		contentAlternatingItems: 'slide-window-element-alternating',
		toolsButtonSuffix: '-slide-window-tools',
		optionsButtonSuffix: '-slide-window-options',
		actionsSuffix: '-slide-window-actions',
		toolbarSuffix: '-slide-window-toolbar',
		titleSuffix: '-slide-window-title',
		actionsButton : 'actions_btn'
	},

	initialize: function(windowId, data) {
		if(typeof(windowId) == "undefined") {
			return;
		}

		this.windowId = windowId;
		this.window = $('#' + this.windowId + this.elements.containerSuffix);
		this.tools = $('#' + this.windowId + this.elements.toolsButtonSuffix);
		this.options = $('#' + this.windowId + this.elements.optionsButtonSuffix);
		this.titlebar = $('#' + this.windowId + this.elements.titleSuffix);

		if(data.hasOwnProperty('showId')) {
				this.showEl = $('#' + data.showId);
		} else {
			alert('slide-window.js - "showId" property does not exists.');
			return false;
		}

		if(data.hasOwnProperty('hideId')) {
			this.hideEl = $('#' + data.hideId);
		} else {
			alert('slide-window.js - "hideId" property does not exists.');
			return false;
		}

		if(data.hasOwnProperty('fullSizeId')) {
			this.fullSizeEl = $('#' + data.fullSizeId);
		} else {
			alert('slide-window.js - "fullSizeId" property does not exists.');
			return false;
		}

		if(data.hasOwnProperty('search')) {
			this.search = $('#' + data.search);
		} else {
			alert('slide-window.js - "search" property does not exists.');
			return false;
		}
		this.setEvents();
		this.halfSizeWindow();
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

	setEvents: function() {
		this.showEl.on('click', function(){
			this.openWindow();
		}.bind(this));

		this.hideEl.on('click', function(){
			this.closeWindow();
		}.bind(this));
		
		this.fullSizeEl.on('click', function(){
			this.resetSize();
		}.bind(this));

		this.titlebar.on('dblclick', function() {
			this.resetSize();
		}.bind(this));

		this.search.on('keyup', function(){
			this.setSearch();
		}.bind(this));

		this.tools.on('click', function() {
			this.toggleToolbar();
		}.bind(this));

		this.options.on('click', function() {
			this.toggleToolbar();
		}.bind(this));

		this.setActionsBtnEvents();
	},

	setActionsBtnEvents: function() {
		var self = this;
		var actions_btn_container = this.window[0].getElementsByClassName(this.elements.actionsButton);
		if (actions_btn_container.length === 1) {
			var actions_btn = actions_btn_container[0].getElementsByTagName('INPUT');
			if (actions_btn.length === 1) {
				actions_btn[0].addEventListener('mouseup', function(e) {
					var row = self.getGridRowUnderCursor(e);
					var el = $(row).find('input[type=hidden]');
					if(el.length === 1) {
						self.actionsRequest.setCallbackParameter(el[0].value);
						self.actionsRequest.dispatch();
					}
				});
			}
		}
	},

	openWindow : function() {
		this.hideOtherWindows();
		this.window.slideToggle({duration: 100, done: function() {
				this.halfSizeWindow();
			}.bind(this)
		});
	},

	closeWindow : function() {
		this.window.slideToggle({done: function() {
				this.resetSize();
			}.bind(this)
		});
	},

	isWindowOpen: function() {
		return !(this.window[0].style.display === 'none');
	},

	showProgress: function(show) {
		var progress = $('#' + this.windowId + this.elements.containerProgressSuffix);
		if (show === true) {
			progress.css({display: 'block'});
		} else if (show === false) {
			progress.hide();
		}
	},

	resetSize : function() {
		if(this.isConfigurationOpen()) {
			if(this.isFullSize()) {
				this.halfSizeWindow();
			} else if(this.isHalfSize()) {
					this.normalSizeWindow();
			} else if (this.isNormalSize()){
				this.halfSizeWindow();
			} else {
				this.normalSizeWindow();
			}
		} else {
			if(this.isFullSize()) {
				this.normalSizeWindow();
			} else if(this.isHalfSize() || this.isNormalSize()) {
				this.fullSizeWindow();
			}
		}
	},

	isNormalSize: function() {
		return (this.windowSize == this.size.widthNormal && this.window.height()  + 'px' == this.size.heightNormal);
	},

	isHalfSize: function() {
		return (this.windowSize == this.size.widthHalf && this.window.height()  + 'px' == this.size.heightHalf);
	},

	isFullSize: function() {
		return (this.windowSize  == this.size.widthFull && this.window.height()  + 'px' == this.size.heightFull);
	},

	normalSizeWindow: function() {
			this.window.animate({width: this.size.widthNormal, height: this.size.heightNormal}, {duration : 400});
			this.windowSize = this.size.widthNormal;
	},
	
	halfSizeWindow: function() {
			this.window.animate({width: this.size.widthHalf, height: this.size.heightHalf}, {duration : 400});
			this.windowSize = this.size.widthHalf;
	},
	
	fullSizeWindow: function() {
			this.window.animate({width: this.size.widthFull, height: this.size.heightFull}, {duration : 400});
			this.windowSize = this.size.widthFull;
	},

	hideOtherWindows: function() {
		$('.slide-window-container').css({
			display : 'none',
			width : this.size.widthNormal,
			height : this.size.heightNormal
		});
	},

	setConfigurationObj: function(obj) {
		this.configurationObj = obj;
	},

	setWindowElementsEvent: function(opts) {
		this.repeaterEl = opts.repeater_id + '_Container';
		this.gridEl = opts.grid_id;
		this.loadRequest = opts.request_obj;
		if (opts.hasOwnProperty('actions_obj')) {
			this.actionsRequest = opts.actions_obj;
		}

		if (this.initElementId) {
			this.openConfigurationById(this.initElementId);
			this.initElementId = null;
			// for open window by init element, default set second tab
			this.configurationObj.switchTabByNo(2);
		}

		this.showProgress(false);
		this.markAllChecked(false);
		this.setLoadRequest();
		this.postWindowOpen();
	},

	setLoadRequest: function() {
		var dataList = [];
		var repeater = $('#' + this.repeaterEl);
		var grid = $('#' + this.gridEl);
		if(grid.length === 1) {
			dataList = grid.find('tr');
			this.makeSortable();
		} else if (repeater) {
			dataList = repeater.find('div.slide-window-element');
		}

		var set_callback_parameter = function(element) {
			var el = $(element).find('input[type=hidden]')
			if(el.length === 1) {
				var val = el[0].value;
				this.openConfigurationById(val);
			}
		}.bind(this);
		this.setSearch();
		dataList.each(function(index, tr) {
			$(tr).on('click', function(event) {
				var target = event.target || event.srcElement;
				var clicked = document.getElementById(target.id);
				// for element selection action (clicked checkbox) configuration window is not open
				if(clicked && clicked.hasAttribute('type') && clicked.getAttribute('type') == 'checkbox') {
					return;
				}
				set_callback_parameter(tr);
			}.bind(tr));
		}.bind(this));
		Formatters.set_formatters();
		this.revertSortingFromCookie();
	},

	openConfigurationById: function(id) {
		this.loadRequest.setCallbackParameter(id);
		this.loadRequest.dispatch();
		this.configurationObj.openConfigurationWindow(this);
        },

	isConfigurationOpen: function() {
		var is_open = false;
		$(this.elements.configurationWindows + ', '+ this.elements.configurationProgress).each(function(index, el) {
			if($(el).css('display') == 'block') {
				is_open = true;
				return false;
			}
		}.bind(is_open));
		return is_open;
	},

	sortTable: function (grid_id, col, reverse, set_cookie) {
		var table = document.getElementById(grid_id);
		var tb = table.tBodies[0], tr = Array.prototype.slice.call(tb.rows, 0), i;
		reverse = -((+reverse) || -1);
		tr = tr.sort(function (a, b) {
			var val, val_a, val_b, el_a, el_b;
			el_a = a.cells[col].childNodes[1];
			el_b = b.cells[col].childNodes[1];
			if (el_a && el_b && el_a.nodeType === 1 && el_b.nodeType === 1 && el_a.hasAttribute('rel') && el_b.hasAttribute('rel')) {
				val_a = el_a.getAttribute('rel');
				val_b = el_b.getAttribute('rel');
			} else {
				val_a = a.cells[col].textContent.trim();
				val_b = b.cells[col].textContent.trim();
			}
			if (!isNaN(parseFloat(val_a)) && isFinite(val_a) && !isNaN(parseFloat(val_b)) && isFinite(val_b)) {
				val = val_a - val_b
			} else {
				val = val_a.localeCompare(val_b);
			}
			return reverse * (val);
		});
		var even;
		for (i = 0; i < tr.length; i++) {
			even = ((i % 2) == 0);
			if (even) {
				tr[i].className = this.elements.contentItems;
			} else {
				tr[i].className = this.elements.contentAlternatingItems;
			}
			tb.appendChild(tr[i]);
		}
		if (set_cookie === true) {
			Cookies.set_cookie(this.gridEl, col + ':' + reverse);
		}
	},

	makeSortable: function (grid) {
		var self = this;
		var grid_id, set_cookie;
		if (grid) {
			grid_id = grid;
			// for external grids (non-slide) do not remember sorting order
			set_cookie = false;
		} else {
			grid_id = this.gridEl;
			set_cookie = true;
		}
		var table = document.getElementById(grid_id);
		table.tHead.style.cursor = 'pointer';
		var th = table.tHead, i;
		th && (th = th.rows[0]) && (th = th.cells);
		if (th) {
			i = th.length;
		} else {
			return;
		}
		var downCounter = 0;
		// skip first column if in column header is input (checkbox for elements mark)
		if (th[0].childNodes[0].nodeName == "INPUT") {
			downCounter = 1;
		}
		while (--i >= downCounter) (function (i) {
			var dir = 1;
			th[i].addEventListener('click', function () {
				self.sortTable(grid_id, i, (dir = 1 - dir), set_cookie);
			});
		}(i));
	},

	revertSortingFromCookie: function() {
		var sorting = Cookies.get_cookie(this.gridEl);
		if (sorting != null) {
			var sort_param = sorting.split(':');
			var col = parseInt(sort_param[0], 10);
			var order = -(parseInt(sort_param[1], 10));
			this.sortTable(this.gridEl, col, order);
		}
	},

	setSearch: function() {
		var search_pattern = new RegExp(this.search[0].value, 'i');
		var repeater = $('#' + this.repeaterEl);
		var grid = $('#' + this.gridEl);
		if (repeater.length === 1) {
			repeater.find('div.' + this.elements.contentItems).each(function(index, value){

				if(search_pattern.test(value.childNodes[2].textContent) == false) {
					$(value).css({'display' : 'none'});
				} else {
					$(value).css({'display' : ''});
				}
			}.bind(search_pattern));
		}

		if (grid.length === 1) {
			grid.find('tbody tr').each(function(index, value) {
				var tds = $(value).find('td');
				var td;
				var found = false;
				for (var i = 0; i < tds.length; i++) {
					td = tds[i].textContent.trim();
					if(search_pattern.test(td) == true) {
						found = true;
						break;
					}
				}

				if(found === true) {
					$(value).show();
				} else {
					$(value).hide();
				}
			}.bind(search_pattern));
		}
	},
	setElementsCount : function() {
		var elements_count = $('div[id="' + this.windowId + this.elements.containerSuffix + '"] div.' + this.elements.contentItems).length || $('div[id="' + this.windowId + this.elements.containerSuffix + '"] tr.' + this.elements.contentItems + ', div[id="' + this.windowId + this.elements.containerSuffix + '"] tr.' + this.elements.contentAlternatingItems).length;
		var count_el = document.getElementById(this.windowId + this.elements.titleSuffix).getElementsByTagName('span')[0];
		count_el.textContent = ' (' + elements_count + ')';
	},
	toggleToolbar: function() {
		if (this.isToolbarOpen() === false) {
			this.markAllChecked(false);
		}
		$('#' + this.windowId + this.elements.toolbarSuffix).slideToggle({duration: 200});
	},
	isToolbarOpen: function() {
		return $('#' + this.windowId + this.elements.toolbarSuffix).is(':visible');
	},
	setActions: function() {
		var checkboxes = this.getCheckboxes();
		checkboxes.each(function(index, el) {
			$(el).on('change', function() {
				var is_checked = this.isAnyChecked(checkboxes);
				if(is_checked === true && !this.areActionsOpen()) {
					this.showActions();
				} else if (is_checked === false && this.areActionsOpen()) {
					this.hideActions();
				}
			}.bind(this));
                }.bind(this));
	},
	isAnyChecked: function(checkboxes) {
		var is_checked = false;
		$(checkboxes).each(function(index, ch) {
			if(ch.checked == true) {
				is_checked = true;
				return false;
			}
		});
		return is_checked;
	},

	getCheckboxes: function() {
		var grid = $('#' + this.gridEl);
		var checkboxes = [];
		if (grid.length === 1) {
			checkboxes = grid.find('input[name="actions_checkbox"]');
		}
		return checkboxes;
	},

	areCheckboxesChecked: function() {
		var checkboxes = this.getCheckboxes();
		return this.isAnyChecked(checkboxes);
	},

	markAllChecked: function(check) {
		this.checked = [];
		var checkboxes = this.getCheckboxes();
		var containerId;
		if(checkboxes.length > 0) {
			checkboxes.each(function(index, ch) {
				if ($(ch).parents('tr').is(':visible')) {
					containerId = ch.getAttribute('rel');
					if (ch.checked == false && check == true) {
						ch.checked = true;
					} else if (ch.checked == true && check == false) {
						ch.checked = false;
					}
					this.markChecked(containerId, ch.checked, ch.value);
				}
			}.bind(this));
			if (containerId) {
				this.packChecked(containerId);
			}
		}

		if(check) {
			this.showActions();
		} else {
			this.hideActions();
		}
	},
	markChecked: function(containerId, checked, param, pack) {
		if (this.checked.length == 0) {
			if(checked == true) {
				this.checked.push(param);
			}
		} else {
			index = this.checked.indexOf(param);
			if(checked === true && index == -1) {
				this.checked.push(param);
			} else if (checked === false && index > -1) {
				this.checked.splice(index, 1);
			}
		}

		if(checked == true) {
			this.showActions();
		} else if(this.checked.length == 0) {
			this.hideActions();
		}

		if (pack === true) {
			this.packChecked(containerId);
		}
	},
	packChecked: function(containerId) {
		var values_packed = this.checked.join(';');
		document.getElementById(containerId).value = values_packed;
	},
	showActions: function() {
		if (this.areActionsOpen()) {
			return;
		}
		if (this.isToolbarOpen()) {
			this.toggleToolbar();
		}
		$('#' + this.windowId + this.elements.actionsSuffix).slideDown({duration: 200});
	},
	hideActions: function() {
		if (!this.areActionsOpen()) {
			return;
		}
		this.checked = [];
		$('#' + this.windowId + this.elements.actionsSuffix).slideUp({duration: 200});
	},
	areActionsOpen: function() {
		return $('#' + this.windowId + this.elements.actionsSuffix).is(':visible');
	},
	postWindowOpen: function() {
		this.setActions();
		this.setElementsCount();
		this.setOptionsBtn();
	},
	setOptionsBtn: function() {
		var options_btn = this.window[0].getElementsByClassName(this.elements.actionsButton);
		var table_window = $('#' + this.windowId + this.elements.containerSuffix).children(this.elements.content);
		if (options_btn.length === 1) {
			var self = this;
			options_btn = options_btn[0];
			table_window.off('mouseover');
			table_window.on('mouseover', function(e) {
				var el = self.getGridRowUnderCursor(e);
				if (el.length === 1 && (el[0].className == self.elements.contentItems || el[0].className == self.elements.contentAlternatingItems)) {
					el[0].style.backgroundColor = '#aeb2b6';
					options_btn.style.display = '';
					var scroll_y = $(document).scrollTop();
					var y = ($(el).offset().top + scroll_y - 57).toString() + 'px';
					options_btn.style.top = y;
				} else {
					options_btn.style.display = 'none';
				}
			});
			table_window.off('mouseout');
			table_window.on('mouseout', function(e) {
				table_window.find('tr').each(function(index, el) {
					el.style.backgroundColor = '';
				});;
				options_btn.style.display = 'none';
			});
		}
	},
	getGridRowUnderCursor: function(e) {
		var x = e.clientX - 100;
		var y = e.clientY;
		var element_mouse_is_over = document.elementFromPoint(x, y);
		var el = [];
		var el_over = $(element_mouse_is_over);
		if (el_over.length === 1 && el_over[0].nodeName != 'TR') {
			el = el_over.children('tr');
			if (el.length === 0) {
				el = el_over.parents('tr');
			}
		}
		return el;
	},
	setInitElementId: function(id) {
		this.initElementId = id;
	},
	quickJumpToElement: function(id, btn_id, panel_obj) {
		this.setInitElementId(id);
		panel_obj.show('container');
		if (this.isWindowOpen() === true) {
			this.openConfigurationById(id);
		} else {
			$('#' + btn_id).click();
		}
	}
});

var SlideWindow = new SlideWindowClass()

$(function() {
	if(navigator.userAgent.search("MSIE") > -1  || navigator.userAgent.search("Firefox") > -1 || navigator.userAgent.search("Chrome") > -1) {
		$('input[type=checkbox], input[type=submit], input[type=radio], input[type=image], a').each(function(el) {
			$(el).on('focus', function() {
				el.blur();
			}.bind(el));
		});
	}
});

function setContentWidth() {
	var content_width = $('#container').width() - $('#workspace-menu-left').width() - 1;
	$('#content').css({'width': content_width + 'px'});
}


$(window).resize(function() {
	setContentWidth();
});

$(function() {
	setContentWidth();
});
