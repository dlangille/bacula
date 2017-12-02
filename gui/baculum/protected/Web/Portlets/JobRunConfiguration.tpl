<%@ MasterClass="Application.Web.Portlets.ConfigurationPanel"%>
<com:TContent ID="ConfigurationWindowContent">
	<com:TActivePanel DefaultButton="Run">
		<h4><%[ Job name: ]%> <com:TActiveLabel ID="JobName" /><com:TActiveLabel ID="JobID" Visible="false" /></h4>
		<span class="text tab tab_active" rel="jobrun_actions_tab"><%[ Actions ]%></span>
		<span class="text tab" rel="jobrun_console_tab"><%[ Console status ]%></span>
		<hr class="tabs" />
		<div id="jobrun_actions_tab">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="JobRunGroup"
				AutoUpdate="true"
				Display="Dynamic"
				HeaderText="<%[ There is not possible to run selected action because: ]%>" />
			<div class="line">
				<div class="text"><com:TLabel ForControl="Level" Text="<%[ Level: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="Level" AutoPostBack="false" CssClass="textbox-auto">
						<prop:Attributes.onchange>
							var job_to_verify = $('#<%=$this->JobToVerifyOptionsLine->ClientID%>');
							var verify_options = $('#<%=$this->JobToVerifyOptionsLine->ClientID%>');
							var verify_by_job_name = $('#<%=$this->JobToVerifyJobNameLine->ClientID%>');
							var verify_by_jobid = $('#<%=$this->JobToVerifyJobIdLine->ClientID%>');
							var accurate = $('#<%=$this->AccurateLine->ClientID%>');
							var estimate = $('#<%=$this->EstimateLine->ClientID%>');
							var verify_current_opt = $('#<%=$this->JobToVerifyOptions->ClientID%>').val();
							if(/^(<%=implode('|', $this->job_to_verify)%>)$/.test(this.value)) {
								accurate.hide();
								estimate.hide();
								verify_options.show();
								job_to_verify.show();
								if (verify_current_opt == 'jobid') {
									verify_by_job_name.hide();
									verify_by_jobid.show();
								} else if (verify_current_opt == 'jobname') {
									verify_by_job_name.show();
									verify_by_jobid.hide();
								}
							} else if (job_to_verify.is(':visible')) {
								job_to_verify.hide();
								verify_options.hide();
								verify_by_job_name.hide();
								verify_by_jobid.hide();
								accurate.show();
								estimate.show();
							}
						</prop:Attributes.onchange>
					</com:TActiveDropDownList>
				</div>
			</div>
			<com:TActivePanel ID="JobToVerifyOptionsLine" CssClass="line">
				<div class="text"><com:TLabel ForControl="JobToVerifyOptions" Text="<%[ Verify option: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="JobToVerifyOptions" AutoPostBack="false" CssClass="textbox-auto">
						<prop:Attributes.onchange>
							var verify_by_job_name = $('#<%=$this->JobToVerifyJobNameLine->ClientID%>');
							var verify_by_jobid = $('#<%=$this->JobToVerifyJobIdLine->ClientID%>');
							if (this.value == 'jobname') {
								verify_by_jobid.hide();
								verify_by_job_name.show();
							} else if (this.value == 'jobid') {
								verify_by_job_name.hide();
								verify_by_jobid.show();
							} else {
								verify_by_job_name.hide();
								verify_by_jobid.hide();
							}
						</prop:Attributes.onchange>
					</com:TActiveDropDownList>
				</div>
			</com:TActivePanel>
			<com:TActivePanel ID="JobToVerifyJobNameLine" CssClass="line">
				<div class="text"><com:TLabel ForControl="JobToVerifyJobName" Text="<%[ Job to Verify: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="JobToVerifyJobName" AutoPostBack="false" CssClass="textbox-auto" />
				</div>
			</com:TActivePanel>
			<com:TActivePanel ID="JobToVerifyJobIdLine" CssClass="line">
				<div class="text"><com:TLabel ForControl="JobToVerifyJobId" Text="<%[ JobId to Verify: ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="JobToVerifyJobId" CssClass="textbox-auto" AutoPostBack="false" />
					<com:TActiveCustomValidator ID="JobToVerifyJobIdValidator" ValidationGroup="JobRunGroup" ControlToValidate="JobToVerifyJobId" ErrorMessage="<%[ JobId to Verify value must be integer greather than 0. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="jobIdToVerifyValidator" />
				</div>
			</com:TActivePanel>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Client" Text="<%[ Client: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="Client" AutoPostBack="false" CssClass="textbox-auto" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="FileSet" Text="<%[ FileSet: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="FileSet" AutoPostBack="false" CssClass="textbox-auto" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Pool" Text="<%[ Pool: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="Pool" AutoPostBack="false" CssClass="textbox-auto" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Storage" Text="<%[ Storage: ]%>" /></div>
				<div class="field">
					<com:TActiveDropDownList ID="Storage" AutoPostBack="false" CssClass="textbox-auto" />
				</div>
			</div>
			<div class="line">
				<div class="text"><com:TLabel ForControl="Priority" Text="<%[ Priority: ]%>" /></div>
				<div class="field">
					<com:TActiveTextBox ID="Priority" CssClass="textbox-auto" AutoPostBack="false" />
					<com:TActiveCustomValidator ID="PriorityValidator" ValidationGroup="JobRunGroup" ControlToValidate="Priority" ErrorMessage="<%[ Priority value must be integer greather than 0. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="priorityValidator" />
				</div>
			</div>
			<com:TPanel ID="AccurateLine" CssClass="line">
				<div class="text"><com:TLabel ForControl="Accurate" Text="<%[ Accurate: ]%>" /></div>
				<div class="field"><com:TActiveCheckBox ID="Accurate" AutoPostBack="false" /></div>
			</com:TPanel>
			<div class="line">
				<div class="text"><com:TLabel ForControl="GoToStartedJob" Text="<%[ Go to started job after start: ]%>" /></div>
				<div class="field"><com:TActiveCheckBox ID="GoToStartedJob" AutoPostBack="false" Checked="true" /></div>
			</div>
			<com:TCallback ID="ReloadRunJobs" OnCallback="Page.JobRunWindow.prepareData" ClientSide.OnComplete="SlideWindow.getObj('JobRunWindow').setLoadRequest();" />
			<script type="text/javascript">
				var jobrun_callback_func = function() {
					/* If Job Run list window is not open or if actually toolbar is used
					 * then Job Run window refresh does not take place.
					 */
					var obj = SlideWindow.getObj('JobRunWindow');
					if (obj.isWindowOpen() === false || obj.isToolbarOpen() === true) {
						return;
					}
					var mainForm = Prado.Validation.getForm();
					var callback = <%=$this->ReloadRunJobs->ActiveControl->Javascript%>;
					if (Prado.Validation.managers[mainForm].getValidatorsWithError('JobRunGroup').length == 0) {
						callback.dispatch();
					}
				}
			</script>
			<div class="button">
				<com:BActiveButton ID="Run" Text="<%[ Run job ]%>" ValidationGroup="JobRunGroup" CausesValidation="true" OnClick="run_job">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('JobRunWindow').progress(false);
						oMonitor();
						jobrun_callback_func();
						if (Prado.Validation.isValid(Prado.Validation.getForm(), 'JobRunGroup') === true) {
							if (document.getElementById('<%=$this->GoToStartedJob->ClientID%>').checked === true) {
								ConfigurationWindow.getObj('JobWindow').progress(false);
								ConfigurationWindow.getObj('JobWindow').show();
								ConfigurationWindow.getObj('JobWindow').switchTabByNo(2);
							} else {
								ConfigurationWindow.getObj('JobRunWindow').switchTab('jobrun_console_tab');
							}
						}
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
				<com:TPanel ID="EstimateLine" CssClass="button_line">
					<com:BActiveButton ID="Estimate" Text="<%[ Estimate job ]%>" OnClick="estimate">
						<prop:ClientSide.OnSuccess>
							ConfigurationWindow.getObj('JobRunWindow').progress(false);
							if (Prado.Validation.isValid(Prado.Validation.getForm(), 'JobRunGroup') === true) {
								ConfigurationWindow.getObj('JobRunWindow').switchTab('jobrun_console_tab');
							}
						</prop:ClientSide.OnSuccess>
					</com:BActiveButton>
				</com:TPanel>
			</div>
		</div>
		<div id="jobrun_console_tab" style="display: none">
			<div class="field-full">
				<com:TActiveTextBox ID="Estimation" TextMode="MultiLine" CssClass="textbox-auto" Style="height: 475px;" ReadOnly="true" />
			</div>
		</div>
	</com:TActivePanel>
</com:TContent>
