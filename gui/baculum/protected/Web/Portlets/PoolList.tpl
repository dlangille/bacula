<%@ MasterClass="Application.Web.Portlets.SlideWindow"%>
<com:TContent ID="SlideWindowContent">
	<com:TActivePanel ID="RepeaterShow">
		<com:TActiveRepeater ID="Repeater">
			<prop:ItemTemplate>
				<div data-type="item_value" rel="<%#$this->DataItem->poolid%>" class="slide-window-element">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/pool.png" alt="" /><%#$this->DataItem->name%>
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
			<com:TActiveTemplateColumn HeaderText="Pool name" SortExpression="name">
				<prop:ItemTemplate>
					<div data-type="item_value" rel="<%=$this->getParent()->Data['poolid']%>"><%=$this->getParent()->Data['name']%></div>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveBoundColumn
				SortExpression="numvols"
				HeaderText="Vol. number"
				DataField="numvols"
				ItemStyle.HorizontalAlign="Center"
			/>
			<com:TActiveTemplateColumn HeaderText="Vol. retention" SortExpression="volretention">
				<prop:ItemTemplate>
					<div rel="<%=$this->getParent()->Data['volretention']%>"><%=(integer)($this->getParent()->Data['volretention'] / 3600 / 24)%> <%=$this->getParent()->Data['volretention'] < 172800 ? 'day' : 'days'%>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveTemplateColumn HeaderText="AutoPrune" SortExpression="autoprune" ItemStyle.HorizontalAlign="Center">
				<prop:ItemTemplate>
					<%=$this->getParent()->Data['autoprune'] == 1 ? 'Yes' : 'No'%>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
			<com:TActiveTemplateColumn HeaderText="Recycle" SortExpression="recycle" ItemStyle.HorizontalAlign="Center">
				<prop:ItemTemplate>
					<%=$this->getParent()->Data['recycle'] == 1 ? 'Yes' : 'No'%>
				</prop:ItemTemplate>
			</com:TActiveTemplateColumn>
		</com:TActiveDataGrid>
	</com:TActivePanel>
	<com:TCallback ID="DataElementCall" OnCallback="Page.PoolWindow.configure">
		<prop:ClientSide.OnComplete>
			ConfigurationWindow.getObj('PoolWindow').show();
			ConfigurationWindow.getObj('PoolWindow').progress(false);
		</prop:ClientSide.OnComplete>
	</com:TCallback>
</com:TContent>
