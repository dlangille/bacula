<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<com:THead Title="Baculum - Bacula Web Interface" ShortcutIcon="<%=$this->getPage()->getTheme()->getBaseUrl()%>/favicon.ico" />
	<body id="message-body">
		<com:TForm>
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/fontawesome.min.js %> />
			<com:TContentPlaceHolder ID="Message" />
		</com:TForm>
	</body>
</html>
