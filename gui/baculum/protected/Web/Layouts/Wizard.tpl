<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<com:THead Title="Baculum - Bacula Web Interface" ShortcutIcon="<%=$this->getPage()->getTheme()->getBaseUrl()%>/favicon.ico" />
	<body>
		<com:TForm>
				<com:TClientScript PradoScripts="prado" />
				<com:TClientScript ScriptUrl=<%~ ../JavaScript/opentip.js %> />
				<com:TClientScript ScriptUrl=<%~ ../JavaScript/tooltip.js %> />
				<com:TClientScript ScriptUrl=<%~ ../JavaScript/misc.js %> />
				<com:TClientScript ScriptUrl=<%~ ../JavaScript/slide-window.js %> />
				<com:TContentPlaceHolder ID="Wizard" />
		</com:TForm>
	</body>
</html>
