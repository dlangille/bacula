<com:TActiveLabel ID="RemoveResourceError" Display="None" CssClass="w3-text-red" />
<div id="resource_remove_ok" class="w3-modal" style="display: none">
	<div class="w3-modal-content w3-card-4 w3-green w3-padding-large w3-animate-zoom" style="width:600px">
		<span onclick="document.getElementById('resource_remove_ok').style.display='none'; window.history.back();" class="w3-button w3-xlarge w3-hover-red w3-display-topright">&times;</span>
		<p><com:TActiveLabel ID="RemoveResourceOk" Display="None" /></p>
	</div>
</div>
<div class="w3-modal resource_remove_confirm" style="display: none">
	<div class="w3-modal-content w3-card-4 w3-padding-large w3-animate-zoom" style="width:600px">
        	<span onclick="$(this).closest('div.resource_remove_confirm').hide();" class="w3-button w3-xlarge w3-hover-red w3-display-topright">&times;</span>
		<h2><%[ Remove resource ]%></h2>
		<p><%[ Are you sure you want to remove this resource? ]%></p>
		<div class="w3-center">
			<button type="button" class="w3-button w3-red" onclick="$(this).closest('div.resource_remove_confirm').hide();"><i class="fa fa-times"></i> &nbsp;<%[ Cancel ]%></button>
			<com:TActiveLinkButton
				ID="RemoveResource"
				OnCommand="SourceTemplateControl.removeResource"
				CssClass="w3-button w3-green"
				Visible="<%=$this->ShowRemoveButton && $this->LoadValues%>"
				Attributes.onclick="$(this).closest('div.resource_remove_confirm').hide();"
			>
				<prop:Text><i class="fa fa-trash-alt"></i> &nbsp;<%=Prado::localize('Remove resource')%></prop:Text>
				<prop:ClientSide.OnComplete>
					var remove_ok = document.getElementById('<%=$this->RemoveResourceOk->ClientID%>');
					if (remove_ok.style.display != 'none') {
						$('#<%=$this->ConfigDirectives->ClientID%>').slideUp();
						document.getElementById('resource_remove_ok').style.display = 'block';
					}
				</prop:ClientSide.OnComplete>
			</com:TActiveLinkButton>
		</div>
	</div>
</div>
<com:TActivePanel ID="ConfigDirectives">
	<com:Application.Web.Portlets.DirectiveSetting
		ID="DirectiveSetting"
		Resource="<%=$this->getResource()%>"
		OnLoadDirectives="loadDirectives"
		Visible="<%=$this->LoadValues%>"
	/>
	<com:TActiveLinkButton
		CssClass="w3-button w3-green w3-margin-bottom"
		Attributes.onclick="$(this).parent().prev('div.resource_remove_confirm').show();"
		Visible="<%=$this->ShowRemoveButton && $this->LoadValues%>"
	>
		<prop:Text><i class="fa fa-trash-alt"></i> &nbsp;<%=Prado::localize('Remove resource')%></prop:Text>
	</com:TActiveLinkButton>
	<com:TActiveRepeater
		ID="RepeaterDirectives"
		OnItemDataBound="createDirectiveElement"
		>
		<prop:ItemTemplate>
			<div class="directive_field">
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'TextBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveText />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'Integer')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveInteger />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'CheckBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveBoolean />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'TimePeriod')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveTimePeriod />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'ComboBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveComboBox />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'ListBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveListBox />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'FileSet')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveFileSet />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'Schedule')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveSchedule />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'Messages')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveMessages />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'Runscript')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveRunscript />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
			</div>
		</prop:ItemTemplate>
	</com:TActiveRepeater>
	<div class="w3-row w3-center">
		<com:TActiveLinkButton
			ID="Cancel"
			CssClass="w3-button w3-green"
			ActiveControl.EnableUpdate="false"
			OnCommand="TemplateControl.unloadDirectives"
			Attributes.onclick="$('div.config_directives').slideUp();"
			Visible="false"
		>
			<prop:Text>
				<i class="fa fa-times"></i> &nbsp;<%=Prado::localize('Cancel')%>
			</prop:Text>
		</com:TActiveLinkButton>
		<com:TActiveLinkButton
			ID="Save"
			CssClass="w3-button w3-green"
			ValidationGroup="Directive"
			ActiveControl.EnableUpdate="false"
			OnCommand="SourceTemplateControl.saveResource"
			CommandParameter="save"
		>
			<prop:Text>
				<i class="fa fa-save"></i> &nbsp;<%=Prado::localize('Save')%>
			</prop:Text>
			<prop:ClientSide.OnLoading>
				$('.save_progress').show();
				$('.save_done').hide();
			</prop:ClientSide.OnLoading>
			<prop:ClientSide.OnComplete>
				$('.save_progress').hide();
				$('.save_done').show();
				var err_el = '<%=$this->SaveDirectiveError->ClientID%>';
				if (document.getElementById(err_el).style.display == 'none') {
					<%=$this->SaveDirectiveActionOk%>
				}
			</prop:ClientSide.OnComplete>
			<prop:ClientSide.OnFailure>
				$('.save_progress').hide();
				$('.save_done').show();
			</prop:ClientSide.OnFailure>
		</com:TActiveLinkButton>
		<i class="fa fa-sync-alt w3-spin save_progress" style="display: none"></i>
		<com:TActiveLabel ID="SaveDirectiveOk" Display="None" CssClass="w3-text-green"><i class="fa fa-check save_done"></i> &nbsp;<%[ OK ]%></com:TActiveLabel>
		<com:TActiveLabel ID="SaveDirectiveError" Display="None" CssClass="w3-text-red"><i class="fa fa-times-circle save_done"></i> &nbsp;<%[ Error ]%></com:TActiveLabel>
		<br />
		<com:TActiveLabel ID="SaveDirectiveErrMsg" Display="None" CssClass="w3-text-red" />
	</div>
</com:TActivePanel>
