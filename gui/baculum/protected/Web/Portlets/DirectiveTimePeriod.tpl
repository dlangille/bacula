<div class="line<%=!$this->display_directive ? ' hide' : '';%>">
	<div class="text"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" Visible="<%=$this->display_directive%>" />:</div>
	<div class="field directive_value">
		<com:TActiveTextBox ID="Directive"
			OnTextChanged="saveValue"
			CssClass="ftype_timeperiod"
			Visible="<%=$this->display_directive%>"
			ActiveControl.EnableUpdate="false"
		/>
		<com:TActiveDropDownList
			ID="TimeFormat"
			DataTextField="label"
			DataValueField="format"
			Visible="<%=$this->display_directive%>"
			ActiveControl.EnableUpdate="false"
		/> <%=$this->getRequired() ? '*' : ''%>
		<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/switch.png" class="reset_btn" onclick="var ftime = Units.format_time_period(parseInt('<%=$this->getDefaultValue()%>', 10), 'second'); document.getElementById('<%=$this->Directive->ClientID%>').value = ftime.value; document.getElementById('<%=$this->TimeFormat->ClientID%>').value = ftime.format;" alt="<%[ Reset to default value ]%>" title="<%[ Reset to default value ]%>" />
		<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" class="remove_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').value = '';" alt="<%[ Remove directive ]%>" title="<%[ Remove directive ]%>" />
		<com:TRequiredFieldValidator
			ID="DirectiveValidator"
			ValidationGroup="Directive"
			Display="Dynamic"
			ControlToValidate="Directive"
			FocusOnError="true"
			Text="Field required."
			Enabled="<%=$this->getRequired() && $this->getShow()%>"
		/>
	</div>
</div>
