<button type="button" onmousedown="openElementOnCursor(event, '<%=$this->FileSetIncludeMenu->ClientID%>_new_fileset', 0, 20);" class="w3-button w3-green w3-margin-bottom" style="display: <%=$this->getDirectiveName() == 'Include' ? '' : 'none'%>"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
<com:Application.Web.Portlets.NewFileSetIncExcMenu ID="FileSetIncludeMenu" />
<com:TActiveRepeater ID="RepeaterFileSetIncludes" OnItemDataBound="createFileSetIncludes">
	<prop:ItemTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive incexc">
			<h2><%#$this->SourceTemplateControl->getDirectiveName()%> #<%#$this->ItemIndex + 1%></h2>
			<button type="button" onmousedown="openElementOnCursor(event, '<%=$this->FileSetFileOptMenu->ClientID%>_new_fileset', 0, 20);" class="w3-button w3-green w3-margin-bottom"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
			<com:Application.Web.Portlets.NewFileSetFileOptMenu ID="FileSetFileOptMenu" />
		<com:TActiveRepeater
			ID="RepeaterFileSetOptions"
			ItemRenderer="Application.Web.Portlets.FileSetOptionRenderer"
		>
			<prop:HeaderTemplate>
				<div class="w3-card-4 w3-padding w3-margin-bottom directive">
					<h2><%[ Options ]%></h2>
			</prop:HeaderTemplate>
			<prop:FooterTemplate>
				</div>
			</prop:FooterTemplate>
		</com:TActiveRepeater>
		<com:TActiveRepeater ID="RepeaterFileSetInclude"  ItemRenderer="Application.Web.Portlets.DirectiveRenderer" CssClass="incexc_item">
			<prop:HeaderTemplate>
				<div class="w3-card-4 w3-padding w3-margin-bottom directive include_file">
					<h3><%[ Files ]%></h3>
			</prop:HeaderTemplate>
			<prop:FooterTemplate>
				</div>
			</prop:FooterTemplate>
		</com:TActiveRepeater>
		</div>
	</prop:ItemTemplate>
</com:TActiveRepeater>
<com:TActiveRepeater ID="RepeaterFileSetExclude" OnItemCreated="createFileSetIncExcElement" CssClass="incexc_item">
	<prop:HeaderTemplate>
		<div class="w3-card-4 w3-padding w3-margin-bottom directive">
			<h2><%[ Exclude ]%></h2>
			<button type="button" onmousedown="openElementOnCursor(event, '<%=$this->FileSetExcMenu->ClientID%>_new_fileset', 0, 20);" class="w3-button w3-green w3-margin-bottom"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
			<com:Application.Web.Portlets.NewFileSetExcMenu ID="FileSetExcMenu" />
			<h3><%[ Files ]%></h3>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
			<div class="directive_field exclude_file">
				<com:Application.Web.Portlets.DirectiveTextBox />
			</div>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</div>
	</prop:FooterTemplate>
</com:TActiveRepeater>
<div class="w3-modal" id="fileset_browser" style="display: none;">
	<div class="w3-modal-content w3-card-4 w3-padding-large w3-animate-zoom" style="width: 85%">
		<span onclick="document.getElementById('fileset_browser').style.display = 'none'" class="w3-button w3-xlarge w3-hover-red w3-display-topright">×</span>
		<h2><%[ Include files to FileSet]%></h2>
		<com:Application.Web.Portlets.FileSetBrowser ID="FSBrowser" />
		<com:TCallback ID="NewIncExcFile" OnCallback="newIncludeExcludeFile" />
		<div class="w3-center">
			<button type="button" class="w3-button w3-red" onclick="document.getElementById('fileset_browser').style.display = 'none'"><i class="fa fa-times"></i> &nbsp; <%[ Cancel ]%></button>
			<a class="w3-button w3-green button_fixed" onclick="set_include_exclude(); document.getElementById('fileset_browser').style.display = 'none'">
				<i class="fa fa-check"></i> &nbsp;<%[ Apply file selection ]%></a>
			</a>
		</div>
	</div>
</div>
<script type="text/javascript">
function set_include_exclude() {
	var request = <%=$this->NewIncExcFile->ActiveControl->Javascript%>;
	var param = {
		'Include': oFileSetBrowser.get_includes(),
		'Exclude': oFileSetBrowser.get_excludes()
	};
	request.setCallbackParameter(param);
	request.dispatch();
}
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
