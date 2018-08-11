<div class="center">
	<com:TActiveLabel ID="NewAuthClientAddOk" Display="None" CssClass="validate" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="Validate" /> <strong><%[ User added successfully. ]%></strong></com:TActiveLabel>
	<com:TActiveLabel ID="NewAuthClientAddError" Display="None" CssClass="validator" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="" /> <strong><%[ Problem during save to config file. Please check users config file permission. ]%></strong></com:TActiveLabel>
	<com:TActiveLabel ID="NewAuthClientAddExists" Display="None" CssClass="validator" EnableViewState="false"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="" /> <strong><%[ Given user already exists in config file. ]%></strong></com:TActiveLabel>
</div>
<com:TPanel ID="AuthPanel" DefaultButton="NewAuthClient">
<div id="<%=$this->ClientID%>new_auth_client" style="display: none">
	<div style="display: <%=($this->getAuthType() == 'basic' ? '' : 'none')%>">
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
					ValidationGroup="<%=$this->ClientID%>Basic"
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
					ValidationGroup="<%=$this->ClientID%>Basic"
					Text="<%[ Please enter API password. ]%>"
				/>
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="RetypeAPIBasicPassword" Text="<%[ Retype password: ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="RetypeAPIBasicPassword"
					CssClass="textbox"
					TextMode="Password"
					MaxLength="30"
					PersistPassword="true"
				/>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="RetypeAPIBasicPassword"
					ValidationGroup="<%=$this->ClientID%>Basic"
					Text="<%[ Please enter retype password. ]%>"
				/>
				<com:TRegularExpressionValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="RetypeAPIBasicPassword"
					RegularExpression="[\S\s]{5,30}"
					ValidationGroup="<%=$this->ClientID%>Basic"
					Text="<%[ Password must be longer than 4 chars. ]%>"
				/>
				<com:TCompareValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="RetypeAPIBasicPassword"
					ControlToCompare="APIBasicPassword"
					ValidationGroup="<%=$this->ClientID%>Basic"
					Text="<%[ Passwords must be the same. ]%>"
				/>
			</div>
		</div>
	</div>
	<div style="display: <%=($this->getAuthType() == 'oauth2' ? '' : 'none')%>">
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
					ValidationGroup="<%=$this->ClientID%>OAuth2"
					Text="<%[ Please enter Client ID. ]%>"
				/>
				<com:TRegularExpressionValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2ClientId"
					RegularExpression="<%=OAuth2::CLIENT_ID_PATTERN%>"
					ValidationGroup="<%=$this->ClientID%>OAuth2"
					Text="<%[ Invalid Client ID value. Client ID may contain a-z A-Z 0-9 - _ characters. ]%>"
					/>
				<a href="javascript:void(0)" onclick="document.getElementById('<%=$this->APIOAuth2ClientId->ClientID%>').value = get_random_string('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_', 32); return false;"><%[ generate ]%></a>
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
					ValidationGroup="<%=$this->ClientID%>OAuth2"
					Text="<%[ Please enter Client Secret. ]%>"
				/>
				<com:TRegularExpressionValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2ClientSecret"
					RegularExpression="<%=OAuth2::CLIENT_SECRET_PATTERN%>"
					ValidationGroup="<%=$this->ClientID%>OAuth2"
					Text="<%[ Invalid Client Secret value. Client Secret may contain any character that is not a whitespace character. ]%>"
				/>
				<a href="javascript:void(0)" onclick="document.getElementById('<%=$this->APIOAuth2ClientSecret->ClientID%>').value = get_random_string('ABCDEFabcdef0123456789', 40); return false;"><%[ generate ]%></a>
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
					ValidationGroup="<%=$this->ClientID%>OAuth2"
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
				<a href="javascript:void(0)" onclick="set_scopes('<%=$this->APIOAuth2Scope->ClientID%>'); return false;" style="vertical-align: top"><%[ set all scopes ]%></a>
				<com:TRequiredFieldValidator
					CssClass="validator-block"
					Display="Dynamic"
					ControlCssClass="invalidate"
					ControlToValidate="APIOAuth2Scope"
					ValidationGroup="<%=$this->ClientID%>OAuth2"
					Text="<%[ Please enter OAuth2 scopes. ]%>"
				/>
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIOAuth2BconsoleCfgPath" Text="<%[ Dedicated Bconsole config file path: ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2BconsoleCfgPath"
					CssClass="textbox"
					CausesValidation="false"
				/> <%[ (optional) ]%>
			</div>
		</div>
		<div class="line">
			<div class="text"><com:TLabel ForControl="APIOAuth2Name" Text="<%[ Short name: ]%>" /></div>
			<div class="field">
				<com:TTextBox
					ID="APIOAuth2Name"
					CssClass="textbox"
					CausesValidation="false"
				/> <%[ (optional) ]%>
			</div>
		</div>
	</div>
	<div class="center" style="width: 550px;<%=($this->getShowButtons() ? '' : 'display: none;')%>">
		<com:BButton
			Text="<%[ Cancel ]%>"
			CausesValidation="false"
			Attributes.onclick="$('#<%=$this->ClientID%>new_auth_client').slideUp(); return false;"
		/>
		<com:BActiveButton
			ID="NewAuthClient"
			ValidationGroup="<%=$this->ClientID%>NewAuthClientGroup"
			OnCommand="TemplateControl.addNewAuthClient"
			Text="<%[ Add ]%>"
			Attributes.onclick="return <%=$this->ClientID%>fields_validation()"
		>
			<prop:ClientSide.OnPreDispatch>
				$('#<%=$this->NewAuthClientAddOk->ClientID%>').hide();
				$('#<%=$this->NewAuthClientAddError->ClientID%>').hide();
				$('#<%=$this->NewAuthClientAddExists->ClientID%>').hide();
			</prop:ClientSide.OnPreDispatch>
			<prop:ClientSide.OnComplete>
				var msg_ok = $('#<%=$this->NewAuthClientAddOk->ClientID%>'); 
				if (msg_ok.is(':visible')) {
					$('#<%=$this->ClientID%>new_auth_client').slideUp();
					setTimeout(function() {
						$('#<%=$this->NewAuthClientAddOk->ClientID%>').slideUp();
					}, 5000);
				}
			</prop:ClientSide.OnComplete>
		</com:BActiveButton>
	</div>
</div>
</com:TPanel>
<script type="text/javascript">
	var <%=$this->ClientID%>fields_validation = function() {
		var basic = <%=($this->getAuthType() === 'basic' ? 1 : 0)%>;
		var oauth2 = <%=($this->getAuthType() === 'oauth2' ? 1 : 0)%>;
		var validation_group;
		if (basic) {
			validation_group = '<%=$this->ClientID%>Basic';
		} else if (oauth2) {
			validation_group = '<%=$this->ClientID%>OAuth2';
		}
		return Prado.Validation.validate(Prado.Validation.getForm(), validation_group);
	}
</script>
