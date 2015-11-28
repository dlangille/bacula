<%@ MasterClass="Application.Portlets.ConfigurationPanel"%>
<com:TContent ID="ConfigurationWindowContent">
	<com:TActivePanel DefaultButton="Apply">
		<h4><%[ Client name: ]%> <com:TActiveLabel ID="ClientName" /><com:TActiveLabel ID="ClientIdentifier" Visible="false" /></h4>
		<com:TActiveLabel ID="ClientDescription" CssClass="description" />
		<span class="text tab tab_active" rel="client_actions_tab"><%[ Actions ]%></span>
		<span class="text tab" rel="client_console_tab"><%[ Console status ]%></span>
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
	</com:TActivePanel>
</com:TContent>
