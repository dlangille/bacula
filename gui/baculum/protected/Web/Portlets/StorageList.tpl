<%@ MasterClass="Application.Web.Portlets.SlideWindow" %>
<com:TContent ID="SlideWindowContent">
	<com:TActivePanel ID="RepeaterShow">
		<com:TActiveRepeater ID="Repeater">
			<prop:ItemTemplate>
				<div data-type="item_value" rel="<%#$this->DataItem->storageid%>" class="slide-window-element">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/server-storage-icon.png" alt="" /><%#$this->DataItem->name%>
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
			<com:TActiveTemplateColumn HeaderText="<%[ Storage name ]%>" SortExpression="name">
				<prop:ItemTemplate>
					<div data-type="item_value" rel="<%=$this->getParent()->Data['storageid']%>"><%=$this->getParent()->Data['name']%></div>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveTemplateColumn HeaderText="<%[ Autochanger ]%>" SortExpression="autochanger" ItemStyle.HorizontalAlign="Center">
				<prop:ItemTemplate>
					<%=$this->getParent()->Data['autochanger'] == 1 ? Prado::localize('Yes') : Prado::localize('No')%>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
		</com:TActiveDataGrid>
	</com:TActivePanel>
	<com:TCallback ID="DataElementCall" OnCallback="Page.StorageWindow.configure">
		<prop:ClientSide.OnComplete>
			ConfigurationWindow.getObj('StorageWindow').show();
			ConfigurationWindow.getObj('StorageWindow').progress(false);
		</prop:ClientSide.OnComplete>
	</com:TCallback>
</com:TContent>
