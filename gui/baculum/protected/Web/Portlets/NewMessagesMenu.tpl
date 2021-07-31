<div id="<%=$this->ClientID%>_new_messages" class="w3-card w3-white w3-padding config_new_messages" style="display: none">
	<i class="fa fa-times w3-right" onclick="$('#<%=$this->ClientID%>_new_messages').hide();" /></i>
	<ul class="w3-ul new_element_menu">
		<li><com:TActiveLinkButton
			ID="Console"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Console
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Stdout"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Stdout
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Stderr"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Stderr
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Syslog"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Syslog
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Catalog"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Catalog
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Director"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Director
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="File"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;File
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Append"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Append
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Mail"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Mail
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="MailOnError"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;MailOnError
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="MailOnSuccess"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;MailOnSuccess
		</com:TActiveLinkButton>
		</li>
		<li><com:TActiveLinkButton
			ID="Operator"
			OnCommand="Parent.SourceTemplateControl.newMessagesDirective"
			ClientSide.OnComplete="var el = $('#<%=$this->ClientID%>_new_messages').next().find('div.directive'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Attributes.onclick="$(this).closest('div.config_new_messages').hide();"
			>
			<i class='fa fa-plus'></i> &nbsp;Operator
		</com:TActiveLinkButton>
		</li>
	</ul>
</div>
