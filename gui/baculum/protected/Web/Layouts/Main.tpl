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
		<div id="error_message_box" class="w3-modal" style="display: none">
			<div class="w3-modal-content w3-card-4 w3-animate-zoom" style="width:600px">
				<header class="w3-container w3-red">
					<span onclick="document.getElementById('error_message_box').style.display='none'" class="w3-button w3-display-topright">×</span>
					<h2><%[ Error ]%></h2>
				</header>
				<div class="w3-panel w3-padding">
					<p><strong><%[ Error code: ]%></strong> <span id="error_message_error_code"></span></p>
					<p><strong><%[ Message: ]%></strong> <span id="error_message_error_msg"></span></p>
				</div>
				<footer class="w3-container w3-center">
					<button type="button" class="w3-button w3-section w3-red" onclick="document.getElementById('error_message_box').style.display='none'"><i class="fa fa-check"></i> &nbsp;<%[ OK ]%></button>
				</footer>
			</div>
		</div>
<script type="text/javascript">
var is_small = $('#small').is(':visible');

var oMonitor;
var default_refresh_interval = 60000;
var default_fast_refresh_interval = 10000;
var timeout_handler;
var last_callback_time = 0;
var callback_time_offset = 0;
var oData;
var MonitorCalls = [];
var MonitorCallsInterval = [];
$(function() {
	if (is_small) {
		W3SideBar.close();
	}
	oMonitor = function() {
		return $.ajax('<%=$this->Service->constructUrl("Monitor")%>', {
			dataType: 'json',
			type: 'post',
			data: {
				'params': (typeof(MonitorParams) == 'object' ? MonitorParams : []),
				'use_limit' : <%=$this->Service->getRequestedPagePath() == "Dashboard" ? '0' : '1'%>,
			},
			beforeSend: function() {
				last_callback_time = new Date().getTime();
			},
			success: function(response) {
				if (timeout_handler) {
					clearTimeout(timeout_handler);
				}
				if (response && response.hasOwnProperty('error') && response.error.error !== 0) {
					show_error(response.error.output, response.error.error);
				}

				oData = response;
				if ('<%=get_class($this->Service->getRequestedPage())%>' == 'Dashboard') {
					Statistics.grab_statistics(oData, JobStatus.get_states());
					Dashboard.update_all(Statistics);
				}

				if (oData.running_jobs.length > 0) {
					refreshInterval =  callback_time_offset + default_fast_refresh_interval;
				} else {
					refreshInterval = default_refresh_interval;
				}
				if (typeof(job_callback_func) == 'function') {
					job_callback_func();
				}
				document.getElementById('running_jobs').textContent = oData.running_jobs.length;
				timeout_handler = setTimeout("oMonitor()", refreshInterval);

				var calls_len = MonitorCalls.length;
				for (var i = 0; i < calls_len; i++) {
					MonitorCalls[i]();
				}
				var calls_interval_len = MonitorCallsInterval.length;
				for (var i = 0; i < calls_interval_len; i++) {
					MonitorCallsInterval[i]();
				}
				if (calls_len > 0) {
					Formatters.set_formatters();
				}
				MonitorCalls = [];
			}
		});
	};
	oMonitor();
});
function show_error(output, error) {
	var err_box = document.getElementById('error_message_box');
	var err_code = document.getElementById('error_message_error_code');
	var err_msg = document.getElementById('error_message_error_msg');
	err_code.textContent = error;
	err_msg.innerHTML = output;
	err_box.style.display = 'block';
}
	</script>
	<com:Application.Web.Portlets.MsgEnvelope Visible="<%=$this->User->isInRole(WebUserRoles::ADMIN)%>" />
	</body>
</html>
