<div id="run_job" class="w3-modal">
	<div class="w3-modal-content w3-animate-top w3-card-4">
		<header class="w3-container w3-teal"> 
			<span onclick="document.getElementById('run_job').style.display='none'" class="w3-button w3-display-topright">&times;</span>
			<h2><%[ Run job ]%><%=$this->getJobName() ? ' - ' . $this->getJobName() : ''%></h2>
		</header>
		<div class="w3-container w3-margin-left w3-margin-right">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="JobGroup"
				AutoUpdate="true"
				Display="Dynamic"
				/>
			<com:TActivePanel ID="JobToRunLine" CssClass="w3-row w3-section w3-text-teal" Display="None">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-tasks"></i> &nbsp;<com:TLabel ForControl="JobToRun" Text="<%[ Job to run: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="JobToRun" AutoPostBack="false" CssClass="w3-select w3-border" />
				</div>
			</com:TActivePanel>
			<div class="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-compass"></i> &nbsp;<com:TLabel ForControl="Level" Text="<%[ Level: ]%>" CssClass="w3-xlarge"/></div>
				<div class="w3-half">
				<com:TActiveDropDownList ID="Level" CssClass="w3-select w3-border" AutoPostBack="false">
					<prop:Attributes.onchange>
						var job_to_verify = $('#<%=$this->JobToVerifyOptionsLine->ClientID%>');
						var verify_options = $('#<%=$this->JobToVerifyOptionsLine->ClientID%>');
						var verify_by_job_name = $('#<%=$this->JobToVerifyJobNameLine->ClientID%>');
						var verify_by_jobid = $('#<%=$this->JobToVerifyJobIdLine->ClientID%>');
						var accurate = $('#<%=$this->AccurateLine->ClientID%>');
						var verify_current_opt = document.getElementById('<%=$this->JobToVerifyOptions->ClientID%>').value;
						if(/^(<%=implode('|', $this->job_to_verify)%>)$/.test(this.value)) {
							accurate.hide();
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
						}
					</prop:Attributes.onchange>
				</com:TActiveDropDownList>
				</div>
			</div>
			<com:TActivePanel ID="JobToVerifyOptionsLine" CssClass="w3-row w3-section w3-text-teal" Display="None">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-search-plus"></i> &nbsp;<com:TLabel ForControl="JobToVerifyOptions" Text="<%[ Verify option: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="JobToVerifyOptions" AutoPostBack="false" CssClass="w3-select w3-border">
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
			<com:TActivePanel ID="JobToVerifyJobNameLine" CssClass="w3-row w3-section w3-text-teal" Display="None">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-check"></i> &nbsp;<com:TLabel ForControl="JobToVerifyJobName" Text="<%[ Job to Verify: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="JobToVerifyJobName" AutoPostBack="false" CssClass="w3-select w3-border" />
				</div>
			</com:TActivePanel>
			<com:TActivePanel ID="JobToVerifyJobIdLine" CssClass="w3-row w3-section w3-text-teal" Display="None">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-file-alt"></i> &nbsp;<com:TLabel ForControl="JobToVerifyJobId" Text="<%[ JobId to Verify: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveTextBox ID="JobToVerifyJobId" CssClass="w3-input w3-border" AutoPostBack="false" />
					<com:TDataTypeValidator ID="JobToVerifyJobIdValidator" ValidationGroup="JobGroup" ControlToValidate="JobToVerifyJobId" ErrorMessage="<%[ JobId to Verify value must be integer greather than 0. ]%>" ControlCssClass="validation-error" Display="None" DataType="Integer" />
				</div>
			</com:TActivePanel>
			<div class="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-desktop"></i> &nbsp;<com:TLabel ForControl="Client" Text="<%[ Client: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="Client" AutoPostBack="false" CssClass="w3-select w3-border" />
				</div>
			</div>
			<div class="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-list-alt"></i> &nbsp;<com:TLabel ForControl="FileSet" Text="<%[ FileSet: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="FileSet" AutoPostBack="false" CssClass="w3-select w3-border" />
				</div>
			</div>
			<div class="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-tape"></i> &nbsp;<com:TLabel ForControl="Pool" Text="<%[ Pool: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="Pool" AutoPostBack="false" CssClass="w3-select w3-border" />
				</div>
			</div>
			<div class="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-database"></i> &nbsp;<com:TLabel ForControl="Storage" Text="<%[ Storage: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveDropDownList ID="Storage" AutoPostBack="false" CssClass="w3-select w3-border" />
				</div>
			</div>
			<div class="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-sort-numeric-up"></i> &nbsp;<com:TLabel ForControl="Priority" Text="<%[ Priority: ]%>" CssClass="w3-xlarge" /></div>
				<div class="w3-half">
					<com:TActiveTextBox ID="Priority" CssClass="w3-input w3-border" AutoPostBack="false" />
					<com:TDataTypeValidator ID="PriorityValidator" ValidationGroup="JobGroup" ControlToValidate="Priority" ErrorMessage="<%[ Priority value must be integer greather than 0. ]%>" ControlCssClass="validation-error" Display="None" DataType="Integer" />
				</div>
			</div>
			<com:TActivePanel ID="AccurateLine" CssClass="w3-row w3-section w3-text-teal">
				<div class="w3-col w3-quarter"><i class="w3-xxlarge fa fa-balance-scale"></i> &nbsp;<com:TLabel ForControl="Accurate" Text="<%[ Accurate: ]%>" CssClass="w3-xlarge"/></div>
				<div class="field"><com:TActiveCheckBox ID="Accurate" AutoPostBack="false" CssClass="w3-check" /></div>
			</com:TActivePanel>
		</div>
		<div id="run_job_log" class="w3-panel w3-card w3-light-grey" style="display: none; max-height: 200px; overflow-x: auto;">
			<div class="w3-code notranslate">
				<pre><com:TActiveLabel ID="EstimationLog" /></pre>
			</div>
		</div>
		<footer class="w3-container w3-center">
			<i id="estimate_status" class="fa fa-sync w3-spin" style="display: none"></i> &nbsp;
			<com:TActiveButton
				ID="Estimate"
				Text="<%[ Estimate job ]%>"
				OnClick="estimate"
				CssClass="w3-button w3-section w3-teal w3-padding"
			>
				<prop:ClientSide.OnLoading>
					document.getElementById('estimate_status').style.display = '';
				</prop:ClientSide.OnLoading>
				<prop:ClientSide.OnComplete>
					document.getElementById('estimate_status').style.display = 'none';
					document.getElementById('run_job_log').style.display = '';
				</prop:ClientSide.OnComplete>
			</com:TActiveButton>
			<com:TButton
				ID="Run"
				Text="<%[ Run job ]%>"
				ValidationGroup="JobGroup"
				CausesValidation="true"
				OnClick="run_again"
				CssClass="w3-button w3-section w3-teal w3-padding"
			>
				<prop:Attributes.onclick>
					var mainForm = Prado.Validation.getForm();
					var send = false;
					if (Prado.Validation.managers[mainForm].getValidatorsWithError('JobGroup').length == 0) {
						send = true;
					}
					return send;
				</prop:Attributes.onclick>
			</com:TButton>
		</footer>
	</div>
</div>
