<a class="big" href="javascript:void(0)" id="add_user_btn"><img src="/themes/Baculum-v1/add.png" alt="Add"><%[ Add new user ]%></a>
<div id="add_user" style="display: none">
	<p><%[ Username: ]%><input id="newuser" type="text" /><%[ Password: ]%><input id="newpwd" type="password" />
	<a href="javascript:void(0)" onclick="Users.addUser()">
		<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="<%[ Save ]%>" title="<%[ Save ]%>"/>
	</a>
	<a href="javascript:void(0)" onclick="Users.cancelAddUser()">
		<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="<%[ Close ]%>" title="<%[ Close ]%>" />
	</a></p>
</div>
<com:TActiveRepeater ID="UsersList">
	<prop:HeaderTemplate>
	<table id="users_list" class="window-section-detail-smallrow">
		<tr>
			<th><%[ User name ]%></th>
			<th><%[ Role ]%></th>
			<th><%[ API host ]%></th>
			<th><%[ Actions ]%></th>
		</tr>
	</prop:HeaderTemplate>
	<prop:ItemTemplate>
		<tr class="slide-window-element">
			<td><%=$this->DataItem['user']%></td>
			<td><%=$this->DataItem['admin'] ? Prado::localize('Administrator') :  Prado::localize('Normal user')%></td>
			<td>
				<com:TPanel Visible="<%=$this->DataItem['admin']%>" Style="line-height: 29px">
					Main
				</com:TPanel>
				<com:TPanel Visible="<%=!$this->DataItem['admin']%>">
						<select rel="user_host" onchange="Users.set_host('<%=$this->DataItem['user']%>', this);">
							<com:TRepeater OnInit="SourceTemplateControl.initHosts">
								<prop:HeaderTemplate>
							<option value=""><%[ Select host ]%></option>
								</prop:HeaderTemplate>
								<prop:ItemTemplate>
							<option value="<%=$this->DataItem%>" <%=(array_key_exists('users', $this->SourceTemplateControl->web_config) && array_key_exists($this->Parent->Parent->Parent->DataItem['user'], $this->SourceTemplateControl->web_config['users']) && $this->SourceTemplateControl->web_config['users'][$this->Parent->Parent->Parent->DataItem['user']] === $this->DataItem) ? 'selected' : ''%>><%=$this->DataItem%></option>
								</prop:ItemTemplate>
							</com:TRepeater>
						</select>
						<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/ajax-loader-arrows.gif" rel="user_host_img" alt="" style="visibility: hidden" />
				</com:TPanel>
			</td>
			<td>
				<a href="javascript:void(0)" <%=$this->DataItem['admin'] ? 'style="visibility: hidden"' : ''%> onclick="Users.rmUser('<%=$this->DataItem['user']%>')"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/user-del.png"> <%[ Remove user ]%></a>
				<a href="javascript:void(0)" onclick="Users.showChangePwd(this)" rel="chpwd_btn">
					<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/key.png" alt="" />
					<%[ Change password ]%>
				</a>
				<span style="display: none;" rel="chpwd">
					<input type="password" onkeydown="event.keyCode == 13 ? Users.changePwd(this, '<%=$this->DataItem['user']%>') : (event.keyCode == 27 ? Users.cancelChangePwd(this.nextElementSibling.nextElementSibling) : '');" />
					<a href="javascript:void(0)" onclick="Users.changePwd(this.prevousElementSibling, '<%=$this->DataItem['user']%>')">
						<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="<%[ Save ]%>" title="<%[ Save ]%>"/>
					</a>
					<a href="javascript:void(0)" onclick="Users.cancelChangePwd(this)">
						<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="<%[ Close ]%>" title="<%[ Close ]%>" />
					</a>
				</span>
			</td>
		</tr>
	</prop:ItemTemplate>
	<prop:FooterTemplate>
		</table>
	</prop:FooterTemplate>
</com:TActiveRepeater>
<p><em><%[ Please note that for each user (excluding administrator) there should exist separate Bconsole config file in form: ]%> <strong><com:TLabel ID="BconsoleCustomPath" /></strong></em></p>
<com:TCallback ID="UserAction" OnCallback="TemplateControl.userAction" ClientSide.OnComplete="Users.hide_loader();" />
<script type="text/javascript">
	var send_user_action = function(action, param, value) {
		Users.current_action = action;
		if (!value) {
			value = '';
		}
		var user_action_callback = <%=$this->UserAction->ActiveControl->Javascript%>;
		user_action_callback.setCallbackParameter([action, param, value].join(';'));
		user_action_callback.dispatch();
	};
	Users.txt = {
		enter_login: '<%[ Please enter login. ]%>',
		invalid_login: '<%[ Invalid login value. Login may contain a-z A-Z 0-9 characters. ]%>',
		invalid_pwd: '<%[ Password must be longer than 4 chars. ]%>'
	};
	Users.action_callback = send_user_action;
	Users.validators = { user_pattern: new RegExp('^<%=BasicUserConfig::USER_PATTERN%>$') };
	Users.init();
</script>
