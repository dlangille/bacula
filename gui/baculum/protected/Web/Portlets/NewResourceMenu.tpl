<div id="<%=$this->ClientID%>_new_resource" class="w3-card w3-white w3-padding config_new_resource left" style="display: none">
	<i class="fa fa-times w3-right" onclick="$('#<%=$this->ClientID%>_new_resource').hide();"></i>
	<ul class="w3-ul" style="display: <%=$this->getComponentType() === 'dir' ? 'block': 'none'%>; margin-top: 0;">
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Director|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Director"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Director');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="JobDefs|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="JobDefs"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'JobDefs');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Client|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Client"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Client');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Job|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Job"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Job');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Storage|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Storage"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Storage');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Catalog|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Catalog"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Catalog');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Schedule|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Schedule"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Schedule');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Fileset|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="FileSet"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'FileSet');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Pool|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Pool"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Pool');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Messages|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Messages"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Messages');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Console|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Console"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Console');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Statistics|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Statistics"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Statistics');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
	</ul>
	<ul style="display: <%=$this->getComponentType() === 'sd' ? 'block': 'none'%>; margin-top: 0;">
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Director|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Director"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Director');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Storage|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Storage"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Storage');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Device|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Device"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Device');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Autochanger|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Autochanger"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Autochanger');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Messages|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Messages"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Messages');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Statistics|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Statistics"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Statistics');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
	</ul>
	<ul style="display: <%=$this->getComponentType() === 'fd' ? 'block': 'none'%>; margin-top: 0;">
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Director|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Director"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Director');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="FileDaemon|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="FileDaemon"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'FileDaemon');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Messages|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Messages"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Messages');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Statistics|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Statistics"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Statistics');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
	</ul>
	<ul style="display: <%=$this->getComponentType() === 'bcons' ? 'block': 'none'%>; margin-top: 0;">
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Director|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Director"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Director');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
		<li><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newResource"
			CommandParameter="Console|<%=$this->getHost()%>|<%=$this->getComponentType()%>|<%=$this->getComponentName()%>"
			Text="Console"
			ClientSide.OnComplete="BaculaConfig.show_new_config('<%=$this->getHost()%>new_resource', '<%=$this->getComponentType()%>', '<%=$this->getComponentName()%>', 'Console');"
			Attributes.onclick="$(this).closest('div.config_new_resource').hide();$('div.config_directives').slideUp();"
			/>
		</li>
	</ul>
</div>
