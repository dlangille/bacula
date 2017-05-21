<div class="config_components" style="display: none">
	<com:TActiveRepeater ID="RepeaterComponents" OnItemCreated="createComponentListElement">
		<prop:ItemTemplate>
			<com:TPanel>
				<table class="component">
					<tr>
						<td onmousedown="(event.target||event.srcElement).id != '<%=$this->Component->ClientID%>' ? $('#<%=$this->Component->ClientID%>').trigger('click') : '';" style="width: 900px" ><com:TActiveLinkButton
							ID="Component"
							ActiveControl.EnableUpdate="false"
							OnCommand="SourceTemplateControl.getResources"
							ClientSide.OnLoading="BaculaConfig.loader_start(sender.options.ID);"
							ClientSide.OnComplete="BaculaConfig.set_config_items(sender.options.ID);"
							Attributes.onclick="return BaculaConfig.unset_config_items(this.id);"
							Text="<strong><%=$this->DataItem['label']%></strong>: <%=$this->DataItem['component_name']%>"
						/>
							<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/ajax-loader-arrows.gif" alt="" style="display: none" />
						</td>
						<td class="right">
							<a href="javascript:void(0)" onmousedown="openElementOnCursor(event, '<%=$this->ResourcesMenu->ClientID%>_new_resource', -80, 20);"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/add.png" alt="<%[ Add ]%>" /> <%[ Add ]%></a>
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
<div class="config_directives" rel="<%=$this->getHost()%>new_resource" style="display: none">
	<h2 rel="<%[ Add new %resource_type resource on %component_name (%component_type) ]%>"></h2>
	<hr />
	<com:Application.Web.Portlets.BaculaConfigDirectives ID="NewResource" LoadValues="<%=false%>" />
</div>
