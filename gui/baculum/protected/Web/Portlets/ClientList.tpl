<%@ MasterClass="Application.Web.Portlets.SlideWindow"%>
<com:TContent ID="SlideWindowContent">
	<com:TActivePanel ID="RepeaterShow">
		<com:TActiveRepeater ID="Repeater">
			<prop:ItemTemplate>
				<div data-type="item_value" rel="<%#$this->DataItem->clientid%>" class="slide-window-element" title="<%=@$this->DataItem->uname%>">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/client-icon.png" alt="" /><%#$this->DataItem->name%>
				</div>
			</prop:ItemTemplate>
		</com:TActiveRepeater>
	</com:TActivePanel>
	<com:TActivePanel ID="DataGridShow">
		<com:TActiveDataGrid
			ID="DataGrid"
			AutoGenerateColumns="false"
			AllowSorting="false"
			OnSortCommand="sortDataGrid"
			CellPadding="5px"
			CssClass="window-section-detail-smallrow"
			ItemStyle.CssClass="slide-window-element"
			AlternatingItemStyle.CssClass="slide-window-element-alternating"
		>
			<com:TActiveTemplateColumn HeaderText="<%[ Client name ]%>" SortExpression="name">
				<prop:ItemTemplate>
					<div data-type="item_value" rel="<%=$this->getParent()->Data['clientid']%>"><%=$this->getParent()->Data['name']%></div>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveTemplateColumn ItemStyle.HorizontalAlign="Center" HeaderText="AutoPrune" SortExpression="autoprune">
				<prop:ItemTemplate>
					<%=$this->getParent()->Data['autoprune'] == 1 ? Prado::localize('Yes') : Prado::localize('No')%>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveTemplateColumn HeaderText="<%[ File Retention ]%>" SortExpression="fileretention">
				<prop:ItemTemplate>
					<div rel="<%=$this->getParent()->Data['fileretention']%>"><%=(integer)($this->getParent()->Data['fileretention'] / 3600 / 24)%> <%=$this->getParent()->Data['fileretention'] < 172800 ? Prado::localize('day') : Prado::localize('days')%></div>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveTemplateColumn HeaderText="<%[ Job Retention ]%>" SortExpression="jobretention">
				<prop:ItemTemplate>
					<%=(integer)($this->getParent()->Data['jobretention'] / 3600 / 24)%> <%=$this->getParent()->Data['jobretention'] < 172800 ? Prado::localize('day') : Prado::localize('days')%>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
		</com:TActiveDataGrid>
	</com:TActivePanel>
	<com:TCallback ID="DataElementCall" OnCallback="Page.ClientWindow.configure">
		<prop:ClientSide.OnComplete>
			ConfigurationWindow.getObj('ClientWindow').show();
			Formatters.set_formatters();
			bind_jobs_for_client_action();
			ConfigurationWindow.getObj('ClientWindow').progress(false);
		</prop:ClientSide.OnComplete>
	</com:TCallback>
</com:TContent>
