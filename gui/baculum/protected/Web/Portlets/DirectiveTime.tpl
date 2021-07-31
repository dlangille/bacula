<div class="directive_field w3-row w3-border w3-padding w3-margin-bottom<%=!$this->display_directive ? ' hide' : '';%>">
	<div class="w3-col w3-left" style="width: 180px; padding: 8px 0;">
		<com:TActiveLabel
			ID="Label"
			ActiveControl.EnableUpdate="false"
			Visible="<%=$this->display_directive%>"
		 />:
	</div>
	<div class="w3-col w3-left directive_value" style="width: 250px">
		<com:TActiveDropDownList ID="Hour"
			OnSelectedIndexChanged="saveValue"
			CssClass="w3-input w3-border w3-show-inline-block"
			Visible="<%=($this->display_directive && $this->ShowHour)%>"
			ActiveControl.EnableUpdate="false"
			AutoPostBack="false"
		/><span<%=!$this->ShowHour ? ' style="display: none"' : '' %>> : </span>
		<com:TActiveDropDownList ID="Minute"
			OnSelectedIndexChanged="saveValue"
			CssClass="w3-input w3-border w3-show-inline-block"
			Visible="<%=($this->display_directive && $this->ShowMinute)%>"
			ActiveControl.EnableUpdate="false"
			AutoPostBack="false"
		/> <%=$this->getRequired() ? '&nbsp;<i class="fa fa-asterisk w3-text-red" style="line-height: 40px"></i>' : ''%>
		<i class="fa fa-undo reset_btn<%=!$this->ShowResetButton ? ' hide' : ''%>" onclick="document.getElementById('<%=$this->Hour->ClientID%>').value = '<%=isset($this->DefaultValue->Hour) ? $this->DefaultValue->Hour : ''%>';document.getElementById('<%=$this->Minute->ClientID%>').value = '<%=isset($this->DefaultValue->Minute) ? $this->DefaultValue->Minute : ''%>';" alt="<%[ Reset to default value ]%>" title="<%[ Reset to default value ]%>"></i>
		<i class="fa fa-trash-alt remove_btn<%=!$this->ShowRemoveButton ? ' hide' : ''%>" onclick="document.getElementById('<%=$this->Hour->ClientID%>').value = '';document.getElementById('<%=$this->Minute->ClientID%>').value = '';" alt="<%[ Remove directive ]%>" title="<%[ Remove directive ]%>"></i>
		<com:TCustomValidator
			ID="TimeValidator"
			ValidationGroup="<%=$this->getValidationGroup()%>"
			Display="Dynamic"
			ControlToValidate="Hour"
			ClientValidationFunction="<%=$this->Hour->ClientID%>_validation_func"
			FocusOnError="true"
			Text="Field required."
			Enabled="<%=$this->getRequired() && $this->getShow()%>"
			Visible="<%=$this->ShowMinute%>"
		/>
	</div>
</div>
