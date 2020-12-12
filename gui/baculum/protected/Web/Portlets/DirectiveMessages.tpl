<button type="button" class="w3-button w3-green w3-margin" onmousedown="openElementOnCursor(event, '<%=$this->MessagesMenu->ClientID%>_new_messages', 0, 20);"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
<p class="bold"><%[ Tip: checking 'All' message type causes, that rest checked message types are saved with negation ex. Catalog = All, !Debug, !Saved, !Skipped ]%></p>
<com:Application.Web.Portlets.NewMessagesMenu ID="MessagesMenu" />
<com:TActiveRepeater ID="RepeaterMessages" OnItemCreated="createDirectiveListElement" OnItemDataBound="loadMessageTypes">
	<prop:ItemTemplate>
		<div class="w3-card w3-padding directive">
			<com:TActiveLinkButton
				CssClass="w3-button w3-green w3-right"
				OnCommand="SourceTemplateControl.removeMessages"
				CommandName="<%=$this->ItemIndex%>"
				CommandParameter="save"
			>
				<i class="fa fa-trash-alt"></i> &nbsp;<%[ Remove ]%>
			</com:TActiveLinkButton>
			<h2><%#$this->Data['directive_name']%></h2>
			<com:Application.Web.Portlets.DirectiveTextBox />
			<com:Application.Web.Portlets.MessageTypes ID="Types" />
		</div>
	</prop:ItemTemplate>
</com:TActiveRepeater>
