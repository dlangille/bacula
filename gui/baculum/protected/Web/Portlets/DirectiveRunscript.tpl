<a href="javascript:void(0)" onmousedown="openElementOnCursor(event, '<%=$this->RunscriptMenu->ClientID%>_new_runscript', 0, 20);"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/add.png" alt="<%[ Add ]%>" /> <%[ Add ]%></a>
<com:Application.Web.Portlets.NewRunscriptMenu ID="RunscriptMenu" />
<com:TActiveRepeater ID="RepeaterRunscriptOptions" OnItemDataBound="createRunscriptOptions">
	<prop:HeaderTemplate>
		<fieldset class="directive runscript">
			<legend>Runscript</legend>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
		<%=($this->getItemIndex() % 7 === 0 ? '<h3 class="runscript">Runscript #' . (($this->getItemIndex()/7)+1) . '</h3><hr />' : '')%>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'ComboBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveComboBox	/>
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'TextBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveText />
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'CheckBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveBoolean />
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</fieldset>
	</prop:FooterTemplate>
</com:TActiveRepeater>
