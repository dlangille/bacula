<div class="w3-quarter">
	<span class="w3-margin-left"><%[ Time range: ]%></span>
	<select id="time_range" name="time_range" class="w3-select w3-border" style="width: 90%; margin-left: 5%;">
		<option value="23400"><%[ Last 6 hours ]%></option>
		<option value="43200"><%[ Last 12 hours ]%></option>
		<option value="86400" selected="selected"><%[ Today ]%></option>
		<option value="172800"><%[ Two days ]%></option>
		<option value="604800"><%[ Last week ]%></option>
		<option value="1209600"><%[ Last two weeks ]%></option>
		<option value="2592000"><%[ Last month ]%></option>
		<option value="7776000"><%[ Last three months ]%></option>
		<option value="15768000"><%[ Last six months ]%></option>
		<option value="31536000"><%[ Last year ]%></option>
	</select>
</div>
<div class="w3-quarter">
	<div class="w3-half">
		<%[ Date From: ]%> <com:TDatePicker ID="DateFrom" DateFormat="yyyy-MM-dd" Style="width: 90%; height: 39px; padding: 6px;" />
	</div>
	<div class="w3-half">
		<%[ Date To: ]%> <com:TDatePicker ID="DateTo" DateFormat="yyyy-MM-dd" Style="width: 90%; height: 39px; padding: 6px;" />
	</div>
</div>
<div class="w3-quarter">
	<span><%[ Client: ]%></span>
	<com:TDropDownList
		ID="Clients"
		CssClass="w3-select w3-border"
		AutoPostBack="false"
		Style="display: inline; width: 90%;"
	/>
</div>
<div class="w3-quarter">
	<span><%[ Job name: ]%></span>
	<select id="graph_jobs" class="w3-select w3-border" style="display: inline; width: 90%;">
		<option value="@"><%[ select job ]%></option>
	</select>
</div>
<span class="w3-margin-left"><%[ Legend: ]%></span>
<div id="legend" class="w3-margin-left"></div>
<div id="graphs_content" style="height: 500px"></div>
<script type="text/javascript">
	MonitorParams = ['jobs'];
	var graph_lang = {
		"graph_title": "<%[ Graph: Jobs size / Time ]%>",
		"xaxis_title": "<%[ Time ]%>",
		"yaxis_title": "<%[ Jobs size (GiB) ]%>"
	};
	var graph;
	$(function() {
		MonitorCalls.push(function() {
			graph = new GraphClass(oData.jobs, graph_lang, 'graphs_content', 'legend', 'time_range', '<%=$this->DateFrom->ClientID%>', '<%=$this->DateTo->ClientID%>', '<%=$this->Clients->ClientID%>', 'graph_jobs');
		});
	});
</script>
<p class="bold"><%[ Tip: for getting zoom, please mark area on graph. ]%></p>
<p class="bold"><%[ Tip 2: for back from zoom, please click somewhere on graph. ]%></p>
