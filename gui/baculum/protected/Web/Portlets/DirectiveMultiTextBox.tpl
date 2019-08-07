<com:TActiveRepeater ID="MultiTextBoxRepeater" OnItemDataBound="createMultiTextBoxElement">
	<prop:ItemTemplate>
		<div class="directive_field w3-row w3-margin-bottom<%=!$this->Data['show'] ? ' hide' : '';%>">
			<div class="w3-col w3-quarter"><com:TActiveLabel ID="Label" ActiveControl.EnableUpdate="false" />:</div>
			<div class="w3-col w3-threequarter directive_value">
				<com:TActiveTextBox ID="Directive"
					CssClass="w3-input w3-border w3-twothird"
					ActiveControl.EnableUpdate="false"
					AutoTrim="true"
				/> 	<com:TActiveLinkButton ID="AddFieldBtn"
						OnCommand="SourceTemplateControl.addField"
						CommandParameter="save"
						ClientSide.OnComplete="var el = $('#<%=$this->SourceTemplateControl->MultiTextBoxRepeater->ClientID%>_Container').find('input[type=\'text\']'); el[el.length-1].focus();"
					>
					<prop:Text>
						<i class="fa fa-plus" title="<%[ Add directive ]%>" alt="<%[ Add directive ]%>"></i>
					</prop:Text>
				</com:TActiveLinkButton>
				<i class="fa fa-trash-alt remove_btn" onclick="document.getElementById('<%=$this->Directive->ClientID%>').value = '';" alt="<%[ Remove directive ]%>" title="<%[ Remove directive ]%>"></i>
			</div>
		</div>
	</prop:ItemTemplate>
</com:TActiveRepeater>
