<div id="new_host_status" class="center">
	<com:TActiveLabel ID="NewHostAddOk" Display="None" CssClass="validate" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="Validate" /> <strong><%[ Host added successfully. ]%></strong></com:TActiveLabel>
	<com:TActiveLabel ID="NewHostAddError" Display="None" CssClass="validator" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="" /> <strong><%[ Problem during save to config file. Please check host config file permission. ]%></strong></com:TActiveLabel>
	<com:TActiveLabel ID="NewHostAddExists" Display="None" CssClass="validator" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="" /> <strong><%[ Given host already exists in config file. ]%></strong></com:TActiveLabel>
</div>
<div id="new_host" style="display: none">
	<div class="line">
		<div class="text"><com:TLabel ForControl="APIProtocol" Text="<%[ Protocol: ]%>" /></div>
		<div class="field">
			<com:TActiveDropDownList ID="APIProtocol" CssClass="textbox" Width="150px" CausesValidation="false">
				<com:TListItem Value="http" Text="HTTP" />
				<com:TListItem Value="https" Text="HTTPS" Selected="true"/>
			</com:TActiveDropDownList>
		</div>
	</div>
	<div class="line">
		<div class="text"><com:TLabel ForControl="APIAddress" Text="<%[ IP Address/Hostname: ]%>" /></div>
		<div class="field">
			<com:TActiveTextBox ID="APIAddress" CssClass="textbox" CausesValidation="false" />
			<com:TRequiredFieldValidator ValidationGroup="NewHostGroup" CssClass="validator-block" Display="Dynamic" ControlCssClass="invalidate" ControlToValidate="APIAddress" Text="<%[ Please enter API address. ]%>" />
		</div>
	</div>
	<div class="line">
		<div class="text"><com:TLabel ForControl="APIPort" Text="<%[ Port: ]%>" /></div>
		<div class="field">
			<com:TActiveTextBox ID="APIPort" CssClass="textbox" CausesValidation="false" Text="9096" Width="70px" />
			<com:TRequiredFieldValidator ValidationGroup="NewHostGroup" CssClass="validator-block" Display="Dynamic" ControlCssClass="invalidate" ControlToValidate="APIPort" Text="<%[ Please enter API port. ]%>" />
		</div>
	</div>
	<div class="auth_setting">
		<div class="line left">
			<com:TRadioButton
				ID="AuthOAuth2"
				GroupName="SelectAuth"
				Attributes.onclick="$('#configure_basic_auth').hide();$('#configure_oauth2_auth').show();"
			/>
			<com:TLabel
				ForControl="AuthOAuth2"
				CssClass="normal"
				Text="<%[ Use OAuth2 for authorization and authentication ]%>"
			/>
		</div>
		<div class="line left">
			<com:TRadioButton
				ID="AuthBasic"
				GroupName="SelectAuth"
				Checked="true"
				Attributes.onclick="$('#configure_oauth2_auth').hide();$('#configure_basic_auth').show();"
			/>
			<com:TLabel
				ForControl="AuthBasic"
				CssClass="normal"
				Text="<%[ Use HTTP Basic authentication ]%>"
			/>
		</div>
	</div>
	<div id="configure_basic_auth" style="display: <%=($this->AuthBasic->Checked === true) ? '' : 'none';%>">
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIBasicLogin" Text="<%[ API Login: ]%>" /></div>
			<div class="field">
				<com:TActiveTextBox
					ID="APIBasicLogin"
					CssClass="textbox"
					CausesValidation="false"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIBasicLogin"
					ValidationGroup="Basic"
					Text="<%[ Please enter API login. ]%>"
				 />
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIBasicPassword" Text="<%[ API Password: ]%>" /></div>
			<div class="field">
				<com:TActiveTextBox
					ID="APIBasicPassword"
					TextMode="Password"
					CssClass="textbox"
					CausesValidation="false"
					PersistPassword="true"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIBasicPassword"
					ValidationGroup="Basic"
					Text="<%[ Please enter API password. ]%>"
				/>
			</div>
		</div>
	</div>
	<div id="configure_oauth2_auth" style="display: <%=($this->AuthOAuth2->Checked === true) ? '' : 'none';%>">
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIOAuth2ClientId" Text="<%[ OAuth2 Client ID: ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2ClientId"
					CssClass="textbox"
					CausesValidation="false"
					MaxLength="32"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2ClientId"
					ValidationGroup="OAuth2"
					Text="<%[ Please enter Client ID. ]%>"
				/>
				<com:TRegularExpressionValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2ClientId"
					RegularExpression="<%=OAuth2::CLIENT_ID_PATTERN%>"
					ValidationGroup="OAuth2"
					Text="<%[ Invalid Client ID value. Client ID may contain a-z A-Z 0-9 - _ characters. ]%>"
					/>
				<a style="display: <%=$this->getClientMode() ? 'none': 'inline'%>" href="javascript:void(0)" onclick="document.getElementById('<%=$this->APIOAuth2ClientId->ClientID%>').value = get_random_string('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_', 32); return false;"><%[ generate ]%></a>
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIOAuth2ClientSecret" Text="<%[ OAuth2 Client Secret: ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2ClientSecret"
					CssClass="textbox"
					CausesValidation="false"
					MaxLength="40"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2ClientSecret"
					ValidationGroup="OAuth2"
					Text="<%[ Please enter Client Secret. ]%>"
				/>
				<com:TRegularExpressionValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2ClientSecret"
					RegularExpression="<%=OAuth2::CLIENT_SECRET_PATTERN%>"
					ValidationGroup="OAuth2"
					Text="<%[ Invalid Client Secret value. Client Secret may contain any character that is not a whitespace character. ]%>"
				/>
				<a style="display: <%=$this->getClientMode() ? 'none': 'inline'%>" href="javascript:void(0)" onclick="document.getElementById('<%=$this->APIOAuth2ClientSecret->ClientID%>').value = get_random_string('ABCDEFabcdef0123456789', 40); return false;"><%[ generate ]%></a>
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIOAuth2RedirectURI" Text="<%[ OAuth2 Redirect URI (example: https://baculumgui:9095/web/redirect): ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2RedirectURI"
					CssClass="textbox"
					CausesValidation="false"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2RedirectURI"
					ValidationGroup="OAuth2"
					Text="<%[ Please enter Redirect URI. ]%>"
				/>
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIOAuth2Scope" Text="<%[ OAuth2 scopes (space separated): ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2Scope"
					CssClass="textbox"
					CausesValidation="false"
					TextMode="MultiLine"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2Scope"
					ValidationGroup="OAuth2"
					Text="<%[ Please enter OAuth2 scopes. ]%>"
				/>
			</div>
		</div>
		<div class="line" style="display: <%=$this->getClientMode() ? 'none': 'block'%>">
			<div class="text"><com:TLabel ForControl="APIOAuth2BconsoleCfgPath" Text="<%[ Dedicated Bconsole config file path: ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2BconsoleCfgPath"
					CssClass="textbox"
					CausesValidation="false"
				/> <%[ (optional) ]%>
			</div>
		</div>
	</div>
	<div class="line">
		<div class="text"><com:TLabel ForControl="APIConnectionTest" Text="<%[ API connection test: ]%>" /></div>
		<div class="field">
			<table border="0" cellpadding="1px" id="new_host_status">
				<tr>
					<td align="center" valign="middle">
						<com:TActiveButton ID="APIConnectionTest" Text="<%[ test ]%>" CausesValidation="true" OnCallback="connectionAPITest">
							<prop:ClientSide.OnLoading>
								$('#<%=$this->APITestResultOk->ClientID%>').hide();
								$('#<%=$this->APITestResultErr->ClientID%>').hide();
								$('#<%=$this->APICatalogSupportYes->ClientID%>').hide();
								$('#<%=$this->APICatalogSupportNo->ClientID%>').hide();
								$('#<%=$this->APIConsoleSupportYes->ClientID%>').hide();
								$('#<%=$this->APIConsoleSupportNo->ClientID%>').hide();
								$('#<%=$this->APIConfigSupportYes->ClientID%>').hide();
								$('#<%=$this->APIConfigSupportNo->ClientID%>').hide();
								$('#<%=$this->APITestLoader->ClientID%>').show();
							</prop:ClientSide.OnLoading>
							<prop:ClientSide.OnComplete>
								$('#<%=$this->APITestLoader->ClientID%>').hide();
							</prop:ClientSide.OnComplete>
						</com:TActiveButton>
					</td>
					<td valign="middle">
						<com:TActiveLabel ID="APITestLoader" Display="None"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/ajax-loader.gif" alt="<%[ Loading... ]%>" /></com:TActiveLabel>
						<com:TActiveLabel ID="APITestResultOk" Display="None" CssClass="validate" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="Validate" /> <%[ OK ]%></com:TActiveLabel>
						<com:TActiveLabel ID="APITestResultErr" Display="None" CssClass="validator-block" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="Invalidate" /> <%[ Connection error ]%></com:TActiveLabel>
					</td>
				</tr>
				<tr>
					<td><%[ Catalog support ]%></td>
					<td>
						<com:TActiveLabel ID="APICatalogSupportYes" Display="None" CssClass="validate" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="Validate" /> <strong><%[ Supported ]%></strong></com:TActiveLabel>
						<com:TActiveLabel ID="APICatalogSupportNo" Display="None" CssClass="validator-info" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_close.png" alt="" /> <strong><%[ Not supported ]%></strong></com:TActiveLabel>
					</td>
				</tr>
				<tr>
					<td><%[ Console support ]%></td>
					<td>
						<com:TActiveLabel ID="APIConsoleSupportYes" Display="None" CssClass="validate" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="Validate" /> <strong><%[ Supported ]%></strong></com:TActiveLabel>
						<com:TActiveLabel ID="APIConsoleSupportNo" Display="None" CssClass="validator-info" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_close.png" alt="" /> <strong><%[ Not supported ]%></strong></com:TActiveLabel>
					</td>
				</tr>
				<tr>
					<td><%[ Config support ]%></td>
					<td>
						<com:TActiveLabel ID="APIConfigSupportYes" Display="None" CssClass="validate" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="Validate" /> <strong><%[ Supported ]%></strong></com:TActiveLabel>
						<com:TActiveLabel ID="APIConfigSupportNo" Display="None" CssClass="validator-info" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_close.png" alt="" /> <strong><%[ Not supported ]%></strong></com:TActiveLabel>
					</td>
				</tr>
			</table>
		</div>
	</div>
	<div class="line" <%=($this->getForceHostName() ? 'style="display: none;"' : '')%>>
		<div class="text"><com:TLabel ForControl="APIHostName" Text="<%[ Save as: ]%>" /></div>
		<div class="field">
			<com:TActiveTextBox ID="APIHostName" CssClass="textbox" CausesValidation="false" /> <%[ (optional) ]%>
		</div>
	</div>
	<div class="center" style="width: 550px;<%=($this->getShowButtons() ? '' : 'display: none;')%>">
		<com:BButton
			Text="<%[ Cancel ]%>"
			CausesValidation="false"
			Attributes.onclick="$('#new_host').slideUp(); return false;"
		/>
		<com:BActiveButton
			ID="NewHost"
			ValidationGroup="NewHostGroup"
			OnCommand="TemplateControl.addNewHost"
			Text="<%[ Add host ]%>"
			Attributes.onclick="return fields_validation()"
		>
			<prop:ClientSide.OnPreDispatch>
				$('#<%=$this->NewHostAddOk->ClientID%>').hide();
				$('#<%=$this->NewHostAddError->ClientID%>').hide();
				$('#<%=$this->NewHostAddExists->ClientID%>').hide();
			</prop:ClientSide.OnPreDispatch>
			<prop:ClientSide.OnComplete>
				var msg_ok = $('#<%=$this->NewHostAddOk->ClientID%>'); 
				if (msg_ok.is(':visible')) {
					$('#new_host').slideUp();
					setTimeout(function() {
						$('#<%=$this->NewHostAddOk->ClientID%>').slideUp();
					}, 5000);
					$('#new_host_status').find('span').hide();
				}
			</prop:ClientSide.OnComplete>
		</com:BActiveButton>
	</div>
</div>
<script type="text/javascript">
	var fields_validation = function() {
		var basic = document.getElementById('<%=$this->AuthBasic->ClientID%>');
		var oauth2 = document.getElementById('<%=$this->AuthOAuth2->ClientID%>');
		var validation_group;
		if (basic.checked === true) {
			validation_group = 'Basic';
		} else if (oauth2.checked === true) {
			validation_group = 'OAuth2';
		}
		return Prado.Validation.validate(Prado.Validation.getForm(), validation_group);
	}
</script>
