<div class="w3-row w3-margin-bottom<%=!$this->display_directive ? ' hide' : '';%>">
	<div class="w3-col w3-quarter"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" Visible="<%=$this->display_directive%>" />:</div>
	<div class="w3-col w3-threequarter directive_value">
		<com:TActiveTextBox ID="Directive"
			TextMode="Password"
			PersistPassword="true"
			OnTextChanged="saveValue"
			CssClass="w3-input w3-border w3-twothird"
			Visible="<%=$this->display_directive%>"
			ActiveControl.EnableUpdate="false"
			AutoTrim="true"
		/>
		 <%=$this->getRequired() ? '&nbsp;<i class="fa fa-asterisk w3-text-red" style="line-height: 40px"></i>' : ''%>
		<a href="javascript:void(0)" onclick="var el = document.getElementById('<%=$this->Directive->ClientID%>'); el.type = el.type == 'text' ? 'password' : 'text'"><i class="fa fa-eye"></i></a>
		<i class="fa fa-undo reset_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').value = '<%=$this->getDefaultValue() === 0 ? '' : $this->getDefaultValue()%>';" alt="<%[ Reset to default value ]%>" title="<%[ Reset to default value ]%>"></i>
		<i class="fa fa-trash-alt remove_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').value = '';" alt="<%[ Remove directive ]%>" title="<%[ Remove directive ]%>"></i>
		<com:TRequiredFieldValidator
			ID="DirectiveValidator"
			ValidationGroup="<%=$this->getValidationGroup()%>"
			Display="Dynamic"
			ControlToValidate="Directive"
			FocusOnError="true"
			Enabled="<%=$this->getRequired() && $this->getShow()%>"
		><%[ Field required. ]%></com:TRequiredFieldValidator>
	</div>
</div>
