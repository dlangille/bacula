<div class="<%=!$this->display_directive ? ' hide' : '';%>">
<button type="button" class="w3-button w3-green w3-margin" onmousedown="openElementOnCursor(event, '<%=$this->MessagesMenu->ClientID%>_new_messages', 0, 20);"><i class="fa fa-plus"></i> &nbsp;<%[ Add ]%></button>
<com:Application.Web.Portlets.NewMessagesMenu ID="MessagesMenu" />
<com:TActiveRepeater ID="RepeaterMessages" OnItemCreated="createDirectiveListElement" OnItemDataBound="loadMessageTypes">
	<prop:ItemTemplate>
		<div class="w3-card w3-white w3-padding directive">
			<h2><%=$this->Data['directive_name']%></h2>
			<com:Application.Web.Portlets.DirectiveText />
			<com:Application.Web.Portlets.MessageTypes ID="Types" />
		</div>
	</prop:ItemTemplate>
</com:TActiveRepeater>
</div>
