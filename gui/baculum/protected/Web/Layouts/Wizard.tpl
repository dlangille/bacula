<!DOCTYPE html>
<html lang="en">
	<com:THead Title="Baculum - Bacula Web Interface">
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<link rel="icon" href="<%=$this->getPage()->getTheme()->getBaseUrl()%>/favicon.ico" type="image/x-icon" />
	</com:THead>
	<body  class="w3-light-grey">
		<com:TForm>
			<com:TClientScript PradoScripts="ajax, effects" />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/fontawesome-all.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/datatables.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/dataTables.responsive.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/responsive.jqueryui.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/opentip.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/tooltip.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/misc.js %> />
			<!-- Top container -->
				<com:TContentPlaceHolder ID="Wizard" />
		</com:TForm>
	</body>
</html>
