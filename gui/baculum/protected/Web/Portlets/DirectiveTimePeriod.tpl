<div class="w3-row w3-margin-bottom<%=!$this->display_directive ? ' hide' : '';%>">
	<div class="w3-col w3-quarter"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" Visible="<%=$this->display_directive%>" />:</div>
	<div class="w3-col w3-threequarter directive_value">
		<com:TActiveTextBox ID="Directive"
			OnTextChanged="saveValue"
			CssClass="w3-input w3-border w3-third"
			Visible="<%=$this->display_directive%>"
			ActiveControl.EnableUpdate="false"
		/>
		<com:TActiveDropDownList
			ID="TimeFormat"
			CssClass="w3-select w3-border w3-quarter"
			DataTextField="label"
			DataValueField="format"
			Visible="<%=$this->display_directive%>"
			ActiveControl.EnableUpdate="false"
			AutoPostBack="false"
			OnSelectedIndexChanged="saveValue"
		/> <%=$this->getRequired() ? '&nbsp;<i class="fa fa-asterisk w3-text-red" style="line-height: 40px"></i>' : ''%>
		<i class="fa fa-undo reset_btn" onclick="var ftime = Units.format_time_period(parseInt('<%=$this->getDefaultValue()%>', 10), 'second'); document.getElementById('<%=$this->Directive->ClientID%>').value = ftime.value; document.getElementById('<%=$this->TimeFormat->ClientID%>').value = ftime.format;" alt="<%[ Reset to default value ]%>" title="<%[ Reset to default value ]%>"></i>
		<i class="fa fa-trash-alt remove_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').value = '';" alt="<%[ Remove directive ]%>" title="<%[ Remove directive ]%>"></i>
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
