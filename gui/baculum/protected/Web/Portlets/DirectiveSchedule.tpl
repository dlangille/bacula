<button type="button" class="w3-button w3-green w3-margin-bottom" onmousedown="openElementOnCursor(event, '<%=$this->RunscriptMenu->ClientID%>_new_schedule', 0, 20);"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
<com:Application.Web.Portlets.NewScheduleMenu ID="RunscriptMenu" />
<com:TActiveRepeater ID="RepeaterScheduleRuns" OnItemDataBound="createRunItem">
	<prop:ItemTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive runscript">
		<com:TActiveLinkButton
			CssClass="w3-button w3-green w3-right"
			OnCommand="SourceTemplateControl.removeSchedule"
			CommandName="<%=$this->ItemIndex%>"
			CommandParameter="save"
		>
			<i class="fa fa-trash-alt"></i> &nbsp;<%[ Remove ]%>
		</com:TActiveLinkButton>
			<h2 class="schedule_options"><%[ Run ]%> #<%=($this->ItemIndex+1)%></h2>
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
			<com:Application.Web.Portlets.DirectiveCheckBox
				ID="Accurate"
			/>
			<com:Application.Web.Portlets.DirectiveTextBox
				ID="Priority"
				CssClass="smallbox"
			/>
			<com:Application.Web.Portlets.DirectiveCheckBox
				ID="SpoolData"
			/>
			<com:Application.Web.Portlets.DirectiveTimePeriod
				ID="MaxRunSchedTime"
			/>
		<div class="w3-border w3-padding w3-margin-top w3-margin-bottom">
			<h3><%[ Month ]%></h3>
			<div style="display: flex; flex-wrap: wrap;">
				<div class="option_cell">
					<com:TRadioButton
						ID="MonthDisable"
						CssClass="w3-radio"
						GroupName="Month"
						Attributes.onchange="$('.month<%=$this->MonthSingle->ClientID%>').hide();"
						Checked="true" />
					<com:TLabel ForControl="MonthDisable" Text="<%[ Run every month ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="MonthSingle"
						CssClass="w3-radio"
						GroupName="Month"
						Attributes.onchange="$('.month<%=$this->MonthSingle->ClientID%>').hide();$('#month<%=$this->MonthSingle->ClientID%>').show()" />
					<com:TLabel ForControl="MonthSingle" Text="<%[ Run one month a year ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="MonthRange"
						CssClass="w3-radio"
						GroupName="Month"
						Attributes.onchange="$('.month<%=$this->MonthSingle->ClientID%>').hide();$('#month_range<%=$this->MonthSingle->ClientID%>').show()" />
					<com:TLabel ForControl="MonthRange" Text="<%[ Run from month to month a year (range) ]%>" />
				</div>
			</div>
			<div id="month<%=$this->MonthSingle->ClientID%>" class="w3-margin month<%=$this->MonthSingle->ClientID%>" style="display: <%=$this->MonthSingle->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="Month"
					Label="<%[ Month ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="MonthSingle"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select month in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->MonthSingle->ClientID%>').prop('checked') && ($('#<%=$this->Month->ClientID%>_Directive').val()).trim() === '');
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
			<div id="month_range<%=$this->MonthSingle->ClientID%>" class="w3-margin month<%=$this->MonthSingle->ClientID%>" style="display: <%=$this->MonthRange->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="MonthRangeFrom"
					Label="<%[ From month ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="MonthRangeTo"
					Label="<%[ To month ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="MonthRange"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select month range in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->MonthRange->ClientID%>').prop('checked') && (($('#<%=$this->MonthRangeFrom->ClientID%>_Directive').val()).trim() === '' || ($('#<%=$this->MonthRangeTo->ClientID%>_Directive').val()).trim() === ''));
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
		</div>
		<div class="w3-border w3-padding w3-margin-top w3-margin-bottom">
			<h3><%[ Week ]%></h3>
			<div style="display: flex; flex-wrap: wrap;">
				<div class="option_cell">
					<com:TRadioButton
						ID="WeekDisable"
						CssClass="w3-radio"
						GroupName="Week"
						Attributes.onchange="$('.week<%=$this->WeekSingle->ClientID%>').hide();"
						Checked="true" />
					<com:TLabel ForControl="WeekDisable" Text="<%[ Run every week ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="WeekSingle"
						CssClass="w3-radio"
						GroupName="Week"
						Attributes.onchange="$('.week<%=$this->WeekSingle->ClientID%>').hide();$('#week<%=$this->WeekSingle->ClientID%>').show()" />
					<com:TLabel ForControl="WeekSingle" Text="<%[ Run one week a month ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="WeekRange"
						CssClass="w3-radio"
						GroupName="Week"
						Attributes.onchange="$('.week<%=$this->WeekSingle->ClientID%>').hide();$('#week_range<%=$this->WeekSingle->ClientID%>').show()" />
					<com:TLabel ForControl="WeekRange" Text="<%[ Run from week to week a month (range) ]%>" />
				</div>
			</div>
			<div id="week<%=$this->WeekSingle->ClientID%>" class="w3-margin week<%=$this->WeekSingle->ClientID%>" style="display: <%=$this->WeekSingle->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="Week"
					Label="<%[ Week ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="WeekSingle"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select week in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->WeekSingle->ClientID%>').prop('checked') && ($('#<%=$this->Week->ClientID%>_Directive').val()).trim() === '');
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
			<div id="week_range<%=$this->WeekSingle->ClientID%>" class="w3-margin week<%=$this->WeekSingle->ClientID%>" style="display: <%=$this->WeekRange->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="WeekRangeFrom"
					Label="<%[ From week ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="WeekRangeTo"
					Label="<%[ To week ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="MonthRange"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select week range in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->WeekRange->ClientID%>').prop('checked') && (($('#<%=$this->WeekRangeFrom->ClientID%>_Directive').val()).trim() === '' || ($('#<%=$this->WeekRangeTo->ClientID%>_Directive').val()).trim() === ''));
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
		</div>
		<div class="w3-border w3-padding w3-margin-top w3-margin-bottom">
			<h3><%[ Day ]%></h3>
			<div style="display: flex; flex-wrap: wrap;">
				<div class="option_cell">
					<com:TRadioButton
						ID="DayDisable"
						CssClass="w3-radio"
						GroupName="Day"
						Attributes.onchange="$('.day<%=$this->DaySingle->ClientID%>').hide();"
						Checked="true" />
					<com:TLabel ForControl="DayDisable" Text="<%[ Run every day ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="DaySingle"
						CssClass="w3-radio"
						GroupName="Day"
						Attributes.onchange="$('.day<%=$this->DaySingle->ClientID%>').hide();$('#day<%=$this->DaySingle->ClientID%>').show()" />
					<com:TLabel ForControl="DaySingle" Text="<%[ Run one day a month ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="DayRange"
						CssClass="w3-radio"
						GroupName="Day"
						Attributes.onchange="$('.day<%=$this->DaySingle->ClientID%>').hide();$('#day_range<%=$this->DaySingle->ClientID%>').show()" />
					<com:TLabel ForControl="DayRange" Text="<%[ Run from day to day a month (range) ]%>" />
				</div>
			</div>
			<div id="day<%=$this->DaySingle->ClientID%>" class="w3-margin day<%=$this->DaySingle->ClientID%>" style="display: <%=$this->DaySingle->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="Day"
					Label="<%[ Day ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="DaySingle"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select day in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->DaySingle->ClientID%>').prop('checked') && ($('#<%=$this->Day->ClientID%>_Directive').val()).trim() === '');
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
			<div id="day_range<%=$this->DaySingle->ClientID%>" class="w3-margin day<%=$this->DaySingle->ClientID%>" style="display: <%=$this->DayRange->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="DayRangeFrom"
					Label="<%[ From day ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="DayRangeTo"
					Label="<%[ To day ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="DayRange"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select day range in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->DayRange->ClientID%>').prop('checked') && (($('#<%=$this->DayRangeFrom->ClientID%>_Directive').val()).trim() === '' || ($('#<%=$this->DayRangeTo->ClientID%>_Directive').val()).trim() === ''));
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
		</div>
		<div class="w3-border w3-padding w3-margin-top w3-margin-bottom">
			<h3><%[ Day of week ]%></h3>
			<div style="display: flex; flex-wrap: wrap;">
				<div class="option_cell">
					<com:TRadioButton
						ID="WdayDisable"
						CssClass="w3-radio"
						GroupName="Wday"
						Attributes.onchange="$('.day<%=$this->WdaySingle->ClientID%>').hide();"
						Checked="true" />
					<com:TLabel ForControl="WdayDisable" Text="<%[ Run every day of week ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="WdaySingle"
						CssClass="w3-radio"
						GroupName="Wday"
						Attributes.onchange="$('.day<%=$this->WdaySingle->ClientID%>').hide();$('#day<%=$this->WdaySingle->ClientID%>').show()" />
					<com:TLabel ForControl="WdaySingle" Text="<%[ Run one day of week ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="WdayRange"
						CssClass="w3-radio"
						GroupName="Wday"
						Attributes.onchange="$('.day<%=$this->WdaySingle->ClientID%>').hide();$('#day_range<%=$this->WdaySingle->ClientID%>').show()" />
					<com:TLabel ForControl="WdayRange" Text="<%[ Run from day of week to day of week (range) ]%>" />
				</div>
			</div>
			<div id="day<%=$this->WdaySingle->ClientID%>" class="w3-margin day<%=$this->WdaySingle->ClientID%>" style="display: <%=$this->WdaySingle->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="Wday"
					Label="<%[ Day of week ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="WdaySingle"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select day of week in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->WdaySingle->ClientID%>').prop('checked') && ($('#<%=$this->Wday->ClientID%>_Directive').val()).trim() === '');
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
			<div id="day_range<%=$this->WdaySingle->ClientID%>" class="w3-margin day<%=$this->WdaySingle->ClientID%>" style="display: <%=$this->WdayRange->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="WdayRangeFrom"
					Label="<%[ From day of week ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:Application.Web.Portlets.DirectiveComboBox
					ID="WdayRangeTo"
					Label="<%[ To day of week ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="WdayRange"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select day of week range in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->WdayRange->ClientID%>').prop('checked') && (($('#<%=$this->WdayRangeFrom->ClientID%>_Directive').val()).trim() === '' || ($('#<%=$this->WdayRangeTo->ClientID%>_Directive').val()).trim() === ''));
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
		</div>
		<div class="w3-border w3-padding w3-margin-top w3-margin-bottom">
			<h3><%[ Hour and minute ]%></h3>
			<div style="display: flex; flex-wrap: wrap;">
				<div class="option_cell">
					<com:TRadioButton
						ID="TimeDisable"
						CssClass="w3-radio"
						GroupName="Time"
						Attributes.onchange="$('.day<%=$this->TimeAt->ClientID%>').hide();" />
					<com:TLabel ForControl="TimeDisable" Text="<%[ Run every full hour ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="TimeAt"
						CssClass="w3-radio"
						GroupName="Time"
						Attributes.onchange="$('.day<%=$this->TimeAt->ClientID%>').hide();$('#day<%=$this->TimeAt->ClientID%>').show()" />
					<com:TLabel ForControl="TimeAt" Text="<%[ Run at hour and minute ]%>" />
				</div>
				<div class="option_cell">
					<com:TRadioButton
						ID="TimeHourly"
						CssClass="w3-radio"
						GroupName="Time"
						Attributes.onchange="$('.day<%=$this->TimeAt->ClientID%>').hide();$('#day_range<%=$this->TimeAt->ClientID%>').show()" />
					<com:TLabel ForControl="TimeHourly" Text="<%[ Run hourly at minute ]%>" />
				</div>
			</div>
			<div id="day<%=$this->TimeAt->ClientID%>" class="w3-margin day<%=$this->TimeAt->ClientID%>" style="display: <%=$this->TimeAt->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveTextBox
					ID="TimeHourAt"
					Label="<%[ Hour ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:Application.Web.Portlets.DirectiveTextBox
					ID="TimeMinAt"
					Label="<%[ Minute ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="TimeAt"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select hour and minute in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->TimeAt->ClientID%>').prop('checked') && (($('#<%=$this->TimeHourAt->ClientID%>_Directive').val()).trim() === '' || ($('#<%=$this->TimeMinAt->ClientID%>_Directive').val()).trim() === ''));
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
			<div id="day_range<%=$this->TimeAt->ClientID%>" class="w3-margin day<%=$this->TimeAt->ClientID%>" style="display: <%=$this->TimeHourly->Checked ? 'block' : 'none'%>">
				<com:Application.Web.Portlets.DirectiveTextBox
					ID="TimeMinHourly"
					Label="<%[ Minute ]%>"
					InConfig="true"
					Show="true"
					CssClass="smallbox"
				/>
				<com:TCustomValidator
					ValidationGroup="Directive"
					ControlToValidate="TimeHourly"
					Display="None"
					ClientValidationFunction="schedule_required_fields_validator"
					ErrorMessage="<%=Prado::localize('Please select hour in Run block')%> #<%=($this->ItemIndex+1)%>.">
					<prop:ClientSide.OnValidate>
						sender.enabled = ($('#<%=$this->TimeHourly->ClientID%>').prop('checked') && ($('#<%=$this->TimeMinHourly->ClientID%>_Directive').val()).trim() === '');
					</prop:ClientSide.OnValidate>
				</com:TCustomValidator>
			</div>
		</div>
	</div>
	<com:TCustomValidator
		ValidationGroup="Directive"
		ControlToValidate="TimeDisable"
		Display="None"
		ClientValidationFunction="schedule_required_fields_validator"
		ErrorMessage="<%=Prado::localize('Please choose at least month or week or day of week or hour in Run block')%> #<%=($this->ItemIndex+1)%>.">
		<prop:ClientSide.OnValidate>
			var m = ($('#<%=$this->MonthSingle->ClientID%>').prop('checked') && $('#<%=$this->Month->ClientID%>_Directive').val());
			var m_r = ($('#<%=$this->MonthRange->ClientID%>').prop('checked') && $('#<%=$this->MonthRangeFrom->ClientID%>_Directive').val() && $('#<%=$this->MonthRangeTo->ClientID%>_Directive').val());
			var w = ($('#<%=$this->WeekSingle->ClientID%>').prop('checked') && $('#<%=$this->Week->ClientID%>_Directive').val());
			var w_r = ($('#<%=$this->WeekRange->ClientID%>').prop('checked') && $('#<%=$this->WeekRangeFrom->ClientID%>_Directive').val() && $('#<%=$this->WeekRangeTo->ClientID%>_Directive').val());
			var wd = ($('#<%=$this->WdaySingle->ClientID%>').prop('checked') && $('#<%=$this->Wday->ClientID%>_Directive').val());
			var wd_r = ($('#<%=$this->WdayRange->ClientID%>').prop('checked') && $('#<%=$this->WdayRangeFrom->ClientID%>_Directive').val() && $('#<%=$this->WdayRangeTo->ClientID%>_Directive').val());
			var t = ($('#<%=$this->TimeAt->ClientID%>').prop('checked') && $('#<%=$this->TimeHourAt->ClientID%>_Directive').val() && $('#<%=$this->TimeMinAt->ClientID%>_Directive').val());
			var t_h = ($('#<%=$this->TimeHourly->ClientID%>').prop('checked') && $('#<%=$this->TimeMinHourly->ClientID%>_Directive').val());
			sender.enabled = (!t && !t_h && !m && !m_r && !w && !w_r && !wd && !wd_r);
		</prop:ClientSide.OnValidate>
	</com:TCustomValidator>
	</prop:ItemTemplate>
</com:TActiveRepeater>
<com:TValidationSummary
        ValidationGroup="Directive"
        Display="None"
	HeaderText="<%[ Validation error ]%>"
 />
<script>
function schedule_required_fields_validator(sender, param) {
	return false;
}
</script>
