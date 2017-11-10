<a class="big" href="javascript:void(0)" onclick="$('#new_host').slideToggle()"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/add.png" alt="<%[ Add ]%>" /> <%[ Add API host]%></a>
<com:Application.Common.Portlets.NewHost ID="AddNewHost" APIRequired="config" ClientMode="true" OnCallback="loadConfig" />
<div class="config_hosts">
	<com:TActiveRepeater ID="RepeaterHosts" OnItemCreated="createHostListElement">
		<prop:ItemTemplate>
			<com:TPanel ID="HostBox" CssClass="config_host">
				<table class="host" onmousedown="$('div.config_host').removeClass('host_selected');(event.target||event.srcElement).id != '<%=$this->Host->ClientID%>' && (event.target||event.srcElement).id != '<%=$this->RemoveHost->ClientID%>' ? $('#<%=$this->Host->ClientID%>').trigger('click') : '';">
					<tr onmouseover="$(this).find('a.action_link').addClass('host_selected');" onmouseout="$(this).find('a.action_link').removeClass('host_selected');">
						<td><%[ Host: ]%> <com:TActiveLinkButton
							ID="Host"
							ActiveControl.EnableUpdate="false"
							OnCommand="SourceTemplateControl.getComponents"
							ClientSide.OnLoading="BaculaConfig.loader_start(sender.options.ID);"
							ClientSide.OnComplete="BaculaConfig.set_config_items(sender.options.ID);"
							ClientSide.OnSuccess="$('#<%=$this->HostBox->ClientID%>').addClass('host_selected');"
							Attributes.onclick="return BaculaConfig.unset_config_items(this.id);"
							CssClass="bold"
							Text="<%=$this->DataItem%>"
						/>
							<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/ajax-loader-arrows.gif" alt="" style="display: none" />
						</td>
						<td><%[ IP Address/Hostname: ]%><span class="bold"> <%=$this->getParent()->getParent()->config[$this->DataItem]['address']%></span></td>
						<td><%[ Port: ]%><span class="bold"> <%=$this->getParent()->getParent()->config[$this->DataItem]['port']%></span>
						</td>
						<td class="right"><com:TActiveLinkButton
							ID="RemoveHost"
							OnCommand="SourceTemplateControl.removeHost"
							CssClass="action_link"
						>
							<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="<%[ Remove ]%>" /> <%[ Remove ]%>
						</com:TActiveLinkButton>
						</td>

					</tr>
				</table>
				<com:Application.Web.Portlets.BaculaConfigComponents />
			</com:TPanel>
		</prop:ItemTemplate>
	</com:TActiveRepeater>
</div>
