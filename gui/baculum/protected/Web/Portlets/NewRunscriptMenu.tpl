<div id="<%=$this->ClientID%>_new_runscript" class="w3-card w3-white w3-padding config_new_runscript left" style="display: none">
	<i class="fa fa-times w3-right" onclick="$('#<%=$this->ClientID%>_new_runscript').hide();" /></i>
	<ul style="margin-top: 0">
		<li><com:TActiveLinkButton
			ID="RunscriptItem"
			OnCommand="Parent.SourceTemplateControl.newRunscriptDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->RunscriptItem->ClientID%>').parents('div').find('h3.options'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Runscript"
			Attributes.onclick="$(this).closest('div.config_new_runscript').hide();"
			/>
		</li>
	</ul>
</div>
