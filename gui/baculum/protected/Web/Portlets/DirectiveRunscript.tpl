<div class="<%=!$this->display_directive ? ' hide' : '';%>">
<button type="button" class="w3-button w3-green" onmousedown="openElementOnCursor(event, '<%=$this->RunscriptMenu->ClientID%>_new_runscript', 0, 20);"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
<com:Application.Web.Portlets.NewRunscriptMenu ID="RunscriptMenu" />
<com:TActiveRepeater ID="RepeaterRunscriptOptions" OnItemDataBound="createRunscriptOptions">
	<prop:HeaderTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive runscript">
			<h2>Runscript</h2>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
		<%=($this->getItemIndex() % 7 === 0 ? '<h3 class="runscript">Runscript #' . (($this->getItemIndex()/7)+1) . '</h3><hr />' : '')%>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'ComboBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveComboBox	/>
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'TextBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveText />
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'CheckBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveBoolean />
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</fieldset>
	</prop:FooterTemplate>
</com:TActiveRepeater>
</div>
