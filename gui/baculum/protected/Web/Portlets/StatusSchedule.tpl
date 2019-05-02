<!-- Header -->
<header class="w3-container">
	<h5>
		<b><i class="fa fa-clock"></i> <%[ Schedule status ]%></b>
	</h5>
</header>
<div class="w3-container">
	<div class="w3-card-4 w3-padding w3-margin-bottom">
		<strong><%[ Filters: ]%></strong>
		 <%[ Date from: ]%> <com:TJuiDatePicker
			ID="DatePicker"
			Options.dateFormat="yy-mm-dd"
			Options.changeYear="true",
			Options.changeMonth="true"
			Options.showAnim="fold"
			Style.Width="100px"
			/>
		<com:TRequiredFieldValidator
			ValidationGroup="ScheduleFilters"
			ControlToValidate="DatePicker"
			Text="<%[ Field required. ]%>"
			Display="Dynamic"
		/>
		<com:TRegularExpressionValidator
			ValidationGroup="ScheduleFilters"
			ControlToValidate="DatePicker"
			RegularExpression="\d{4}-\d{2}-\d{2}"
			Text="<%[ Invalid date format. ]%>"
			Display="Dynamic"
		/>
		<%[ Days: ]%> <com:TTextBox
			ID="Days"
			Style.Width="50px"
			Text="90"
		/>
		<com:TRequiredFieldValidator
			ValidationGroup="ScheduleFilters"
			ControlToValidate="Days"
			Text="<%[ Field required. ]%>"
			Display="Dynamic"
		/>
		<com:TDataTypeValidator
			ValidationGroup="ScheduleFilters"
			ControlToValidate="Days"
			DataType="Integer"
			Text="<%[ You must enter an integer. ]%>"
			Display="Dynamic"
		/>
		<com:TRangeValidator
			ValidationGroup="ScheduleFilters"
			ControlToValidate="Days"
			DataType="Integer"
			MinValue="1"
			MaxValue="1000"
			Text="<%[ Input must be between 1 and 1000. ]%>"
			Display="Dynamic"
		/>
		<com:TLabel
			ID="ClientLabel"
			ForControl="Client"
			Text="<%[ Client: ]%>"
		/> <com:TDropDownList
			ID="Client"
			Style.Width="150px"
		/>
		<com:TLabel
			ID="ScheduleLabel"
			ForControl="Schedule"
			Text="<%[ Schedule: ]%>"
		/>
		 <com:TDropDownList
			ID="Schedule"
			Style.Width="150px"
		/>
		<com:TActiveLinkButton
			ID="ApplyFilter"
			CausesValidation="true"
			ValidationGroup="ScheduleFilters"
			CssClass="w3-green w3-button w3-margin-left"
			OnClick="applyFilters"
		>
			<i class="fa fa-check"></i> &nbsp;<%[ Apply filters ]%>
		</com:TActiveLinkButton>
	</div>
	<table id="schedule_list" class="w3-table w3-striped w3-hoverable w3-white w3-margin-bottom" style="width: 100%">
		<thead>
			<tr>
				<th></th>
				<th><%[ Level ]%></th>
				<th><%[ Type ]%></th>
				<th><%[ Priority ]%></th>
				<th><%[ Scheduled ]%></th>
				<%=empty($this->getJob()) ? '<th>' . Prado::localize('Job name') . '</th>': ''%>
				<th><%[ Client ]%></th>
				<th><%[ FileSet ]%></th>
				<th><%[ Schedule ]%></th>
			</tr>
		</thead>
		<tbody id="schedule_list_body">
		</tbody>
	</table>
</div>
<script type="text/javascript">
var oJobScheduleList = {
	table: null,
	data: <%=json_encode($this->schedules)%>,
	ids: {
		schedule_list: 'schedule_list',
		schedule_list_body: 'schedule_list_body'
	},
	init: function() {
		this.set_table();
	},
	set_data(data) {
		data = JSON.parse(data);
		this.data = data;
	},
	get_data(data) {
		return this.data;
	},
	set_table: function() {
		this.table = $('#' + this.ids.schedule_list).DataTable({
			data: this.get_data(),
			deferRender: true,
			columns: [
				{
					className: 'details-control',
					orderable: false,
					data: null,
					defaultContent: '<button type="button" class="w3-button w3-blue"><i class="fa fa-angle-down"></i></button>'
				},
				{
					data: 'level',
					render: function(data, type, row) {
						var ret;
						if (!data) {
							ret = '-';
						} else {
							ret = JobLevel.get_level(data);
						}
						return ret;
					}
				},
				{
					data: 'type',
					render: function(data, type, row) {
						return JobType.get_type(data);
					}
				},
				{data: 'priority'},
				{
					data: 'schedtime_epoch',
					render: function(data, type, row) {
						return Units.format_date(data);
					}
				},
				<%=empty($this->getJob()) ? '{data: "name"},' : ''%>
				{data: 'client'},
				{data: 'fileset'},
				{data: 'schedule'}
			],
			responsive: {
				details: {
					type: 'column'
				}
			},
			columnDefs: [{
				className: 'control',
				orderable: false,
				targets: 0
			},
			{
				className: "dt-center",
				targets: [ 1, 3 ]
			}],
			order: [4, 'asc']
		});
	}
};

function set_job_schedule_data(data) {
	oJobScheduleList.set_data(data);
}
function init_job_schedule() {
	if (oJobScheduleList.table) {
		oJobScheduleList.table.destroy();
	}
	oJobScheduleList.init();
}
<%=count($this->schedules) > 0 ? 'init_job_schedule();' : ''%>
</script>
