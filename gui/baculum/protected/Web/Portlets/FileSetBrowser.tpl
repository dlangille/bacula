<div class="w3-container">
	<div class="w3-container">
		<div class="w3-third"><com:TLabel ForControl="Client" Text="<%[ Client: ]%>" /></div>
		<div class="w3-third">
			<com:TActiveDropDownList
				ID="Client"
				CssClass="w3-select w3-border"
				Width="350px"
				CausesValidation="false"
				OnSelectedIndexChanged="selectClient"
				>
			</com:TActiveDropDownList>
		</div>
	</div>
	<p><%[ To browse Windows host please type in text field below drive letter as path, for example: C:/ ]%></p>
	<div class="w3-section w3-half">
		<input type="text" id="fileset_browser_path" class="w3-input w3-twothird w3-border" placeholder="<%[ Go to path ]%>" />
		<button type="button" class="w3-button w3-green" onclick="oFileSetBrowser.ls_items(document.getElementById('fileset_browser_path').value);"><i class="fa fa-check"></i> &nbsp;<%[ OK ]%></button>
	</div>
	<div class="w3-section w3-half">
		<input type="text" id="fileset_browser_add_include_path" class="w3-input w3-twothird w3-border w3-margin-left" placeholder="<%[ Add new include path ]%>" onkeypress="oFileSetBrowser.add_include_path_by_input(event);" autocomplete="off" />
		<button type="button" class="w3-button w3-green" onclick="oFileSetBrowser.add_include_path();"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
	</div>
	<div id="fileset_browser_file_container" class="w3-container w3-half w3-border"></div>
	<div id="fileset_browser_include_container" class="w3-container w3-half w3-border"></div>
	<div class="w3-section w3-half">
		<input type="text" id="fileset_browser_add_exclude_path" class="w3-input w3-twothird w3-border w3-margin-left" placeholder="<%[ Add new global exclude path ]%>" onkeypress="oFileSetBrowser.add_exclude_path_by_input(event);" autocomplete="off" />
		<button type="button" class="w3-button w3-green" onclick="oFileSetBrowser.add_exclude_path();"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
	</div>
	<div id="fileset_browser_exclude_container" class="w3-container w3-half w3-border"></div>
	<com:TCallback ID="FileSetBrowserFiles" OnCallback="TemplateControl.getItems" />
