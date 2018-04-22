<div class="w3-card-4 w3-padding w3-margin-bottom directive incexc">
	<h2><%=$this->getDirectiveName()%></h2>
	<button type="button" onmousedown="openElementOnCursor(event, '<%=$this->FileSetMenu->ClientID%>_new_fileset', 0, 20);" class="w3-button w3-green w3-margin-bottom"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
	<com:Application.Web.Portlets.NewFileSetMenu ID="FileSetMenu" />
<com:TActiveRepeater ID="RepeaterFileSetOptions" OnItemDataBound="createFileSetOptions">
	<prop:HeaderTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive">
			<h2><%[ Options ]%></h2>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
		<%=($this->getItemIndex() % 31 === 0 ? '<h3 class="options">Options #' . (($this->getItemIndex()/31)+1) . '</h3><hr />' : '')%>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'ComboBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveComboBox />
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
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'Integer')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveInteger />
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
		<com:Application.Web.Portlets.BConditional BCondition="<%#($this->Data['field_type'] === 'ListBox')%>">
			<prop:TrueTemplate>
				<com:Application.Web.Portlets.DirectiveListBox />
			</prop:TrueTemplate>
		</com:Application.Web.Portlets.BConditional>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</div>
	</prop:FooterTemplate>
</com:TActiveRepeater>
<com:TActiveRepeater ID="RepeaterFileSetInclude" OnItemCreated="createFileSetIncExcElement" CssClass="incexc_item">
	<prop:HeaderTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive include_file">
			<h2><%[ Files ]%></h2>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
			<div class="directive_field">
				<com:Application.Web.Portlets.DirectiveText />
			</div>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</div>
	</prop:FooterTemplate>
</com:TActiveRepeater>
<com:TActiveRepeater ID="RepeaterFileSetExclude" OnItemCreated="createFileSetIncExcElement" CssClass="incexc_item">
	<prop:HeaderTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive">
			<h2><%[ Files ]%></h2>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
			<div class="directive_field">
				<com:Application.Web.Portlets.DirectiveText />
			</div>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</div>
	</prop:FooterTemplate>
</com:TActiveRepeater>
</div>
<script type="text/javascript">
var incexc = document.getElementsByClassName('incexc');
for (var i = 0; i < incexc.length; i++) {
	var dvs = incexc[i].getElementsByTagName('H2');
	for (var j = 0; j < dvs.length; j++) {
		if (dvs[j].childNodes.length === 0) {
			incexc[i].style.display = 'none';
			break;
		}
	}
}
</script>
