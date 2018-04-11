<div id="<%=$this->ClientID%>_new_messages" class="w3-card w3-white w3-padding config_new_messages" style="display: none">
	<div class="right"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/close.png" alt="" style="margin: 3px 3px 0 0" onclick="$('#<%=$this->ClientID%>_new_messages').hide();" /></div>
	<ul style="margin-top: 0">
		<li><com:TActiveLinkButton
			ID="Console"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Console->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Console"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Stdout"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Stdout->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Stdout"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Stderr"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Stderr->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Stderr"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Syslog"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Syslog->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Syslog"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Catalog"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Catalog->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Catalog"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Director"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Director->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Director"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="File"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->File->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="File"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Append"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Append->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Append"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Mail"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Mail->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Mail"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="MailOnError"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->MailOnError->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="MailOnError"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="MailOnSuccess"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->MailOnSuccess->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="MailOnSuccess"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
		<li><com:TActiveLinkButton
			ID="Operator"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->Operator->ClientID%>').parents('div').find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Operator"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			/>
		</li>
	</ul>
</div>
