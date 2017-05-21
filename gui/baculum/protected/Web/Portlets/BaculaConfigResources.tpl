<div class="config_resources" style="display: none">
	<com:TActiveRepeater ID="RepeaterResources" OnItemCreated="createResourceListElement">
		<prop:ItemTemplate>
			<com:TPanel>
				<table class="resource" onmousedown="(event.target||event.srcElement).id != '<%=$this->Resource->ClientID%>' ? $('#<%=$this->Resource->ClientID%>').trigger('click') : '';" onmouseover="$(this).find('a.action_link').addClass('resource_selected');" onmouseout="$(this).find('a.action_link').removeClass('resource_selected');">
					<tr>
						<td><com:TActiveLinkButton
							ID="Resource"
							ActiveControl.EnableUpdate="false"
							OnCommand="SourceTemplateControl.getDirectives"
							ClientSide.OnLoading="BaculaConfig.loader_start(sender.options.ID);"
							ClientSide.OnComplete="BaculaConfig.set_config_items(sender.options.ID);"
							Attributes.onclick="return BaculaConfig.unset_config_items(this.id);"
							Text="<strong><%=$this->DataItem['resource_type']%></strong>: <%=$this->DataItem['resource_name']%>"
						/>
							<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/ajax-loader-arrows.gif" alt="" style="display: none" />
						</td>
						<td class="right" style="height: 26px">
							<!--a class="action_link" href="javascript:void(0)"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/config.png" alt="<%[ Edit ]%>" /> <%[ Edit ]%></a>
							<a class="action_link" href="javascript:void(0)"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="<%[ Remove ]%>" /> <%[ Remove ]%></a--></td>
					</tr>
				</table>
				<com:Application.Web.Portlets.BaculaConfigDirectives
					Resource="<%#$this->DataItem['resource_name']%>"
					LoadValues="<%=true%>"
				/>
			</com:TPanel>
		</prop:ItemTemplate>
	</com:TActiveRepeater>
</div>
