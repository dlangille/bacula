<div class="directive_field w3-row w3-margin-bottom<%=!$this->display_directive ? ' hide' : '';%>">
	<div class="w3-col w3-quarter"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" Visible="<%=$this->display_directive%>" />:</div>
	<div class="w3-col w3-threequarter directive_value">
		<com:TActiveCheckBox
			ID="Directive"
			CssClass="w3-check"
			AutoPostBack="false"
			Visible="<%=$this->display_directive%>"
			OnCheckedChanged="saveValue"
			ActiveControl.EnableUpdate="false"
		/>
		<i class="fa fa-undo reset_btn<%=!$this->ShowResetButton ? ' hide' : ''%>" onclick="document.getElementById('<%=$this->Directive->ClientID%>').checked = <%=$this->getDefaultValue() ? 'true' : 'false'%>;" alt="<%[ Reset to default value ]%>" title="<%[ Reset to default value ]%>"></i>
	</div>
</div>
