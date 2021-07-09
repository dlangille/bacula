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
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/misc.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/fontawesome.min.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/datatables.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/dataTables.responsive.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/responsive.jqueryui.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/dataTables.buttons.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/buttons.html5.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/buttons.colVis.js %> />
			<com:BClientScript ScriptUrl=<%~ ../../Common/JavaScript/dataTables.select.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/flotr2.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/bacula-config.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/misc.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/graph.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/statistics.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/gauge.js %> />
			<com:Application.Common.Portlets.TableDefaults />
			<!-- Top container -->
			<div class="w3-bar w3-top w3-black w3-large" style="z-index:4">
				<button type="button" class="w3-bar-item w3-button w3-hover-none w3-hover-text-light-grey" onclick="W3SideBar.open();"><i class="fa fa-bars"></i>  Menu</button>
				<span class="w3-bar-item w3-right">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/logo.png" alt="" />
				</span>
			</div>
			<com:Application.Web.Portlets.MainSideBar />
			<!-- !PAGE CONTENT! -->
			<div class="w3-main page_main_el" id="page_main" style="margin-left: 250px; margin-top: 43px;">
				<span class="w3-tag w3-large w3-purple w3-right w3-padding-small w3-margin-top w3-margin-right">
					<i class="fa fa-cogs w3-large"></i> <%[ Running jobs: ]%> <span id="running_jobs"></span>
				</span>
				<span id="msg_envelope" class="w3-tag w3-large w3-green w3-text-white w3-right w3-padding-small w3-margin-top w3-margin-right" style="cursor: pointer;<%=$this->User->isInRole(WebUserRoles::ADMIN) === false ? 'display: none' : ''%>" title="<%[ Display messages log window ]%>">
					<i class="fas fa-envelope w3-large"></i>
				</span>
				<com:TLabel ID="UserAPIHostsContainter"CssClass="w3-right w3-margin-top w3-margin-right">
					<%[ API host: ]%>
					<com:TDropDownList
						ID="UserAPIHosts"
						CssClass="w3-select w3-border w3-small"
						OnTextChanged="setAPIHost"
						AutoPostBack="true"
						Width="200px"
						Height="34px"
					/>
				</com:TLabel>
				<script type="text/javascript">
					var SIZE_VALUES_UNIT = '<%=(count($this->web_config) > 0 && key_exists('size_values_unit', $this->web_config['baculum'])) ? $this->web_config['baculum']['size_values_unit'] : WebConfig::DEF_SIZE_VAL_UNIT%>';
					var DATE_TIME_FORMAT = '<%=(count($this->web_config) > 0 && key_exists('date_time_format', $this->web_config['baculum'])) ? $this->web_config['baculum']['date_time_format'] : WebConfig::DEF_DATE_TIME_FORMAT%>';
				</script>
				<com:TContentPlaceHolder ID="Main" />
				<!-- Footer -->
				<footer class="w3-container w3-right-align w3-small"><%[ Version: ]%> <%=Params::BACULUM_VERSION%></footer>
			</div>
		</com:TForm>
		<div id="small" class="w3-hide-large"></div>
<com:Application.Web.Portlets.ErrorMessageBox />
<com:Application.Web.Portlets.ResourceMonitor />
<com:Application.Web.Portlets.MsgEnvelope Visible="<%=$this->User->isInRole(WebUserRoles::ADMIN)%>" />
<script>
var is_small = $('#small').is(':visible');
$(function() {
	if (is_small) {
		W3SideBar.close();
	}
});
</script>
	</body>
</html>
