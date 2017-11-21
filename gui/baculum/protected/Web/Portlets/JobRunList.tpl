<%@ MasterClass="Application.Web.Portlets.SlideWindow" %>
<com:TContent ID="SlideWindowContent">
	<com:TActivePanel ID="RepeaterShow">
	<com:TActiveRepeater ID="Repeater">
		<prop:ItemTemplate>
			<%=($this->getPage()->JobRunWindow->oldDirector != $this->DataItem['director']) ? '<div class="window-section"><span>' . Prado::localize('Director:') . ' ' . $this->DataItem['director']  . '<span></div>': ''%>
			<div data-type="item_value" rel="<%#$this->DataItem['name']%>" class="slide-window-element">
				<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/job-icon.png" alt="" /> <%#$this->DataItem['name']%>
			</div>
			<%=!($this->getPage()->JobRunWindow->oldDirector = $this->DataItem['director'])%>
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
		<com:TActiveTemplateColumn HeaderText="<%[ Job name ]%>" SortExpression="name">
			<prop:ItemTemplate>
				<div data-type="item_value" rel="<%=$this->getParent()->DataItem['name']%>"><%=$this->getParent()->DataItem['name']%></div>
			</prop:ItemTemplate>
		</com:TActiveTemplateColumn>
		<com:TActiveBoundColumn
				SortExpression="director"
				HeaderText="<%[ Director ]%>"
				DataField="director"
				ItemStyle.HorizontalAlign="Center"
			/>
	</com:TActiveDataGrid>
	</com:TActivePanel>
	<com:TCallback ID="DataElementCall" OnCallback="Page.JobRunWindow.configure">
		<prop:ClientSide.OnComplete>
			ConfigurationWindow.getObj('JobRunWindow').show();
			ConfigurationWindow.getObj('JobRunWindow').progress(false);
		</prop:ClientSide.OnComplete>
	</com:TCallback>
</com:TContent>
