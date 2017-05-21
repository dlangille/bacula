<div id="<%=$this->ClientID%>_new_fileset" class="config_new_fileset left" style="display: none">
	<div class="right"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/close.png" alt="" style="margin: 3px 3px 0 0" onclick="$('#<%=$this->ClientID%>_new_fileset').hide();" /></div>
	<ul style="margin-top: 0">
		<li style="display: <%=$this->getDirectiveName() == 'Include' ? '' : 'none'%>"><com:TActiveLinkButton
			ID="IncludeFileItem"
			OnCommand="Parent.SourceTemplateControl.newIncludeFile"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->IncludeFileItem->ClientID%>').parents('fieldset').find('fieldset.include_file'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="File"
			Attributes.onclick="$(this).closest('div.config_new_fileset').hide();"
			/>
		</li>
		<li style="display: <%=$this->getDirectiveName() == 'Include' ? '' : 'none'%>"><com:TActiveLinkButton
			ID="OptionsItem"
			OnCommand="Parent.SourceTemplateControl.newIncludeOptions"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->OptionsItem->ClientID%>').parents('fieldset').find('h3.options'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Options"
			Attributes.onclick="$(this).closest('div.config_new_fileset').hide();"
			/>
		</li>
		<li style="display: <%=$this->getDirectiveName() == 'Exclude' ? '' : 'none'%>"><com:TActiveLinkButton
			OnCommand="Parent.SourceTemplateControl.newExcludeFile"
			CommandParameter="save"
			Text="Exclude"
			Attributes.onclick="$(this).closest('div.config_new_fileset').hide();"
			/>
		</li>
	</ul>
</div>
