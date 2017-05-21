<fieldset class="directive">
	<legend><%[ Run ]%></legend>
	<com:TPanel ID="DirectiveContainer">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Level"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Pool"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Storage"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Messages"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="NextPool"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="FullPool"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="DifferentialPool"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="IncrementalPool"
		/>
		<com:Application.Web.Portlets.DirectiveBoolean
			ID="Accurate"
		/>
		<com:Application.Web.Portlets.DirectiveText
			ID="Priority"
		/>
		<com:Application.Web.Portlets.DirectiveBoolean
			ID="SpoolData"
		/>
		<com:Application.Web.Portlets.DirectiveBoolean
			ID="writepartafterjob"
		/>
		<com:Application.Web.Portlets.DirectiveTimePeriod
			ID="MaxRunSchedTime"
		/>
	</com:TPanel>
	<com:TRadioButton
		ID="MonthDisable"
		GroupName="Month"
		Attributes.onclick="$('.month<%=$this->MonthSingle->ClientID%>').hide();"
		Checked="true" />
	<com:TLabel ForControl="MonthDisable" Text="<%[ Disabled ]%>" />
	<com:TRadioButton
		ID="MonthSingle"
		GroupName="Month"
		Attributes.onclick="$('.month<%=$this->MonthSingle->ClientID%>').hide();$('#month<%=$this->MonthSingle->ClientID%>').show()" />
	<com:TLabel ForControl="MonthSingle" Text="<%[ Single month ]%>" />
	<com:TRadioButton
		ID="MonthRange"
		GroupName="Month"
		Attributes.onclick="$('.month<%=$this->MonthSingle->ClientID%>').hide();$('#month_range<%=$this->MonthSingle->ClientID%>').show()" />
	<com:TLabel ForControl="MonthRange" Text="<%[ Month range ]%>" />
	<div id="month<%=$this->MonthSingle->ClientID%>" class="month<%=$this->MonthSingle->ClientID%>" style="display: <%=$this->MonthSingle->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Month"
			Label="<%[ Month ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<div id="month_range<%=$this->MonthSingle->ClientID%>" class="month<%=$this->MonthSingle->ClientID%>" style="display: <%=$this->MonthRange->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="MonthRangeFrom"
			Label="<%[ From month ]%>"
			InConfig="true"
			Show="true"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="MonthRangeTo"
			Label="<%[ To month ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<hr />
	<com:TRadioButton
		ID="WeekDisable"
		GroupName="Week"
		Attributes.onclick="$('.week<%=$this->WeekSingle->ClientID%>').hide();"
		Checked="true" />
	<com:TLabel ForControl="WeekDisable" Text="<%[ Disabled ]%>" />
	<com:TRadioButton
		ID="WeekSingle"
		GroupName="Week"
		Attributes.onclick="$('.week<%=$this->WeekSingle->ClientID%>').hide();$('#week<%=$this->WeekSingle->ClientID%>').show()" />
	<com:TLabel ForControl="WeekSingle" Text="<%[ Single week ]%>" />
	<com:TRadioButton
		ID="WeekRange"
		GroupName="Week"
		Attributes.onclick="$('.week<%=$this->WeekSingle->ClientID%>').hide();$('#week_range<%=$this->WeekSingle->ClientID%>').show()" />
	<com:TLabel ForControl="WeekRange" Text="<%[ Week range ]%>" />
	<div id="week<%=$this->WeekSingle->ClientID%>" class="week<%=$this->WeekSingle->ClientID%>" style="display: <%=$this->WeekSingle->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Week"
			Label="<%[ Week ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<div id="week_range<%=$this->WeekSingle->ClientID%>" class="week<%=$this->WeekSingle->ClientID%>" style="display: <%=$this->WeekRange->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="WeekRangeFrom"
			Label="<%[ From week ]%>"
			InConfig="true"
			Show="true"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="WeekRangeTo"
			Label="<%[ To week ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>

	<hr />
	<com:TRadioButton
		ID="DayDisable"
		GroupName="Day"
		Attributes.onclick="$('.day<%=$this->DaySingle->ClientID%>').hide();"
		Checked="true" />
	<com:TLabel ForControl="DayDisable" Text="<%[ Disabled ]%>" />
	<com:TRadioButton
		ID="DaySingle"
		GroupName="Day"
		Attributes.onclick="$('.day<%=$this->DaySingle->ClientID%>').hide();$('#day<%=$this->DaySingle->ClientID%>').show()" />
	<com:TLabel ForControl="DaySingle" Text="<%[ Single day ]%>" />
	<com:TRadioButton
		ID="DayRange"
		GroupName="Day"
		Attributes.onclick="$('.day<%=$this->DaySingle->ClientID%>').hide();$('#day_range<%=$this->DaySingle->ClientID%>').show()" />
	<com:TLabel ForControl="DayRange" Text="<%[ Day range ]%>" />
	<div id="day<%=$this->DaySingle->ClientID%>" class="day<%=$this->DaySingle->ClientID%>" style="display: <%=$this->DaySingle->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Day"
			Label="<%[ Day ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<div id="day_range<%=$this->DaySingle->ClientID%>" class="day<%=$this->DaySingle->ClientID%>" style="display: <%=$this->DayRange->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="DayRangeFrom"
			Label="<%[ From day ]%>"
			InConfig="true"
			Show="true"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="DayRangeTo"
			Label="<%[ To day ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<hr />
	<com:TRadioButton
		ID="WdayDisable"
		GroupName="Wday"
		Attributes.onclick="$('.day<%=$this->WdaySingle->ClientID%>').hide();"
		Checked="true" />
	<com:TLabel ForControl="WdayDisable" Text="<%[ Disabled ]%>" />
	<com:TRadioButton
		ID="WdaySingle"
		GroupName="Wday"
		Attributes.onclick="$('.day<%=$this->WdaySingle->ClientID%>').hide();$('#day<%=$this->WdaySingle->ClientID%>').show()" />
	<com:TLabel ForControl="WdaySingle" Text="<%[ Single day of week ]%>" />
	<com:TRadioButton
		ID="WdayRange"
		GroupName="Wday"
		Attributes.onclick="$('.day<%=$this->WdaySingle->ClientID%>').hide();$('#day_range<%=$this->WdaySingle->ClientID%>').show()" />
	<com:TLabel ForControl="WdayRange" Text="<%[ Day of week range ]%>" />
	<div id="day<%=$this->WdaySingle->ClientID%>" class="day<%=$this->WdaySingle->ClientID%>" style="display: <%=$this->WdaySingle->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="Wday"
			Label="<%[ Day of week ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<div id="day_range<%=$this->WdaySingle->ClientID%>" class="day<%=$this->WdaySingle->ClientID%>" style="display: <%=$this->WdayRange->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="WdayRangeFrom"
			Label="<%[ From day of week ]%>"
			InConfig="true"
			Show="true"
		/>
		<com:Application.Web.Portlets.DirectiveComboBox
			ID="WdayRangeTo"
			Label="<%[ To day of week ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<hr />
	<com:TRadioButton
		ID="TimeDisable"
		GroupName="Time"
		Attributes.onclick="$('.day<%=$this->TimeAt->ClientID%>').hide();"
		Checked="true" />
	<com:TLabel ForControl="TimeDisable" Text="<%[ Disabled ]%>" />
	<com:TRadioButton
		ID="TimeAt"
		GroupName="Time"
		Attributes.onclick="$('.day<%=$this->TimeAt->ClientID%>').hide();$('#day<%=$this->TimeAt->ClientID%>').show()" />
	<com:TLabel ForControl="TimeAt" Text="<%[ At HH:MM ]%>" />
	<com:TRadioButton
		ID="TimeHourly"
		GroupName="Time"
		Attributes.onclick="$('.day<%=$this->TimeAt->ClientID%>').hide();$('#day_range<%=$this->TimeAt->ClientID%>').show()" />
	<com:TLabel ForControl="TimeHourly" Text="<%[ Hourly at ]%>" />
	<div id="day<%=$this->TimeAt->ClientID%>" class="day<%=$this->TimeAt->ClientID%>" style="display: <%=$this->TimeAt->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveText
			ID="TimeHourAt"
			DirectiveValue="<%=$this->directives['time']['hour']%>"
			Label="<%[ Hour ]%>"
			InConfig="true"
			Show="true"
		/>
		<com:Application.Web.Portlets.DirectiveText
			ID="TimeMinAt"
			DirectiveValue="<%=$this->directives['time']['minute']%>"
			Label="<%[ Minute ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
	<div id="day_range<%=$this->TimeAt->ClientID%>" class="day<%=$this->TimeAt->ClientID%>" style="display: <%=$this->TimeHourly->Checked ? 'block' : 'none'%>">
		<com:Application.Web.Portlets.DirectiveText
			ID="TimeHourHourly"
			DirectiveValue="0"
			Label="<%[ Hour ]%>"
			InConfig="true"
			Show="true"
		/>
		<com:Application.Web.Portlets.DirectiveText
			ID="TimeMinHourly"
			DirectiveValue="<%=$this->directives['time']['minute']%>"
			Label="<%[ Minute ]%>"
			InConfig="true"
			Show="true"
		/>
	</div>
</fieldset>
