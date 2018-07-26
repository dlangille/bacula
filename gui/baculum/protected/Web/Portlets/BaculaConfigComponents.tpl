<div class="config_components w3-margin-left" style="display: none">
	<com:TActiveRepeater ID="RepeaterComponents" OnItemCreated="createComponentListElement">
		<prop:ItemTemplate>
			<com:TPanel>
				<table class="component" style="width: 100%">
					<tr>
						<td onmousedown="var el = (event.target||event.srcElement); el.parentNode.id != '<%=$this->Component->ClientID%>' && el.id != '<%=$this->Component->ClientID%>' ? $('#<%=$this->Component->ClientID%>').trigger('click') : '';" style="width: 80%; cursor: pointer;" class="w3-threequarter">
						<com:TActiveLinkButton
							ID="Component"
							ActiveControl.EnableUpdate="false"
							OnCommand="SourceTemplateControl.getResources"
							ClientSide.OnLoading="BaculaConfig.loader_start(sender.options.ID);"
							ClientSide.OnComplete="BaculaConfig.set_config_items(sender.options.ID);"
							Attributes.onclick="return BaculaConfig.unset_config_items(this.id);"
							Text="<strong><%=$this->Data['label']%></strong>: <%=$this->Data['component_name']%>"
							Style="text-decoration: none"
						/>
							<i class="fa fa-sync w3-spin" style="display: none"><i/>
						</td>
						<td class="right" style="width: 20%">
							<a class="w3-button w3-green w3-right button_fixed" href="javascript:void(0)" onmousedown="openElementOnCursor(event, '<%=$this->ResourcesMenu->ClientID%>_new_resource', -80, 20);"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></a>
							<com:Application.Web.Portlets.NewResourceMenu ID="ResourcesMenu" />
						</td>
					</tr>
				</table>
				<com:Application.Web.Portlets.BaculaConfigResources />
			</com:TPanel>
		</prop:ItemTemplate>
	</com:TActiveRepeater>
</div>
	<com:TActiveLabel ID="ErrorMsg" Display="None" />
<div class="config_directives new_resource" rel="<%=$this->getHost()%>new_resource" style="display: none">
	<h2 rel="<%[ Add new %resource_type resource on %component_name (%component_type) ]%>"></h2>
	<hr />
	<com:Application.Web.Portlets.BaculaConfigDirectives ID="NewResource" LoadValues="<%=false%>" />
</div>
