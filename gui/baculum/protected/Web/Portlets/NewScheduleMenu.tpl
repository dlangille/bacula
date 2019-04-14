<div id="<%=$this->ClientID%>_new_schedule" class="w3-card w3-white w3-padding config_new_schedule left" style="display: none">
	<i class="fa fa-times w3-right" onclick="$('#<%=$this->ClientID%>_new_schedule').hide();" /></i>
	<ul style="margin-top: 0">
		<li><com:TActiveLinkButton
			ID="RunscriptItem"
			OnCommand="Parent.SourceTemplateControl.newScheduleDirective"
			CommandParameter="save"
			ClientSide.OnComplete="var el = $('#<%=$this->RunscriptItem->ClientID%>').parents('div').find('h2'); BaculaConfig.scroll_to_element(el[el.length-1]);"
			Text="Run"
			Attributes.onclick="$(this).closest('div.config_new_schedule').hide();"
			/>
		</li>
	</ul>
</div>
