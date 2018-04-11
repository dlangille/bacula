<div id="<%=$this->ClientID%>_new_fileset" class="w3-card w3-white w3-padding left config_new_fileset" style="display: none">
	<i class="fa fa-times w3-right" onclick="$('#<%=$this->ClientID%>_new_fileset').hide();" /></i>
	<ul class="w3-ul" style="margin-top: 0">
		<li style="display: <%=$this->getDirectiveName() == 'Include' ? '' : 'none'%>"><com:TActiveLinkButton
			ID="IncludeFileItem"
			OnCommand="Parent.SourceTemplateControl.newIncludeFile"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->IncludeFileItem->ClientID%>').parents('div').find('div.include_file'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="File"
			Attributes.onclick="$(this).closest('div.config_new_fileset').hide();"
			/>
		</li>
		<li style="display: <%=$this->getDirectiveName() == 'Include' ? '' : 'none'%>"><com:TActiveLinkButton
			ID="OptionsItem"
			OnCommand="Parent.SourceTemplateControl.newIncludeOptions"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->OptionsItem->ClientID%>').parents('div').find('h3.options'); BaculaConfig.scroll_to_element(el[el.length-1]);"
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
