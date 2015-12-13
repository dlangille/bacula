var Units = {
	get_decimal_size: function(size) {
		size = parseInt(size, 10);
		var size_unit = 'B';
		var units = ['K', 'M', 'G', 'T', 'P'];
		var unit;
		var dec_size = size.toString() + ((size > 0 ) ? size_unit : '');
		while(size >= 1000) {
			size /= 1000;
			unit = units.shift(units);
		}
		if (unit) {
			dec_size = (Math.floor(size * 10) / 10).toFixed(1);
			dec_size += unit + size_unit;
		}
		return dec_size;
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
