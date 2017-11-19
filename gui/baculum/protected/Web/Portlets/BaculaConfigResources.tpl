<div class="config_resources" style="display: none">
	<com:TActiveLabel ID="RemoveResourceOk" Display="None" CssClass="validate" />
	<com:TActiveLabel ID="RemoveResourceError" Display="None" CssClass="validator" />
	<com:TActiveRepeater ID="RepeaterResources" OnItemCreated="createResourceListElement">
		<prop:ItemTemplate>
			<com:TPanel>
				<script type="text/javascript">
				<%=$this->Resource->ClientID%>_mousedown = function(event) {
					var t = (event.target||event.srcElement);
					var res_id = '<%=$this->Resource->ClientID%>';
					if (t.id != res_id && !/^<%=$this->RemoveResource->ClientID%>/.test(t.id)) {
						$('.validate, .validator').hide(); // hide validator messages
						$('#' + res_id).trigger('click');
					}
				};
				document.getElementById('<%=$this->RemoveResource->ClientID%>').onclick = function(event) {
					var t = (event.target||event.srcElement);
					var cmsg = '<%[ Are you sure that you want to remove %s resource "%s"? ]%>';
					cmsg = cmsg.replace('%s', '<%=$this->DataItem['resource_type']%>');
					cmsg = cmsg.replace('%s', '<%=$this->DataItem['resource_name']%>');
					if (/^<%=$this->RemoveResource->ClientID%>/.test(t.id) && confirm(cmsg)) {
						return true;
					}
					return false;
				};
				</script>
				<table class="resource" onmousedown="return <%=$this->Resource->ClientID%>_mousedown(event);" onmouseover="$(this).find('a.action_link').addClass('resource_selected');" onmouseout="$(this).find('a.action_link').removeClass('resource_selected');">
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
							<!--a class="action_link" href="javascript:void(0)"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/config.png" alt="<%[ Edit ]%>" /> <%[ Edit ]%></a -->
							<com:TActiveLinkButton
								ID="RemoveResource"
								OnCommand="SourceTemplateControl.removeResource"
								CssClass="action_link"
							>
							<prop:ClientSide.OnComplete>
								var vid = '<%=$this->SourceTemplateControl->RemoveResourceError->ClientId%>';
								if (document.getElementById(vid).style.display === 'none') {
									var container = $('#<%=$this->RemoveResource->ClientID%>').closest('div')[0];
									container.parentNode.removeChild(container);
								}
								$('html, body').animate({
									scrollTop: $('#' + vid).closest('div').prev().offset().top
								}, 500);
							</prop:ClientSide.OnComplete>
								<img id="<%=$this->RemoveResource->ClientID%>_img" src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="<%[ Remove ]%>" /> <%[ Remove ]%>
							</com:TActiveLinkButton>
						</td>
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
