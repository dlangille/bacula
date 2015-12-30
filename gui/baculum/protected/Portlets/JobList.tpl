<%@ MasterClass="Application.Portlets.SlideWindow" %>
<com:TContent ID="SlideWindowContent">
	<com:TActivePanel ID="RepeaterShow" EnableViewState="false">
	<com:TActiveRepeater ID="Repeater" EnableViewState="false">
		<prop:ItemTemplate>
			<com:TPanel ID="JobElement" CssClass="slide-window-element">
				<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/job-icon.png" alt="" /> [<%=@$this->DataItem->jobid%>] <%=@$this->DataItem->name%>
				<input type="hidden" name="<%=$this->ClientID%>" value="<%=isset($this->DataItem->jobid) ? $this->DataItem->jobid : ''%>" />
			</com:TPanel>
		</prop:ItemTemplate>
	</com:TActiveRepeater>
	</com:TActivePanel>
	<com:TActivePanel ID="DataGridShow">
	<com:TActiveDataGrid
		ID="DataGrid"
		EnableViewState="false"
		AutoGenerateColumns="false"
		AllowSorting="false"
		OnSortCommand="sortDataGrid"
		CellPadding="5px"
		CssClass="window-section-detail"
		ItemStyle.CssClass="slide-window-element"
		AlternatingItemStyle.CssClass="slide-window-element-alternating"
	>
		<com:TActiveTemplateColumn HeaderText="<input type='checkbox' name='actions_checkbox' onclick=SlideWindow.getObj('JobWindow').markAllChecked(this.checked)>" ItemStyle.HorizontalAlign="Center">
			<prop:ItemTemplate>
				<input type="checkbox" name="actions_checkbox" value="<%=$this->getParent()->Data['jobid']%>" id="<%=$this->getPage()->JobWindow->CheckedValues->ClientID%><%=$this->getParent()->Data['jobid']%>" rel="<%=$this->getPage()->JobWindow->CheckedValues->ClientID%>" onclick="SlideWindow.getObj('JobWindow').markChecked(this.getAttribute('rel'), this.checked, this.value, true);" />
			</prop:ItemTemplate>
                </com:TActiveTemplateColumn>
		<com:TActiveBoundColumn
			SortExpression="jobid"
			HeaderText="ID"
			DataField="jobid"
			ItemStyle.HorizontalAlign="Center"
		/>
		<com:TActiveTemplateColumn HeaderText="<%[ Job name ]%>" SortExpression="name">
			<prop:ItemTemplate>
				<div title="<%=$this->getParent()->Data['name']%>"><%=$this->getPage()->JobWindow->formatJobName($this->getParent()->Data['name'])%></div>
                                <input type="hidden" name="<%=$this->getParent()->ClientID%>" value="<%=$this->getParent()->Data['jobid']%>" />
			</prop:ItemTemplate>
		</com:TActiveTemplateColumn>
		<com:TActiveTemplateColumn ItemTemplate="<%=isset($this->getPage()->JobWindow->jobTypes[$this->getParent()->Data['type']]) ? $this->getPage()->JobWindow->jobTypes[$this->getParent()->Data['type']] : ''%>" SortExpression="type">
			<prop:HeaderText>
				<span title="<%=Prado::localize('Type')%>" style="cursor: help">T</span>
			</prop:HeaderText>
		</com:TActiveTemplateColumn>
		<com:TActiveTemplateColumn ItemTemplate="<%=array_key_exists($this->getParent()->Data['level'], $this->getPage()->JobWindow->jobLevels) ? $this->getPage()->JobWindow->jobLevels[$this->getParent()->Data['level']] : ''%>" SortExpression="level">
			<prop:HeaderText>
				<span title="<%=Prado::localize('Level')%>" style="cursor: help">L</span>
			</prop:HeaderText>
		</com:TActiveTemplateColumn>
		<com:TActiveTemplateColumn HeaderText="<%[ Job status ]%>" SortExpression="jobstatus">
			<prop:ItemTemplate>
				<div class="job-status-<%=$this->getPage()->JobWindow->getJobStatusLetter($this->getParent()->Data)%>" title="<%=$this->getPage()->JobWindow->getJobStatusDescription($this->getParent()->Data)%>"><%=$this->getPage()->JobWindow->getJobStatusValue($this->getParent()->Data)%></div>
			</prop:ItemTemplate>
		</com:TActiveTemplateColumn>
		<com:TActiveTemplateColumn HeaderText="<%[ Size ]%>" SortExpression="jobbytes">
			<prop:ItemTemplate>
				<div class="size" rel="<%=$this->getParent()->Data['jobbytes']%>"><%=$this->getParent()->Data['jobbytes']%></div>
			</prop:ItemTemplate>
		</com:TActiveTemplateColumn>
		<com:TActiveBoundColumn SortExpression="jobfiles" HeaderText="<%[ Files ]%>" DataField="jobfiles" />
		<com:TActiveTemplateColumn HeaderText="<%[ End time ]%>" SortExpression="endtime">
			<prop:ItemTemplate>
				<%=$this->getParent()->Data['endtime']%>
				<%=in_array($this->getParent()->Data['jobstatus'], $this->getPage()->JobWindow->runningJobStates) ? '<img src="' . $this->getPage()->getTheme()->getBaseUrl() . '/loader-alter.gif" />' : ''%>
			</prop:ItemTemplate>
		</com:TActiveTemplateColumn>
	</com:TActiveDataGrid>
	<com:TActiveHiddenField ID="CheckedValues" />
	</com:TActivePanel>
	<com:TCallback ID="DataElementCall" OnCallback="Page.JobWindow.configure">
		<prop:ClientSide.OnComplete>
			ConfigurationWindow.getObj('JobWindow').show();
			ConfigurationWindow.getObj('JobWindow').progress(false);
			status_callback_func();
		</prop:ClientSide.OnComplete>
	</com:TCallback>
	<com:TCallback ID="RunJobCall" OnCallback="Page.JobWindow.run_again">
		<prop:ClientSide.OnLoading>
			ConfigurationWindow.getObj('JobWindow').progress(true);
			var img_btn = $('run_job_again_btn');
			var img_src_path = img_btn.readAttribute('src').replace(/[^\/]+\S$/, '');
			img_btn.writeAttribute('disabled', 'disabled');
			img_btn.writeAttribute('src', img_src_path + 'loader.gif');
		</prop:ClientSide.OnLoading>
		<prop:ClientSide.OnComplete>
			var img_btn = $('run_job_again_btn');
			var img_src_path = img_btn.readAttribute('src').replace(/[^\/]+\S$/, '');
			img_btn.writeAttribute('src', img_src_path + 'play.png');
			img_btn.removeAttribute('disabled');
			ConfigurationWindow.getObj('JobWindow').progress(false);
			ConfigurationWindow.getObj('JobWindow').show();
			ConfigurationWindow.getObj('JobWindow').switchTabByNo(2);
			status_callback_func();
			oMonitor();
		</prop:ClientSide.OnComplete>
	</com:TCallback>
	<div class="actions_btn" style="display: none">
		<input type="image" id="run_job_again_btn" title="<%[ Run job again ]%>" src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/play.png" onclick="return false" />
	</div>
</com:TContent>
