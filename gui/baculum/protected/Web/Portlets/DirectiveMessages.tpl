<a href="javascript:void(0)" onmousedown="openElementOnCursor(event, '<%=$this->MessagesMenu->ClientID%>_new_messages', 0, 20);"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/add.png" alt="<%[ Add ]%>" /> <%[ Add ]%></a>
<com:Application.Web.Portlets.NewMessagesMenu ID="MessagesMenu" />
<com:TActiveRepeater ID="RepeaterMessages" OnItemCreated="createDirectiveListElement" OnItemDataBound="loadMessageTypes">
	<prop:ItemTemplate>
		<fieldset class="directive">
			<legend><%=$this->DataItem['directive_name']%></legend>
			<com:Application.Web.Portlets.DirectiveText />
			<com:Application.Web.Portlets.MessageTypes ID="Types" />
		</fieldset>
	</prop:ItemTemplate>
</com:TActiveRepeater>
