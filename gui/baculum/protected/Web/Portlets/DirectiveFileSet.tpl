<fieldset class="directive">
	<legend><%=$this->getDirectiveName()%></legend>
	<a href="javascript:void(0)" onmousedown="openElementOnCursor(event, '<%=$this->FileSetMenu->ClientID%>_new_fileset', 0, 20);"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/add.png" alt="<%[ Add ]%>" /> <%[ Add ]%></a>
	<com:Application.Web.Portlets.NewFileSetMenu ID="FileSetMenu" />
<com:TActiveRepeater ID="RepeaterFileSetOptions" OnItemDataBound="createFileSetOptions">
	<prop:HeaderTemplate>
	<fieldset class="directive">
		<legend><%[ Options ]%></legend>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
		<%=($this->getItemIndex() % 31 === 0 ? '<h3 class="options">Options #' . (($this->getItemIndex()/31)+1) . '</h3><hr />' : '')%>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'ComboBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveComboBox />
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
<com:TActiveRepeater ID="RepeaterFileSetInclude" OnItemCreated="createFileSetIncExcElement">
	<prop:HeaderTemplate>
	<fieldset class="directive include_file">
		<legend><%[ Files ]%></legend>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
			<div class="directive_field">
				<com:Application.Web.Portlets.DirectiveText />
			</div>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
	</fieldset>
	</prop:FooterTemplate>
</com:TActiveRepeater>
<com:TActiveRepeater ID="RepeaterFileSetExclude" OnItemCreated="createFileSetIncExcElement">
	<prop:HeaderTemplate>
	<fieldset class="directive">
		<legend><%[ Files ]%></legend>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
			<div class="directive_field">
				<com:Application.Web.Portlets.DirectiveText />
			</div>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
	</fieldset>
	</prop:FooterTemplate>
</com:TActiveRepeater>
</fieldset>
