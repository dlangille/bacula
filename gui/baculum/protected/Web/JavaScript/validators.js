var Validators = {
	requiredFieldsValidator: function(fields) {
		var valid = false;
		if (fields) {
			var j = 0;
			for (var i = 0; i < fields.length; i++) {
				if (fields[i] || fields[i] === 0) {
					j++;
				}
			}
			if (j === fields.length) {
				valid = true;
			}
		}
		return valid;
	}
}
