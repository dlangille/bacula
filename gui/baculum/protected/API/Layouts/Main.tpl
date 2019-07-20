<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<com:THead Title="Baculum - Bacula Web Interface" ShortcutIcon="<%=$this->getPage()->getTheme()->getBaseUrl()%>/favicon.ico" />
	<body class="api">
		<com:TForm>
			<com:TClientScript PradoScripts="effects" />
			<com:TContentPlaceHolder ID="Main" />
		</com:TForm>
		<footer class="footer"><%[ Version: ]%> <%=Params::BACULUM_VERSION%></footer>
	</body>
</html>
