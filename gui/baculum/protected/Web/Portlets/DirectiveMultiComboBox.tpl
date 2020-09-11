<com:TActiveRepeater ID="MultiComboBoxRepeater" OnItemDataBound="createMultiComboBoxElement">
	<prop:ItemTemplate>
		<div class="directive_field w3-row w3-margin-bottom<%=!isset($this->Data['show']) ? ' hide' : '';%>">
			<div class="w3-col w3-quarter"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" />:</div>
			<div class="w3-col w3-threequarter directive_value">
				<com:TActiveDropDownList ID="Directive"
					CssClass="w3-input w3-border w3-twothird"
					AutoPostBack="false"
					ActiveControl.EnableUpdate="false"
				/> 	<com:TActiveLinkButton ID="AddFieldBtn"
						OnCommand="SourceTemplateControl.addField"
						CommandParameter="save"
					>
					<i class="fa fa-plus" title="<%[ Add directive ]%>" alt="<%[ Add directive ]%>"></i>
				</com:TActiveLinkButton>
				<i class="fa fa-trash-alt remove_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').value = '';" alt="<%[ Remove directive ]%>" title="<%[ Remove directive ]%>"></i>
			</div>
		</div>
	</prop:ItemTemplate>
</com:TActiveRepeater>
