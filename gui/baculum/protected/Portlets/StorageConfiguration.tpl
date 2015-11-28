<%@ MasterClass="Application.Portlets.ConfigurationPanel"%>
<com:TContent ID="ConfigurationWindowContent">
		<h4><%[ Storage name: ]%> <com:TActiveLabel ID="StorageName" /><com:TActiveLabel ID="StorageID" Visible="false" /></h4>
		<span class="text tab tab_active" rel="storage_actions_tab"><%[ Actions ]%></span>
		<span class="text tab" rel="storage_console_tab"><%[ Console status ]%></span>
		<hr class="tabs" />
		<div id="storage_actions_tab">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="AutoChangerGroup"
				AutoUpdate="true"
				Display="Dynamic"
				HeaderText="<%[ There is not possible to run selected action because: ]%>" />
			<com:TActivePanel ID="AutoChanger" Visible="false" Style="margin-bottom: 10px">
				<div class="line">
					<div class="text"><com:TLabel ForControl="Drive" Text="<%[ Drive number: ]%>" /></div>
					<div class="field">
						<com:TActiveTextBox ID="Drive" AutoPostBack="false" Text="0" MaxLength="3" CssClass="textbox-short" />
						<com:TActiveCustomValidator ID="DriveValidator" ValidationGroup="AutoChangerGroup" ControlToValidate="Drive" ErrorMessage="<%[ Drive number must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="driveValidator" />
					</div>
				</div>
				<div class="line">
					<div class="text"><com:TLabel ForControl="Slot" Text="<%[ Slot number: ]%>" /></div>
					<div class="field">
						<com:TActiveTextBox ID="Slot" AutoPostBack="false" Text="0" MaxLength="3" CssClass="textbox-short" />
						<com:TActiveCustomValidator ID="SlotValidator" ValidationGroup="AutoChangerGroup" ControlToValidate="Slot" ErrorMessage="<%[ Slot number must be integer. ]%>" ControlCssClass="validation-error" Display="None" OnServerValidate="slotValidator" />
					</div>
				</div>
			</com:TActivePanel>
			<div class="button-center">
				<com:BActiveButton ID="Mount" OnClick="mount" ValidationGroup="AutoChangerGroup" CausesValidation="true" Text="<%[ Mount ]%>">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('StorageWindow').progress(false);
						if (Prado.Validation.isValid(Prado.Validation.getForm(), 'AutoChangerGroup') === true) {
							ConfigurationWindow.getObj('StorageWindow').switchTab('storage_console_tab');
						}
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
				<com:BActiveButton ID="Release" OnClick="release" Text="<%[ Release ]%>" ValidationGroup="AutoChangerGroup" CausesValidation="true">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('StorageWindow').progress(false);
						if (Prado.Validation.isValid(Prado.Validation.getForm(), 'AutoChangerGroup') === true) {
							ConfigurationWindow.getObj('StorageWindow').switchTab('storage_console_tab');
						}
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
				<com:BActiveButton ID="Umount" OnClick="umount" ValidationGroup="AutoChangerGroup" CausesValidation="true" Text="<%[ Umount ]%>">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('StorageWindow').progress(false);
						if (Prado.Validation.isValid(Prado.Validation.getForm(), 'AutoChangerGroup') === true) {
							ConfigurationWindow.getObj('StorageWindow').switchTab('storage_console_tab');
						}
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
				<com:BActiveButton ID="Status" OnClick="status" Text="<%[ Status ]%>">
					<prop:ClientSide.OnSuccess>
						ConfigurationWindow.getObj('StorageWindow').progress(false);
						ConfigurationWindow.getObj('StorageWindow').switchTab('storage_console_tab');
					</prop:ClientSide.OnSuccess>
				</com:BActiveButton>
			</div>
		</div>
		<div id="storage_console_tab" style="display: none">
			<div class="field-full">
				<com:TActiveTextBox ID="ShowStorage" TextMode="MultiLine" CssClass="textbox-auto" Style="height: 475px" ReadOnly="true" />
			</div>
		</div>
</com:TContent>
