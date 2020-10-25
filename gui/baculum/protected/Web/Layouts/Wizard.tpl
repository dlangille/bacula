<!DOCTYPE html>
<html lang="en">
	<com:THead Title="Baculum - Bacula Web Interface">
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<link rel="icon" href="<%=$this->getPage()->getTheme()->getBaseUrl()%>/favicon.ico" type="image/x-icon" />
	</com:THead>
	<body  class="w3-light-grey">
		<script type="text/javascript">
			var SIZE_VALUES_UNIT = '<%=(count($this->web_config) > 0 && key_exists('size_values_unit', $this->web_config['baculum'])) ? $this->web_config['baculum']['size_values_unit'] : WebConfig::DEF_SIZE_VAL_UNIT%>';
			var DATE_TIME_FORMAT = '<%=(count($this->web_config) > 0 && key_exists('date_time_format', $this->web_config['baculum'])) ? $this->web_config['baculum']['date_time_format'] : WebConfig::DEF_DATE_TIME_FORMAT%>';
		</script>
		<com:TForm>
			<com:TClientScript PradoScripts="ajax, effects" />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/fontawesome.min.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/datatables.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/dataTables.responsive.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/responsive.jqueryui.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/dataTables.buttons.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/buttons.html5.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/buttons.colVis.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/opentip.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/tooltip.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/misc.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/bacula-config.js %> />
			<!-- Top container -->
				<com:TContentPlaceHolder ID="Wizard" />
		</com:TForm>
	<script type="text/javascript">
		Formatters.set_formatters();
	</script>
	</body>
</html>
