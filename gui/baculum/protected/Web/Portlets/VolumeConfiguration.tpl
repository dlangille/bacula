<%@ MasterClass="Application.Web.Portlets.ConfigurationPanel"%>
<com:TContent ID="ConfigurationWindowContent">
	<com:TActivePanel DefaultButton="Apply">
		<strong><%[ Volume name: ]%> <com:TActiveLabel ID="VolumeName" /><com:TActiveLabel ID="VolumeID" Visible="false" /></strong>
		<span class="text tab tab_active" rel="volume_actions_tab"><%[ Actions ]%></span>
		<span class="text tab" rel="volume_jobs_tab"><%[ Jobs on Volume ]%></span>
		<hr class="tabs" />
		<div id="volume_actions_tab">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="VolumeGroup"
				AutoUpdate="true"
				Display="Dynamic"
				HeaderText="<%[ There is not possible to run selected action because: ]%>" />
			<div class="line">
				<div class="text"><com:TLabel ForControl="VolumeStatus" Text="<%[ Volume status: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="VolumeStatus" CssClass="textbox-auto" AutoPostBack="false" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="RetentionPeriod" Text="<%[ Retention period (in hours): ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="RetentionPeriod" MaxLength="20" CssClass="textbox-auto" AutoPostBack="false" />
					<com:TActiveCustomValidator ID="RetentionPeriodValidator" ValidationGroup="VolumeGroup" ControlToValidate="RetentionPeriod" ErrorMessage="<%[ Retention period value must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="retentionPeriodValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Pool" Text="<%[ Pool: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="Pool" AutoPostBack="false" CssClass="textbox-auto" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="UseDuration" Text="<%[ Vol. use duration (in hours): ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="UseDuration" AutoPostBack="false" CssClass="textbox-auto" />
					<com:TActiveCustomValidator ID="UseDurationValidator" ValidationGroup="VolumeGroup" ControlToValidate="UseDuration" ErrorMessage="<%[ Vol. use duration value must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="useDurationValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="MaxVolJobs" Text="<%[ Max vol. jobs: ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="MaxVolJobs" AutoPostBack="false" CssClass="textbox-auto" />
					<com:TActiveCustomValidator ID="MaxVolJobsValidator" ValidationGroup="VolumeGroup" ControlToValidate="MaxVolJobs" ErrorMessage="<%[ Max vol. jobs value must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="maxVolJobsValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="MaxVolFiles" Text="<%[ Max vol. files: ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="MaxVolFiles" AutoPostBack="false" CssClass="textbox-auto" />
					<com:TActiveCustomValidator ID="MaxVolFilesValidator" ValidationGroup="VolumeGroup" ControlToValidate="MaxVolFiles" ErrorMessage="<%[ Max vol. files value must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="maxVolFilesValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="MaxVolBytes" Text="<%[ Max vol. bytes: ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="MaxVolBytes" AutoPostBack="false" CssClass="textbox-auto" />
					<com:TActiveCustomValidator ID="MaxVolBytesValidator" ValidationGroup="VolumeGroup" ControlToValidate="MaxVolBytes" ErrorMessage="<%[ Max vol. bytes value must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="maxVolBytesValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Slot" Text="<%[ Slot number: ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="Slot" AutoPostBack="false" CssClass="textbox-auto" />
					<com:TActiveCustomValidator ID="SlotValidator" ValidationGroup="VolumeGroup" ControlToValidate="Slot" ErrorMessage="<%[ Slot value must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="slotValidator" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Recycle" Text="<%[ Recycle: ]%>" /></div>
				<div class="field"><com:TActiveCheckBox ID="Recycle" AutoPostBack="false" /></div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Enabled" Text="<%[ Enabled: ]%>" /></div>
				<div class="field"><com:TActiveCheckBox ID="Enabled" AutoPostBack="false" /></div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="InChanger" Text="<%[ In changer: ]%>" /></div>
				<div class="field"><com:TActiveCheckBox ID="InChanger" AutoPostBack="false" /></div>
			</div>
			<com:TCallback ID="ReloadVolumes" OnCallback="Page.VolumeWindow.prepareData" ClientSide.OnComplete="SlideWindow.getObj('VolumeWindow').setLoadRequest();" />
			<script type="text/javascript">
				var volume_callback_func = function() {
					/*
					 * Check if Volume list window is open and if any checkbox from actions is not checked.
					 * Also check if toolbar is open.
					 * If yes, then is possible to refresh Volume list window.
					 */
					var obj = SlideWindow.getObj('VolumeWindow');
					if (obj.isWindowOpen() === false || obj.areCheckboxesChecked() === true || obj.isToolbarOpen() === true) {
						return;
					}
					var mainForm = Prado.Validation.getForm();
					var callback = <%=$this->ReloadVolumes->ActiveControl->Javascript%>;
					if (Prado.Validation.managers[mainForm].getValidatorsWithError('VolumeGroup').length == 0) {
						obj.markAllChecked(false);
						callback.dispatch();
					}
				}
			</script>
			<div class="button">
				<com:Application.Web.Portlets.BActiveButton ID="Purge" Text="<%[ Purge ]%>" OnClick="purge" ClientSide.OnSuccess="ConfigurationWindow.getObj('VolumeWindow').progress(false);volume_callback_func()" />
				<com:Application.Web.Portlets.BActiveButton ID="Prune" Text="<%[ Prune ]%>" OnClick="prune" ClientSide.OnSuccess="ConfigurationWindow.getObj('VolumeWindow').progress(false);volume_callback_func()" />
				<com:Application.Web.Portlets.BActiveButton ValidationGroup="VolumeGroup" CausesValidation="true" ID="Apply" Text="<%[ Apply ]%>" OnClick="apply" ClientSide.OnSuccess="ConfigurationWindow.getObj('VolumeWindow').progress(false);volume_callback_func()" />
			</div>
		</div>
		<div id="volume_jobs_tab" style="display: none">
			<div style="max-height: 500px; overflow-y: auto;">
				<com:TActiveDataGrid
					ID="JobsOnVolume"
					EnableViewState="false"
					AutoGenerateColumns="false"
					AllowSorting="false"
					OnSortCommand="sortDataGrid"
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
			<p class="center bold" id="no_jobs_on_volume" style="display: none"><%[ No jobs on the volume. ]%></p>
			<script type="text/javascript">
				var bind_jobs_on_volume_action = function() {
					var grid_id = '<%=$this->JobsOnVolume->ClientID%>';
					var no_jobs_msg_id = 'no_jobs_on_volume';
					if (!document.getElementById(grid_id)) {
						$('#' + no_jobs_msg_id).show();
						return;
					}
					$('#' + no_jobs_msg_id).hide();
					SlideWindow.makeSortable(grid_id);
					var odd_rows = ['#' + grid_id, 'tr.' + SlideWindow.elements.contentItems].join(' ');
					var even_rows = ['#' + grid_id, 'tr.' + SlideWindow.elements.contentAlternatingItems].join(' ');
					var callback = <%=$this->OpenJobCall->ActiveControl->Javascript%>;
					$(odd_rows + ', ' + even_rows).each(function(index, el) {
						$(el).on('click', function(e) {
							var el = $(e.srcElement||e.target);

							if (el.length === 1) {
								el = el.parents('tr').find('input[type=hidden]');
								if (el.length === 1) {
									var val = el[0].value;
									callback.setCallbackParameter(val);
									callback.dispatch();
									ConfigurationWindow.getObj('VolumeWindow').progress(true);
								}
							}
						});
					}.bind(this));
					SlideWindow.sortTable(grid_id, 0, true);
				}.bind(this);
			</script>
			<com:TCallback ID="OpenJobCall" OnCallback="openJob">
				<prop:ClientSide.OnComplete>
					ConfigurationWindow.getObj('VolumeWindow').progress(false);
					ConfigurationWindow.getObj('JobWindow').switchTabByNo(2);
					ConfigurationWindow.getObj('JobWindow').show();
				</prop:ClientSide.OnComplete>
			</com:TCallback>
		</div>
	</com:TActivePanel>
</com:TContent>
