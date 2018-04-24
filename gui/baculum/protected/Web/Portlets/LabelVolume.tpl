<com:TActiveLinkButton
	CssClass="w3-button w3-green"
	Attributes.onclick="document.getElementById('label_volume').style.display = 'block';"
	OnClick="loadValues"
>
	<i class="fa fa-tag"></i> &nbsp;<%[ Label volume(s) ]%>
</com:TActiveLinkButton>
</button>
<div id="label_volume" class="w3-modal">
	<div class="w3-modal-content w3-animate-top w3-card-4">
		<header class="w3-container w3-teal"> 
			<span onclick="document.getElementById('label_volume').style.display='none'" class="w3-button w3-display-topright">&times;</span>
			<h2><%[ Label volume(s) ]%></h2>
		</header>
		<div class="w3-padding">
			<com:TValidationSummary
				ID="ValidationSummary"
				CssClass="validation-error-summary"
				ValidationGroup="LabelVolumeGroup"
				AutoUpdate="true"
				Display="Dynamic"
				HeaderText="<%[ There is not possible to run selected action because: ]%>"
			/>
			<com:TRegularExpressionValidator
				ValidationGroup="LabelVolumeGroup"
				ControlToValidate="LabelName"
				ErrorMessage="<%[ Label name have to contain string value from set [0-9a-zA-Z_-]. ]%>"
				ControlCssClass="validation-error"
				Display="Dynamic"
				RegularExpression="[0-9a-zA-Z\-_]+"
			 />
			<com:TRegularExpressionValidator
				ID="SlotsLabelValidator"
				ValidationGroup="LabelVolumeGroup"
				ControlToValidate="SlotsLabel"
				ErrorMessage="<%[ Slots for label have to contain string value from set [0-9-,]. ]%>"
				ControlCssClass="validation-error"
				Display="Dynamic"
				RegularExpression="[0-9\-\,]+"
			/>
			<com:TRegularExpressionValidator
				ID="DriveLabelValidator"
				ValidationGroup="LabelVolumeGroup"
				ControlToValidate="DriveLabel"
				ErrorMessage="<%[ Drive has to contain digit value from set [0-9]. ]%>"
				ControlCssClass="validation-error"
				Display="Dynamic"
				RegularExpression="[0-9]+"
			/>
			<div class="w3-row-padding w3-section-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="Barcodes" Text="<%[ Use barcodes as label: ]%>" /></div>
				<div class="w3-col w3-half"><com:TActiveCheckBox ID="Barcodes" CssClass="w3-check" Attributes.onclick="set_barcodes();"/></div>
			</div>
			<div id="label_with_name" class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="LabelName" Text="<%[ Label name: ]%>" /></div>
				<div class="w3-col w3-half">
					<com:TActiveTextBox ID="LabelName" CssClass="w3-input w3-border" />
					<com:TRequiredFieldValidator
						ValidationGroup="LabelVolumeGroup"
						ControlToValidate="LabelName"
						ErrorMessage="<%[ Field required. ]%>"
						Display="Dynamic"
					>
						<prop:ClientSide.OnValidate>
							sender.enabled = !document.getElementById('<%=$this->Barcodes->ClientID%>').checked;
						</prop:ClientSide.OnValidate>
 					</com:TRequiredFieldValidator>
				</div>
			</div>
			<div id="label_with_barcodes" class="w3-row-padding w3-section" style="display: none">
				<div class="w3-col w3-half"><com:TLabel ForControl="SlotsLabel" Text="<%[ Slots for label: ]%>" /></div>
				<div class="w3-col w3-half">
					<com:TActiveTextBox ID="SlotsLabel" CssClass="w3-input w3-border" Text="0" />
					<com:TRequiredFieldValidator
						ValidationGroup="LabelVolumeGroup"
						ControlToValidate="SlotsLabel"
						ErrorMessage="<%[ Field required. ]%>"
						Display="Dynamic"
					>
						<prop:ClientSide.OnValidate>
							sender.enabled = document.getElementById('<%=$this->Barcodes->ClientID%>').checked;
						</prop:ClientSide.OnValidate>
 					</com:TRequiredFieldValidator>
				</div>
			</div>
			<div class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="StorageLabel" Text="<%[ Storage: ]%>" /></div>
				<div class="w3-col w3-half"><com:TActiveDropDownList ID="StorageLabel" CssClass="w3-select w3-border" /></div>
			</div>
			<div class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="DriveLabel" Text="<%[ Drive number: ]%>" /></div>
				<div class="w3-col w3-half">
					<com:TActiveTextBox ID="DriveLabel" CssClass="w3-input w3-border" Text="0" />
					<com:TRequiredFieldValidator
						ValidationGroup="LabelVolumeGroup"
						ControlToValidate="DriveLabel"
						ErrorMessage="<%[ Field required. ]%>"
						Display="Dynamic"
					/>
				</div>
			</div>
			<div class="w3-row-padding w3-section">
				<div class="w3-col w3-half"><com:TLabel ForControl="PoolLabel" Text="<%[ Pool: ]%>" /></div>
				<div class="w3-col w3-half"><com:TActiveDropDownList ID="PoolLabel" CssClass="w3-select w3-border" /></div>
			</div>
			<div id="label_volume_log" class="w3-panel w3-card w3-light-grey" style="display: none; max-height: 200px; overflow-x: auto;">
				<div class="w3-code notranslate">
					<pre><com:TActiveLabel ID="LabelVolumeLog" /></pre>
				</div>
			</div>
			<div class="w3-container w3-center w3-section">
				<button type="button" class="w3-button w3-red" onclick="document.getElementById('label_volume').style.display='none';"><i class="fa fa-times"></i> &nbsp;<%[ Close ]%></button>
				<com:TActiveLinkButton
					ID="LabelButton"
					CausesValidation="true"
					ValidationGroup="LabelVolumeGroup"
					OnClick="labelVolumes"
					CssClass="w3-button w3-green"
					ClientSide.OnLoading="$('#status_label_volume_loading').show();"
					ClientSide.OnSuccess="$('#status_label_volume_loading').hide();$('#label_volume_log').show();"
				>
					<i class="fa fa-tag"></i> &nbsp;<%[ Label ]%>
				</com:TActiveLinkButton>
				<i id="status_label_volume_loading" class="fa fa-sync w3-spin" style="display: none;"></i>
			</div>
		</div>
	</div>
</div>
<script type="text/javascript">
function set_barcodes() {
	var chkb = document.getElementById('<%=$this->Barcodes->ClientID%>');
	var name_el = document.getElementById('label_with_name');
	var barcodes_el = document.getElementById('label_with_barcodes');
	if (chkb.checked) {
		name_el.style.display = 'none';
		barcodes_el.style.display = 'block';
	} else {
		barcodes_el.style.display = 'none';
		name_el.style.display = 'block';
	}
}
</script>
