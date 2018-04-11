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
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/flotr2.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/fontawesome-all.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/datatables.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/dataTables.responsive.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/responsive.jqueryui.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/excanvas.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/bacula-config.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/misc.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/graph.js %> />
			<com:BClientScript ScriptUrl=<%~ ../JavaScript/statistics.js %> />
			<!-- Top container -->
			<div class="w3-bar w3-top w3-black w3-large" style="z-index:4">
				<button type="button" class="w3-bar-item w3-button w3-hover-none w3-hover-text-light-grey" onclick="W3SideBar.open();"><i class="fa fa-bars"></i>  Menu</button>
				<span class="w3-bar-item w3-right">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/logo.png" alt="" />
				</span>
			</div>
			<com:Application.Web.Portlets.MainSideBar />
			<!-- !PAGE CONTENT! -->
			<div class="w3-main" id="page_main" style="margin-left:300px; margin-top:43px;">
				<span class="w3-tag w3-large w3-purple w3-right w3-padding-small w3-margin-top w3-margin-right">
					<i class="fa fa-cogs w3-large"></i> <%[ Running jobs: ]%> <span id="running_jobs"></span>
				</span>
				<com:TContentPlaceHolder ID="Main" />
				<!-- Footer -->
				<footer class="w3-container w3-padding-16 w3-light-grey">
				</footer>
			</div>
		</com:TForm>
		<div id="small" class="w3-hide-large"></div>
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
$(function() {
	if (is_small) {
		W3SideBar.close();
	}
	oMonitor = function() {
		return $.ajax('<%=$this->Service->constructUrl("Monitor")%>', {
			dataType: 'json',
			type: 'post',
			data: {'params': (typeof(MonitorParams) == 'object' ? MonitorParams : [])},
			beforeSend: function() {
				last_callback_time = new Date().getTime();
			},
			success: function(response) {
				if (timeout_handler) {
					clearTimeout(timeout_handler);
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
				if (calls_len > 0) {
					Formatters.set_formatters();
				}
				MonitorCalls = [];
			}
		});
	};
	oMonitor();
});
	</script>
	</body>
</html>
