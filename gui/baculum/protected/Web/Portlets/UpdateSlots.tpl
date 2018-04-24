<com:TActiveLinkButton
	CssClass="w3-button w3-green"
	Attributes.onclick="document.getElementById('update_slots').style.display = 'block';"
	OnClick="loadValues"
>
	<i class="fa fa-retweet"></i> &nbsp;<%[ Update slots ]%>
</com:TActiveLinkButton>
</button>
<div id="update_slots" class="w3-modal">
	<div class="w3-modal-content w3-animate-top w3-card-4">
		<header class="w3-container w3-teal"> 
			<span onclick="document.getElementById('update_slots').style.display='none'" class="w3-button w3-display-topright">&times;</span>
			<h2><%[ Update slots ]%></h2>
		</header>
		<div class="w3-padding">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="UpdateSlotsGroup"
				AutoUpdate="true"
				Display="Dynamic"
				HeaderText="<%[ There is not possible to run selected action because: ]%>"
			/>
			<com:TRegularExpressionValidator
				ID="SlotsUpdateValidator"
				ValidationGroup="UpdateSlotsGroup"
				ControlToValidate="SlotsUpdate"
				ErrorMessage="<%[ Slots for update have to contain string value from set [0-9-,]. ]%>"
				ControlCssClass="validation-error"
				Display="Dynamic"
				RegularExpression="[0-9\-\,]+"
			/>
			<com:TRegularExpressionValidator
				ID="DriveUpdateValidator"
				ValidationGroup="UpdateSlotsGroup"
				ControlToValidate="DriveUpdate"
				ErrorMessage="<%[ Drive has to contain digit value from set [0-9]. ]%>"
				ControlCssClass="validation-error"
				Display="Dynamic"
				RegularExpression="[0-9]+"
			/>
			<div class="w3-row-padding w3-section-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="Barcodes" Text="<%[ Update slots using barcodes ]%>" /></div>
				<div class="w3-col w3-half"><com:TActiveCheckBox ID="Barcodes" CssClass="w3-check" Checked="true" /></div>
			</div>
			<div class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="StorageUpdate" Text="<%[ Storage: ]%>" /></div>
				<div class="w3-col w3-half"><com:TActiveDropDownList ID="StorageUpdate" CssClass="w3-select w3-border" /></div>
			</div>
			<div class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="SlotsUpdate" Text="<%[ Slots for update: ]%>" /></div>
				<div class="w3-col w3-half">
					<com:TActiveTextBox ID="SlotsUpdate" CssClass="w3-input w3-border" Text="0" />
					<com:TRequiredFieldValidator
						ValidationGroup="UpdateSlotsGroup"
						ControlToValidate="SlotsUpdate"
						ErrorMessage="<%[ Field required. ]%>"
						Display="Dynamic"
					/>
				</div>
			</div>
			<div class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="DriveUpdate" Text="<%[ Drive number: ]%>" /></div>
				<div class="w3-col w3-half">
					<com:TActiveTextBox ID="DriveUpdate" CssClass="w3-input w3-border" Text="0" />
					<com:TRequiredFieldValidator
						ValidationGroup="UpdateSlotsGroup"
						ControlToValidate="DriveUpdate"
						ErrorMessage="<%[ Field required. ]%>"
					/>
				</div>
			</div>
			<div id="update_slots_log" class="w3-panel w3-card w3-light-grey" style="display: none; max-height: 200px; overflow-x: auto;">
				<div class="w3-code notranslate">
					<pre><com:TActiveLabel ID="UpdateSlotsLog" /></pre>
				</div>
			</div>
			<div class="w3-container w3-center w3-section">
				<button type="button" class="w3-button w3-red" onclick="document.getElementById('update_slots').style.display='none';"><i class="fa fa-times"></i> &nbsp;<%[ Close ]%></button>
				<com:TActiveLinkButton
					ID="UpdateButton"
					CausesValidation="true"
					ValidationGroup="UpdateSlotsGroup"
					OnClick="update"
					CssClass="w3-button w3-green"
					ClientSide.OnLoading="$('#status_update_slots_loading').show();"
					ClientSide.OnSuccess="$('#status_update_slots_loading').hide();$('#update_slots_log').show();"
				>
					<i class="fa fa-retweet"></i> &nbsp;<%[ Update slots ]%>
				</com:TActiveLinkButton>
				<i id="status_update_slots_loading" class="fa fa-sync w3-spin" style="display: none;"></i>
			</div>
		</div>
	</div>
</div>
