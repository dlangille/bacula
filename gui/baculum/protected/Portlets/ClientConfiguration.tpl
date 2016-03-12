<%@ MasterClass="Application.Portlets.ConfigurationPanel"%>
<com:TContent ID="ConfigurationWindowContent">
	<com:TActivePanel DefaultButton="Apply">
		<h4><%[ Client name: ]%> <com:TActiveLabel ID="ClientName" /><com:TActiveLabel ID="ClientIdentifier" Visible="false" /></h4>
		<com:TActiveLabel ID="ClientDescription" CssClass="description" />
		<span class="text tab tab_active" rel="client_actions_tab"><%[ Actions ]%></span>
		<span class="text tab" rel="client_console_tab"><%[ Console status ]%></span>
		<span class="text tab" rel="client_jobs_tab"><%[ Jobs for Client ]%></span>
		<hr class="tabs" />
		<div id="client_actions_tab">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="ClientGroup"
				AutoUpdate="true"
				Display="Dynamic"
				HeaderText="<%[ There is not possible to run selected action because: ]%>" />
			<div class="line">
				<div class="text"><com:TLabel ForControl="FileRetention" Text="<%[ File retention (in days): ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="FileRetention" MaxLength="14" AutoPostBack="false" CssClass="textbox-auto" Text="" />
					<com:TActiveCustomValidator ID="FileRetentionValidator" ValidationGroup="ClientGroup" ControlToValidate="FileRetention" ErrorMessage="<%[ File retention value must be positive integer or zero. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="fileRetentionValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="JobRetention" Text="<%[ Job retention (in days): ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="JobRetention" MaxLength="14" AutoPostBack="false" CssClass="textbox-auto" Text="" />
					<com:TActiveCustomValidator ID="JobRetentionValidator" ValidationGroup="ClientGroup" ControlToValidate="JobRetention" ErrorMessage="<%[ Job retention value must be positive integer or zero. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="jobRetentionValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="AutoPrune" Text="<%[ AutoPrune: ]%>" /></div>
				<div class="field"><com:TActiveCheckBox ID="AutoPrune" AutoPostBack="false" /></div>
			</div>
			<com:TCallback ID="ReloadClients" OnCallback="Page.ClientWindow.prepareData" ClientSide.OnComplete="SlideWindow.getObj('ClientWindow').setLoadRequest();" />
			<script type="text/javascript">
				var client_callback_func = function() {
					/* If Client list window is not open or if actually toolbar is used
					 * then Client window refresh does not take place.
					 */
					var obj = SlideWindow.getObj('ClientWindow');
					if (obj.isWindowOpen() === false || obj.isToolbarOpen() === true) {
						return;
					}
					var mainForm = Prado.Validation.getForm();
					var callback = <%=$this->ReloadClients->ActiveControl->Javascript%>;
					if (Prado.Validation.managers[mainForm].getValidatorsWithError('ClientGroup').length == 0) {
						callback.dispatch();
					}
				}
			</script>
			<div class="button">
				<com:BActiveButton ID="Status" Text="<%[ Status ]%>" OnClick="status">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('ClientWindow').progress(false);
						ConfigurationWindow.getObj('ClientWindow').switchTab('client_console_tab');
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
				<com:BActiveButton ValidationGroup="ClientGroup" OnClick="apply" CausesValidation="true" ID="Apply" Text="<%[ Apply ]%>">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('ClientWindow').progress(false);
						client_callback_func();
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
			</div>
		</div>
		<div id="client_console_tab" style="display: none">
			<div class="field-full">
				<com:TActiveTextBox ID="ShowClient" TextMode="MultiLine" CssClass="textbox-auto" Style="height: 440px" ReadOnly="true" />
			</div>
		</div>
		<div id="client_jobs_tab" style="display: none">
			<div style="max-height: 444px; overflow-y: auto;">
				<com:TActiveDataGrid
					ID="JobsForClient"
					EnableViewState="false"
					AutoGenerateColumns="false"
					AllowSorting="false"
					CellPadding="5px"
					CssClass="window-section-detail"
					ItemStyle.CssClass="slide-window-element"
					AlternatingItemStyle.CssClass="slide-window-element-alternating"
					>
					<com:TActiveBoundColumn HeaderText="ID" DataField="jobid" />
					<com:TActiveTemplateColumn HeaderText="<%[ Job name ]%>" SortExpression="name">
						<prop:ItemTemplate>
							<div title="<%=$this->getParent()->Data['name']%>"><%=$this->getPage()->JobWindow->formatJobName($this->getParent()->Data['name'])%></div>
							<input type="hidden" name="<%=$this->getParent()->ClientID%>" value="<%=$this->getParent()->Data['jobid']%>" />
						</prop:ItemTemplate>
					</com:TActiveTemplateColumn>
					<com:TActiveTemplateColumn>
						<prop:HeaderText>
							<span title="<%=Prado::localize('Type')%>" style="cursor: help">T</span>
						</prop:HeaderText>
						<prop:ItemTemplate>
							<%=$this->getParent()->Data['type']%>
						</prop:ItemTemplate>
					</com:TActiveTemplateColumn>
					<com:TActiveTemplateColumn>
						<prop:HeaderText>
							<span title="<%=Prado::localize('Level')%>" style="cursor: help">L</span>
						</prop:HeaderText>
						<prop:ItemTemplate>
							<%=$this->getParent()->Data['level']%>
						</prop:ItemTemplate>
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
			</div>
			<p class="center bold" id="no_jobs_for_client" style="display: none"><%[ No jobs for the client. ]%></p>
			<script type="text/javascript">
				var bind_jobs_for_client_action = function() {
					var grid_id = '<%=$this->JobsForClient->ClientID%>';
					var no_jobs_msg_id = 'no_jobs_for_client';
					if (!document.getElementById(grid_id)) {
						$(no_jobs_msg_id).show();
						return;
					}
					$(no_jobs_msg_id).hide();
					SlideWindow.makeSortable(grid_id);
					var odd_rows = ['#' + grid_id, 'tr.' + SlideWindow.elements.contentItems].join(' ');
					var even_rows = ['#' + grid_id, 'tr.' + SlideWindow.elements.contentAlternatingItems].join(' ');
					var callback = <%=$this->OpenJobCall->ActiveControl->Javascript%>;
					$$(odd_rows, even_rows).each(function(el) {
						el.observe('click', function(e) {
							var el = $(e.srcElement||e.target);
							if (el) {
								el = el.up('tr').down('input[type=hidden]')
								var val = el.getValue();
								callback.ActiveControl.CallbackParameter = val;
								callback.dispatch();
								ConfigurationWindow.getObj('ClientWindow').progress(true);
							}
						}.bind(this));
					}.bind(this));
					SlideWindow.sortTable(grid_id, 0, true);
				}.bind(this);
			</script>
			<com:TCallback ID="OpenJobCall" OnCallback="openJob">
				<prop:ClientSide.OnComplete>
					ConfigurationWindow.getObj('ClientWindow').progress(false);
					ConfigurationWindow.getObj('JobWindow').switchTabByNo(2);
					ConfigurationWindow.getObj('JobWindow').show();
				</prop:ClientSide.OnComplete>
			</com:TCallback>
		</div>
	</com:TActivePanel>
</com:TContent>
