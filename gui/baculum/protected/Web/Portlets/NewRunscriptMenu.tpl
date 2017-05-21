<div id="<%=$this->ClientID%>_new_runscript" class="config_new_runscript left" style="display: none">
	<div class="right"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/close.png" alt="" style="margin: 3px 3px 0 0" onclick="$('#<%=$this->ClientID%>_new_runscript').hide();" /></div>
	<ul style="margin-top: 0">
		<li><com:TActiveLinkButton
			ID="RunscriptItem"
			OnCommand="Parent.SourceTemplateControl.newRunscriptDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->RunscriptItem->ClientID%>').parents('div.directive_field').find('fieldset').find('h3.runscript'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Runscript"
			Attributes.onclick="$(this).closest('div.config_new_runscript').hide();"
			/>
		</li>
	</ul>
</div>
