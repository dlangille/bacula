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
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/fontawesome.min.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/datatables.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/dataTables.responsive.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/responsive.jqueryui.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/dataTables.buttons.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/buttons.html5.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/buttons.colVis.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/dataTables.select.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/misc.js %> />
			<!-- Top container -->
			<div class="w3-bar w3-top w3-black w3-large" style="z-index: 4">
				<button type="button" class="w3-bar-item w3-button w3-hover-none w3-hover-text-light-grey" onclick="W3SideBar.open();"><i class="fa fa-bars"></i>  Menu</button>
				<span class="w3-bar-item w3-right">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/logo.png" alt="" />
				</span>
			</div>
			<com:Application.API.Portlets.APISideBar />
			<div class="w3-main page_main_el" id="page_main" style="margin-left: 250px; margin-top: 43px;">
				<com:TContentPlaceHolder ID="Main" />
				<footer class="w3-container w3-right-align w3-small"><%[ Version: ]%> <%=Params::BACULUM_VERSION%></footer>
			</div>
			<div id="small" class="w3-hide-large"></div>
		</com:TForm>
<script type="text/javascript">
var is_small = $('#small').is(':visible');
if (is_small) {
	W3SideBar.close();
}
</script>
	</body>
</html>