</div>
<script type="text/javascript">
var oFileSetBrowser = {
	file_content: null,
	include_content: null,
	path_field: null,
	path: [],
	ids: {
		file_container: 'fileset_browser_file_container',
		include_container: 'fileset_browser_include_container',
		exclude_container: 'fileset_browser_exclude_container',
		path_field: 'fileset_browser_path',
		add_include_path_field: 'fileset_browser_add_include_path',
		add_exclude_path_field: 'fileset_browser_add_exclude_path'
	},
	css: {
		item: 'item',
		item_included: 'item_included',
		item_excluded: 'item_excluded',
		item_inc_exc_btn: 'item_inc_exc_btn',
		item_selected_del_btn: 'item_selected_del_btn',
		item_name: 'item_name',
		dir_img: 'dir_item_img',
		file_img: 'file_item_img',
		link_img: 'link_item_img'
	},
	init: function() {
		this.file_content = document.getElementById(this.ids.file_container);
		this.include_content = document.getElementById(this.ids.include_container);
		this.exclude_content = document.getElementById(this.ids.exclude_container);
		this.path_field = document.getElementById(this.ids.path_field);
		this.make_droppable();
	},
	reset: function() {
		this.clear_content();
		this.clear_includes();
		this.path_field.value = '';
		document.getElementById('<%=$this->Client->ClientID%>').value = '';
	},
	ls_items: function(path) {
		var dpath;
		if (path) {
			this.set_path(path);
			dpath = path;
		} else {
			dpath = this.get_path();
		}
		var request = <%=$this->FileSetBrowserFiles->ActiveControl->Javascript%>;
		request.setCallbackParameter(dpath);
		request.dispatch();
	},
	set_content: function(content) {
		this.clear_content();

		var items = JSON.parse(content);
		items.sort(function(a, b) {
			if (a.type === 'd' && b.type !== 'd') {
				return -1;
			} else if (a.type !== 'd' && b.type === 'd') {
				return 1;
			}
			return a.item.localeCompare(b.item, undefined, { numeric: true, sensitivity: 'base' });
		});

		for (var i = 0; i < items.length; i++) {
			this.set_item(items[i]);
		}
		this.make_draggable();
	},
	clear_content: function() {
		while (this.file_content.firstChild) {
			this.file_content.removeChild(this.file_content.firstChild);
		}
	},
	make_draggable: function() {
		$('.' + this.css.item).draggable({
			helper: "clone"
		});
	},
	make_droppable: function() {
		$('#' + this.ids.include_container).droppable({
			accept: '.' + this.css.item,
			drop: function(e, ui) {
				var path = ui.helper[0].getAttribute('rel');
				this.add_include(path);
			}.bind(this)
		});
		$('#' + this.ids.exclude_container).droppable({
			accept: '.' + this.css.item,
			drop: function(e, ui) {
				var path = ui.helper[0].getAttribute('rel');
				this.add_exclude(path);
			}.bind(this)
		});
	},
	set_item: function(item) {
		var pattern = new RegExp('^' + this.get_path() + '/?');
		var item_name = item.item;
		if (item_name.substr(0, 1) !== '/') {
			item_name += '/';
		}
		if (item_name !== this.get_path()) {
			item_name = item.item.replace(pattern, '');
		} else {
			item_name = '.';
		}
		var el = document.createElement('DIV');
		el.className = this.css.item;
		el.setAttribute('rel', item.item);
		var title = item_name;
		var img = document.createElement('DIV');
		if (item.type === 'd') {
			img.className = this.css.dir_img;
			el.addEventListener('click', function(e) {
				var path = el.getAttribute('rel');
				this.set_path(path);
				this.ls_items(path);
			}.bind(this));
		} else if (item.type === '-') {
			img.className = this.css.file_img;
		} else if (item.type === 'l') {
			img.className = this.css.link_img;
			title = item_name + item.dest;
		}
		var name = document.createElement('DIV');
		name.className  = this.css.item_name;
		name.textContent = item_name;

		var include_btn = document.createElement('A');
		include_btn.className = this.css.item_inc_exc_btn;
		var include_btn_txt = document.createTextNode('<%[ Include ]%>');
		include_btn.appendChild(include_btn_txt);
		include_btn.addEventListener('click', function(e) {
			e.stopPropagation();
			this.add_include(item.item);
			return false;
		}.bind(this));

		var exclude_btn = document.createElement('A');
		exclude_btn.className = this.css.item_inc_exc_btn;
		var exclude_btn_txt = document.createTextNode('<%[ Exclude ]%>');
		exclude_btn.appendChild(exclude_btn_txt);
		exclude_btn.addEventListener('click', function(e) {
			e.stopPropagation();
			this.add_exclude(item.item);
			return false;
		}.bind(this));

		el.setAttribute('title',title);

		el.appendChild(img);
		el.appendChild(name);
		el.appendChild(exclude_btn);
		el.appendChild(include_btn);
		this.file_content.appendChild(el);
		if (item_name === '.' && this.get_path() !== '/') {
			this.set_special_items();
		}
	},
	set_special_items: function() {
		var item_name = '..';
		var el = document.createElement('DIV');
		el.className = this.css.item;
		el.setAttribute('rel', item_name);
		var img = document.createElement('DIV');
		img.className = this.css.dir_img;
		var name = document.createElement('DIV');
		name.className  = this.css.item_name;
		name.textContent = item_name;

		el.addEventListener('click', function(e) {
			var path = el.getAttribute('rel');
			this.set_path(path);
			this.ls_items();
		}.bind(this));

		el.setAttribute('title', item_name);

		el.appendChild(img);
		el.appendChild(name);
		this.file_content.appendChild(el);
	},
	get_includes: function() {
		var container = document.getElementById(this.ids.include_container);
		var inc_elements = container.querySelectorAll('div.' + this.css.item_included);
		var includes = [];
		for (var i = 0; i < inc_elements.length; i++) {
			includes.push(inc_elements[i].getAttribute('rel'));
		}
		return includes;
	},
	get_excludes: function() {
		var container = document.getElementById(this.ids.exclude_container);
		var exc_elements = container.querySelectorAll('div.' + this.css.item_excluded);
		var excludes = [];
		for (var i = 0; i < exc_elements.length; i++) {
			excludes.push(exc_elements[i].getAttribute('rel'));
		}
		return excludes;
	},
	add_include: function(item) {
		var el = document.createElement('DIV');
		el.className = this.css.item_included;
		el.setAttribute('rel', item);
		var name = document.createElement('DIV');
		name.textContent = item;
		/**
		 * Container is needed, because <i> is replaced to SVG icon
		 * and it is impossible to set click event.
		 */
		var del_btn_container = document.createElement('DIV');
		var del_btn = document.createElement('I');
		del_btn.className = 'fa fa-trash-alt item_selected_del_btn';
		del_btn_container.appendChild(del_btn);
		del_btn_container.addEventListener('click', function(e) {
			el.parentNode.removeChild(el);
		})
		el.appendChild(del_btn_container);
		el.appendChild(name);
		this.include_content.appendChild(el);
	},
	add_exclude: function(item) {
		var el = document.createElement('DIV');
		el.className = this.css.item_excluded;
		el.setAttribute('rel', item);
		var name = document.createElement('DIV');
		name.textContent = item;
		/**
		 * Container is needed, because <i> is replaced to SVG icon
		 * and it is impossible to set click event.
		 */
		var del_btn_container = document.createElement('DIV');
		var del_btn = document.createElement('I');
		del_btn.className = 'fa fa-trash-alt item_selected_del_btn';
		del_btn_container.appendChild(del_btn);
		del_btn_container.addEventListener('click', function(e) {
			el.parentNode.removeChild(el);
		})
		el.appendChild(del_btn_container);
		el.appendChild(name);
		this.exclude_content.appendChild(el);
	},
	clear_includes: function() {
		while (this.include_content.firstChild) {
			this.include_content.removeChild(this.include_content.firstChild);
		}
	},
	clear_excludes: function() {
		while (this.exclude_content.firstChild) {
			this.exclude_content.removeChild(this.exclude_content.firstChild);
		}
	},
	set_path: function(item) {
		var path = item.split('/');
		if (path.length === 1) {
			if (item === '..') {
				this.path.pop();
			} else {
				this.path.push(item);
			}
		} else {
			this.path = path;
		}
		this.path_field.value = this.get_path();
	},
	get_path: function() {
		var path = this.path.join('/');
		if (!path) {
			path += '/';
		}
		return path;
	},
	add_include_path_by_input: function(e) {
		var evt = e || window.event;
		var key_code = evt.keyCode || evt.which;
		if (key_code === 13) {
			this.add_include_path();
		}
	},
	add_include_path: function() {
		var el = document.getElementById(this.ids.add_include_path_field);
		if (el.value) {
			this.add_include(el.value);
			el.value = '';
			el.focus();
		}
	},
	add_exclude_path_by_input: function(e) {
		var evt = e || window.event;
		var key_code = evt.keyCode || evt.which;
		if (key_code === 13) {
			this.add_exclude_path();
		}
	},
	add_exclude_path: function() {
		var el = document.getElementById(this.ids.add_exclude_path_field);
		if (el.value) {
			this.add_exclude(el.value);
			el.value = '';
			el.focus();
		}
	}
};

function FileSetBrowser_set_content(content) {
	oFileSetBrowser.set_content(content);
}
$(function() {
	oFileSetBrowser.init();
});
</script>
