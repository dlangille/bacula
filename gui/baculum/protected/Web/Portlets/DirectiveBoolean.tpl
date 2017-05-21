<div class="line<%=!$this->display_directive ? ' hide' : '';%>">
	<div class="text"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" Visible="<%=$this->display_directive%>" />:</div>
	<div class="field directive_value">
		<com:TActiveCheckBox
			ID="Directive"
			AutoPostBack="false"
			Visible="<%=$this->display_directive%>"
			OnCheckedChanged="saveValue"
			ActiveControl.EnableUpdate="false"
		/>
		<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/switch.png" class="reset_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').checked = <%=$this->getDefaultValue() ? 'true' : 'false'%>;" alt="<%[ Reset to default value ]%>" title="<%[ Reset to default value ]%>" />
	</div>
</div>
