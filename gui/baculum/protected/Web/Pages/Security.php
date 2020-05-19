<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2020 Kern Sibbald
 *
 * The main author of Baculum is Marcin Haba.
 * The original author of Bacula is Kern Sibbald, with contributions
 * from many others, a complete list can be found in the file AUTHORS.
 *
 * You may use this file and others of this release according to the
 * license defined in the LICENSE file, which includes the Affero General
 * Public License, v3.0 ("AGPLv3") and some additional permissions and
 * terms pursuant to its AGPLv3 Section 7.
 *
 * This notice must be preserved when any source code is
 * conveyed and/or propagated.
 *
 * Bacula(R) is a registered trademark of Kern Sibbald.
 */

Prado::using('System.Web.UI.ActiveControls.TActiveCheckBox');
Prado::using('System.Web.UI.ActiveControls.TActiveCustomValidator');
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActiveHiddenField');
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TActiveListBox');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('System.Web.UI.WebControls.TCheckBox');
Prado::using('System.Web.UI.WebControls.TLabel');
Prado::using('System.Web.UI.WebControls.TListItem');
Prado::using('System.Web.UI.WebControls.TRadioButton');
Prado::using('System.Web.UI.WebControls.TRegularExpressionValidator');
Prado::using('System.Web.UI.WebControls.TRequiredFieldValidator');
Prado::using('System.Web.UI.WebControls.TValidationSummary');
Prado::using('Application.Common.Class.Ldap');
Prado::using('Application.Web.Class.BaculumWebPage');

/**
 * Security page (auth methods, users, roles...).
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class Security extends BaculumWebPage {

	/**
	 * Modal window types for users and roles.
	 */
	const TYPE_ADD_WINDOW = 'add';
	const TYPE_EDIT_WINDOW = 'edit';

	/**
	 * Options for import users.
	 */
	const IMPORT_OPT_ALL_USERS = 0;
	const IMPORT_OPT_SELECTED_USERS = 1;
	const IMPORT_OPT_CRITERIA = 2;

	/**
	 * Options for import criteria.
	 */
	const IMPORT_CRIT_USERNAME = 0;
	const IMPORT_CRIT_LONG_NAME = 1;
	const IMPORT_CRIT_DESCRIPTION = 2;
	const IMPORT_CRIT_EMAIL = 3;


	/**
	 * Store web user config.
	 */
	private $user_config = [];

	/**
	 * Initialize page.
	 *
	 * @param mixed $param oninit event parameter
	 * @return none
	 */
	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsCallBack || $this->IsPostBack) {
			return;
		}
		$this->initDefAccessForm();
		$this->initAuthForm();
		$this->initUserWindow();
		$this->initRoleWindow();
		$this->setBasicAuthConfig();
	}

	/**
	 * Initialize form with default access settings.
	 *
	 * @return none
	 */
	public function initDefAccessForm() {
		$this->setRoles(
			$this->GeneralDefaultAccessRole,
			WebUserRoles::NORMAL
		);

		$this->setAPIHosts(
			$this->GeneralDefaultAccessAPIHost,
			HostConfig::MAIN_CATALOG_HOST
		);
		if (isset($this->web_config['security']['def_access'])) {
			if ($this->web_config['security']['def_access'] === WebConfig::DEF_ACCESS_NO_ACCESS) {
				$this->GeneralDefaultNoAccess->Checked = true;
			} elseif ($this->web_config['security']['def_access'] === WebConfig::DEF_ACCESS_DEFAULT_SETTINGS) {
				$this->GeneralDefaultAccess->Checked = true;
			}
			if (isset($this->web_config['security']['def_role'])) {
				$this->GeneralDefaultAccessRole->SelectedValue = $this->web_config['security']['def_role'];
			}
			if (isset($this->web_config['security']['def_api_host'])) {
				$this->GeneralDefaultAccessAPIHost->SelectedValue = $this->web_config['security']['def_api_host'];
			}
		} else {
			$this->GeneralDefaultAccess->Checked = true;
		}
	}

	/**
	 * Initialize form with authentication method settings.
	 *
	 * @return none
	 */
	public function initAuthForm() {
		if (isset($this->web_config['security']['auth_method'])) {
			if ($this->web_config['security']['auth_method'] ===  WebConfig::AUTH_METHOD_BASIC) {
				$this->BasicAuth->Checked = true;
			} elseif ($this->web_config['security']['auth_method'] ===  WebConfig::AUTH_METHOD_LDAP) {
				$this->LdapAuth->Checked = true;
			}

			// Fill LDAP auth fileds
			if (key_exists('auth_ldap', $this->web_config)) {
				$this->LdapAuthServerAddress->Text = $this->web_config['auth_ldap']['address'];
				$this->LdapAuthServerPort->Text = $this->web_config['auth_ldap']['port'];
				$this->LdapAuthServerLdaps->Checked = ($this->web_config['auth_ldap']['ldaps'] == 1);
				$this->LdapAuthServerProtocolVersion->Text = $this->web_config['auth_ldap']['protocol_ver'];
				$this->LdapAuthServerBaseDn->Text = $this->web_config['auth_ldap']['base_dn'];
				if ($this->web_config['auth_ldap']['auth_method'] === Ldap::AUTH_METHOD_ANON) {
					$this->LdapAuthMethodAnonymous->Checked = true;
				} elseif ($this->web_config['auth_ldap']['auth_method'] === Ldap::AUTH_METHOD_SIMPLE) {
					$this->LdapAuthMethodSimple->Checked = true;
				}
				$this->LdapAuthMethodSimpleUsername->Text = $this->web_config['auth_ldap']['bind_dn'];
				$this->LdapAuthMethodSimplePassword->Text = $this->web_config['auth_ldap']['bind_password'];
				$this->LdapAuthServerBaseDn->Text = $this->web_config['auth_ldap']['base_dn'];
				$this->LdapAttributesUsername->Text = $this->web_config['auth_ldap']['user_attr'];
				$this->LdapAttributesLongName->Text = $this->web_config['auth_ldap']['long_name_attr'];
				$this->LdapAttributesEmail->Text = $this->web_config['auth_ldap']['email_attr'];
				$this->LdapAttributesDescription->Text = $this->web_config['auth_ldap']['desc_attr'];
			}
			// Fill Basic auth fields
			if (key_exists('auth_basic', $this->web_config)) {
				$this->BasicAuthAllowManageUsers->Checked = ($this->web_config['auth_basic']['allow_manage_users'] == 1);
				$this->BasicAuthUserFile->Text = $this->web_config['auth_basic']['user_file'];
				$this->BasicAuthHashAlgorithm->SelectedValue = $this->web_config['auth_basic']['hash_alg'];
			}
		} else {
			// Default set to Basic auth method
			$this->BasicAuth->Checked = true;
		}
	}

	/**
	 * Initialize values in user modal window.
	 *
	 * @return none
	 */
	public function initUserWindow() {
		// set API hosts
		$this->setAPIHosts($this->UserAPIHost);

		// set roles
		$this->setRoles($this->UserRoles);
	}

	/**
	 * Set role list control.
	 *
	 * @param object $control control which contains role list
	 * @param mixed $def_val default value or null if no default value to set
	 * @return none
	 */
	private function setRoles($control, $def_val = null) {
		// set roles
		$roles = $this->getModule('user_role')->getRoles();
		$role_items = [];
		foreach ($roles as $role_name => $role) {
			$role_items[$role_name] = $role['long_name'] ?: $role_name;
		}
		$control->DataSource = $role_items;
		if ($def_val) {
			$control->SelectedValue = $def_val;
		}
		$control->dataBind();
	}

	/**
	 * Set API host list control.
	 *
	 * @param object $control control which contains API host list
	 * @param mixed $def_val default value or null if no default value to set
	 * @return none
	 */
	private function setAPIHosts($control, $def_val = null) {
		$api_hosts = array_keys($this->getModule('host_config')->getConfig());
		array_unshift($api_hosts, '');
		$control->DataSource = array_combine($api_hosts, $api_hosts);
		if ($def_val) {
			$control->SelectedValue = $def_val;
		}
		$control->dataBind();
	}

	/**
	 * Set and load user list.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter callback parameter
	 * @return none
	 */
	public function setUserList($sender, $param) {
		$config = $this->getModule('user_config')->getConfig();
		$this->getCallbackClient()->callClientFunction('oUsers.load_user_list_cb', [
			array_values($config)
		]);
		$this->user_config = $config;
	}

	/**
	 * Load data in user modal window.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param callback parameter
	 * @return none
	 */
	public function loadUserWindow($sender, $param) {
		//$this->getModule('user_config')->importBasicUsers();
		$username = $param->getCallbackParameter();
		$config = $this->getModule('user_config')->getUserConfig($username);
		if (count($config) > 0) {
			// It is done only for existing users
			$this->UserName->Text = $config['username'];
			$this->UserLongName->Text = $config['long_name'];
			$this->UserDescription->Text = $config['description'];
			$this->UserEmail->Text = $config['email'];
			$this->UserPassword->Text = '';
			$selected_indices = [];
			$roles = explode(',', $config['roles']);
			for ($i = 0; $i < $this->UserRoles->getItemCount(); $i++) {
				if (in_array($this->UserRoles->Items[$i]->Value, $roles)) {
					$selected_indices[] = $i;
				}
			}
			$this->UserRoles->setSelectedIndices($selected_indices);
			$this->UserAPIHost->SelectedValue = $config['api_hosts'];
			$this->UserIps->Text = $config['ips'];
			$this->UserEnabled->Checked = ($config['enabled'] == 1);
		}

		// It is done both for new user and for edit user
		if ($this->isManageUsersAvail()) {
			$this->getCallbackClient()->show('user_window_password');
		} else {
			$this->getCallbackClient()->hide('user_window_password');
		}
	}

	/**
	 * Save user.
	 * It works both for new users and for edited users.
	 * Saves values from modal popup.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param callback parameter
	 * @return none
	 */
	public function saveUser($sender, $param) {
		if (!$this->UserIps->IsValid) {
			// invalid IP restriction value
			return;
		}
		$user_win_type = $this->UserWindowType->Value;
		$username = $this->UserName->Text;
		$this->getCallbackClient()->hide('user_window_username_exists');
		if ($user_win_type === self::TYPE_ADD_WINDOW) {
			$config = $this->getModule('user_config')->getUserConfig($username);
			if (count($config) > 0) {
				$this->getCallbackClient()->show('user_window_username_exists');
				return;
			}
		}

		$config = [];
		$config['long_name'] = $this->UserLongName->Text;
		$config['description'] = $this->UserDescription->Text;
		$config['email'] = $this->UserEmail->Text;

		$selected_indices = $this->UserRoles->getSelectedIndices();
		$roles = [];
		foreach ($selected_indices as $indice) {
			for ($i = 0; $i < $this->UserRoles->getItemCount(); $i++) {
				if ($i === $indice) {
					$roles[] = $this->UserRoles->Items[$i]->Value;
				}
			}
		}
		$config['roles'] = implode(',', $roles);
		$config['api_hosts'] = $this->UserAPIHost->SelectedValue;
		$config['ips'] = $this->trimIps($this->UserIps->Text);
		$config['enabled'] = $this->UserEnabled->Checked ? 1 : 0;
		$result = $this->getModule('user_config')->setUserConfig($username, $config);

		// Set password if auth method supports it
		if ($result === true && !empty($this->UserPassword->Text) && $this->isManageUsersAvail()) {
			// Set Basic auth users password
			if ($this->getModule('web_config')->isAuthMethodBasic() &&
				isset($this->web_config['auth_basic']['user_file'])) {

				$opts = [];
				if (isset($this->web_config['auth_basic']['hash_alg'])) {
					$opts['hash_alg'] = $this->web_config['auth_basic']['hash_alg'];
				}

				// Setting basic users works both for adding and editing users
				$basic = $this->getModule('basic_webuser');
				$basic->setUsersConfig(
					$username,
					$this->UserPassword->Text,
					false,
					null,
					$opts
				);
			}
		}

		$this->setUserList(null, null);
		$this->setRoleList(null, null);
		$this->getCallbackClient()->callClientFunction('oUsers.save_user_cb');
	}

	/**
	 * Remove users action.
	 * Here is possible to remove one user or many.
	 * This action is linked with table bulk actions.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param callback parameter
	 * @return none
	 */
	public function removeUsers($sender, $param) {
		$usernames = explode('|', $param->getCallbackParameter());
		$config = $this->getModule('user_config')->getConfig();
		for ($i = 0; $i < count($usernames); $i++) {
			if (key_exists($usernames[$i], $config)) {
				unset($config[$usernames[$i]]);
			}
		}
		$result = $this->getModule('user_config')->setConfig($config);

		if ($result === true && $this->isManageUsersAvail() &&
			$this->getModule('web_config')->isAuthMethodBasic() &&
			isset($this->web_config['auth_basic']['user_file'])) {
			// Remove basic auth users too
			$basic = $this->getModule('basic_webuser');
			$basic->removeUsers($usernames);
		}

		// refresh user list
		$this->setUserList(null, null);

		// refresh role list
		$this->setRoleList(null, null);
	}

	/**
	 * Initialize values in role modal window.
	 *
	 * @return none
	 */
	public function initRoleWindow() {
		// set role resources
		$resources = $this->getModule('page_category')->getCategories(false);
		$this->RoleResources->DataSource = array_combine($resources, $resources);
		$this->RoleResources->dataBind();
	}

	/**
	 * Set and load role list.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter callback parameter
	 * @return none
	 */
	public function setRoleList($sender, $param) {
		$config = $this->getModule('user_role')->getRoles();
		$this->addUserStatsToRoles($config);
		$this->getCallbackClient()->callClientFunction('oRoles.load_role_list_cb', [
			array_values($config)
		]);
	}

	/**
	 * Load data in role modal window.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param callback parameter
	 * @return none
	 */
	public function loadRoleWindow($sender, $param) {
		$role = $param->getCallbackParameter();
		$config = $this->getModule('user_role')->getRole($role);
		if (count($config) > 0) {
			// Edit role window
			$this->Role->Text = $config['role'];
			$this->RoleLongName->Text = $config['long_name'];
			$this->RoleDescription->Text = $config['description'];
			$selected_indices = [];
			$resources = explode(',', $config['resources']);
			for ($i = 0; $i < $this->RoleResources->getItemCount(); $i++) {
				if (in_array($this->RoleResources->Items[$i]->Value, $resources)) {
					$selected_indices[] = $i;
				}
			}
			$this->RoleResources->setSelectedIndices($selected_indices);
			$this->RoleEnabled->Checked = ($config['enabled'] == 1);
			if ($this->getModule('user_role')->isRolePreDefined($role)) {
				$this->RoleSave->Display = 'None';
				$this->PreDefinedRoleMsg->Display = 'Dynamic';
			} else {
				$this->RoleSave->Display = 'Dynamic';
				$this->PreDefinedRoleMsg->Display = 'None';
			}
		} else {
			// New role window
			$this->RoleSave->Display = 'Dynamic';
			$this->PreDefinedRoleMsg->Display = 'None';
		}
	}

	/**
	 * Save role.
	 * It works both for new roles and for edited roles.
	 * Saves values from modal popup.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param callback parameter
	 * @return none
	 */
	public function saveRole($sender, $param) {
		$role_win_type = $this->RoleWindowType->Value;
		$role = $this->Role->Text;
		$this->getCallbackClient()->hide('role_window_role_exists');
		if ($role_win_type === self::TYPE_ADD_WINDOW) {
			$config = $this->getModule('user_role')->getRole($role);
			if (count($config) > 0) {
				$this->getCallbackClient()->show('role_window_role_exists');
				return;
			}
		}
		if ($this->getModule('user_role')->isRolePreDefined($role)) {
			// Predefined roles cannot be saved
			return;
		}
		$config = [];
		$config['long_name'] = $this->RoleLongName->Text;
		$config['description'] = $this->RoleDescription->Text;

		$selected_indices = $this->RoleResources->getSelectedIndices();
		$resources = [];
		foreach ($selected_indices as $indice) {
			for ($i = 0; $i < $this->RoleResources->getItemCount(); $i++) {
				if ($i === $indice) {
					$resources[] = $this->RoleResources->Items[$i]->Value;
				}
			}
		}
		$config['resources'] = implode(',', $resources);
		$config['enabled'] = $this->RoleEnabled->Checked ? 1 : 0;
		$this->getModule('role_config')->setRoleConfig($role, $config);
		$this->setRoleList(null, null);
		if ($role_win_type === self::TYPE_ADD_WINDOW) {
			// refresh user window for new role
			$this->initUserWindow();
		}
		$this->getCallbackClient()->callClientFunction('oRoles.save_role_cb');
	}

	/**
	 * Remove roles action.
	 * Here is possible to remove one role or many.
	 * This action is linked with table bulk actions.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param callback parameter
	 * @return none
	 */
	public function removeRoles($sender, $param) {
		$roles = explode('|', $param->getCallbackParameter());
		$config = $this->getModule('role_config')->getConfig();
		$user_role = $this->getModule('user_role');
		for ($i = 0; $i < count($roles); $i++) {
			if (key_exists($roles[$i], $config)) {
				if ($user_role->isRolePreDefined($roles[$i])) {
					// Predefined roles cannot be saved
					continue;
				}
				unset($config[$roles[$i]]);
			}
		}
		$this->getModule('role_config')->setConfig($config);
		$this->setRoleList(null, null);
		// refresh user window to now show removed roles
		$this->initUserWindow();
	}

	/**
	 * Add user statistics to roles.
	 * It adds user count to information about roles.
	 *
	 * @param array $role_config role config (note, passing by reference)
	 * @return none
	 */
	private function addUserStatsToRoles(&$role_config) {
		$config = [];
		if (count($this->user_config) > 0) {
			$config = $this->user_config;
		} else {
			$config = $this->getModule('user_config')->getConfig();
		}
		$user_roles = [];
		foreach ($role_config as $role => $prop) {
			$user_roles[$role] = 0;
		}
		foreach ($config as $username => $prop) {
			$roles = explode(',', $prop['roles']);
			for ($i = 0; $i < count($roles); $i++) {
				$user_roles[$roles[$i]]++;
			}
		}
		foreach ($role_config as $role => $prop) {
			$role_config[$role]['user_count'] = $user_roles[$role];
		}
	}

	/**
	 * Set basic authentication user file.
	 *
	 * @return none
	 */
	private function setBasicAuthConfig() {
		if ($this->isManageUsersAvail() && isset($this->web_config['auth_basic']['user_file'])) {
			$this->getModule('basic_webuser')->setConfigPath($this->web_config['auth_basic']['user_file']);
		}
	}

	/**
	 * Get basic users and provide them to template.
	 *
	 * @param TActiveLinkButton $sender sender
	 * @param TCommandEventParameter $param event parameter object
	 * @return none
	 */
	public function getBasicUsers($sender, $param) {
		if ($param instanceof Prado\Web\UI\TCommandEventParameter && $param->getCommandParameter() === 'load') {
			// reset criteria filters when modal is open
			$this->GetUsersImportOptions->SelectedValue = self::IMPORT_OPT_ALL_USERS;
			$this->GetUsersCriteria->SelectedValue = self::IMPORT_CRIT_USERNAME;
			$this->GetUsersCriteriaFilter->Text = '';
			$this->getCallbackClient()->hide('get_users_criteria');
			$this->getCallbackClient()->hide('get_users_advanced_options');

			// set role resources
			$this->setRoles($this->GetUsersDefaultRole, WebUserRoles::NORMAL);

			// set API hosts
			$this->setAPIHosts($this->GetUsersDefaultAPIHost, HostConfig::MAIN_CATALOG_HOST);
		}

		$params = $this->getBasicParams();

		// add additional parameters
		$this->addBasicExtraParams($params);

		$pattern = '';
		if (!empty($params['filter_val'])) {
			$pattern = '*' . $params['filter_val'] . '*';
		}

		$basic = $this->getModule('basic_webuser');
		// set path from input because user can have unsaved changes
		$basic->setConfigPath($this->BasicAuthUserFile->Text);
		$users = $basic->getUsers($pattern);
		$users = array_keys($users);
		$user_list = $this->convertBasicUsers($users);
		$this->getCallbackClient()->callClientFunction('oUserSecurity.set_user_table_cb', [
			$user_list
		]);
		if (count($users) > 0) {
			// Success
			$this->TestBasicGetUsersMsg->Text = '';
			$this->TestBasicGetUsersMsg->Display = 'None';
			$this->getCallbackClient()->hide('basic_get_users_error');
			$this->getCallbackClient()->show('basic_get_users_ok');
		} else {
			// Error
			$this->getCallbackClient()->show('basic_get_users_error');
			$this->TestBasicGetUsersMsg->Text = Prado::localize('Empty user list');
			$this->TestBasicGetUsersMsg->Display = 'Dynamic';
		}
	}

	/**
	 * Convert basic users from simple username list into full form.
	 * There is option to return user list in config file form or data table form.
	 *
	 * @param array $users simple user list
	 * @param boolean $config_form_result if true, sets the list in config file form
	 * @return array user list
	 */
	private function convertBasicUsers(array $users, $config_form_result = false) {
		$user_list = [];
		for ($i = 0; $i < count($users); $i++) {
			$user = [
				'username' => $users[$i],
				'long_name' => '',
				'email' => '',
				'description' => ''
			];
			if ($config_form_result) {
				$user_list[$users[$i]] = $user;
			} else {
				$user_list[] = $user;
			}
		}
		return $user_list;
	}

	/**
	 * Get basic auth specific parameters with form values.
	 *
	 * @return array array basic auth parameters
	 */
	private function getBasicParams() {
		$params = [];
		$params['allow_manage_users'] = $this->BasicAuthAllowManageUsers->Checked ? 1 : 0;
		$params['user_file'] = $this->BasicAuthUserFile->Text;
		$params['hash_alg'] = $this->BasicAuthHashAlgorithm->SelectedValue;
		return $params;
	}

	/**
	 * Add to basic auth params additional parameters.
	 * Note, extra parameters are not set in config.
	 *
	 * @param array $params basic auth parameters (passing by reference)
	 * @return none
	 */
	private function addBasicExtraParams(&$params) {
		if ($this->GetUsersImportOptions->SelectedValue == self::IMPORT_OPT_CRITERIA) {
			$params['filter_val'] = $this->GetUsersCriteriaFilter->Text;
		}
	}

	/**
	 * Prepare basic users to import.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param event parameter object
	 * @return array web users list to import
	 */
	public function prepareBasicUsers($sender, $param) {
		$users_web = [];
		$import_opt = (integer)$this->GetUsersImportOptions->SelectedValue;
		$basic_webuser = $this->getModule('basic_webuser');
		switch ($import_opt) {
			case self::IMPORT_OPT_ALL_USERS: {
				$users_web = $basic_webuser->getUsers();
				$users_web = array_keys($users_web);
				$users_web = $this->convertBasicUsers($users_web, true);
				break;
			}
			case self::IMPORT_OPT_SELECTED_USERS: {
				if ($param instanceof Prado\Web\UI\ActiveControls\TCallbackEventParameter) {
					$cb_param = $param->getCallbackParameter();
					if (is_array($cb_param)) {
						for ($i = 0; $i < count($cb_param); $i++) {
							$val = (array)$cb_param[$i];
							$users_web[$val['username']] = $val;
						}
					}
				}
				break;
			}
			case self::IMPORT_OPT_CRITERIA: {
				$params = $this->getBasicParams();
				// add additional parameters
				$this->addBasicExtraParams($params);
				if (!empty($params['filter_val'])) {
					$pattern = '*' . $params['filter_val'] . '*';
					$users_web = $basic_webuser->getUsers($pattern);
					$users_web = array_keys($users_web);
					$users_web = $this->convertBasicUsers($users_web, true);

				}
				break;
			}
		}
		return $users_web;
	}

	/**
	 * Test basic user file.
	 *
	 * @param TActiveLinkButton $sender sender object
	 * @param TCallbackEventParameter $param event parameter object
	 * @return none
	 */
	public function doBasicUserFileTest($sender, $param) {
		$user_file = $this->BasicAuthUserFile->Text;
		$msg = '';
		$valid = true;
		if (!file_exists($user_file)) {
			$valid = false;
			$msg = Prado::localize('The user file is not accessible.');
		} else if (!is_readable($user_file)) {
			$valid = false;
			$msg = Prado::localize('The user file is not readable by web server user.');
		} else if (!is_writeable($user_file)) {
			$valid = false;
			$msg = Prado::localize('The user file is readable but not writeable by web server user.');
		}
		$this->BasicAuthUserFileMsg->Text = $msg;
		if ($valid) {
			$this->getCallbackClient()->show('basic_auth_user_file_test_ok');
			$this->BasicAuthUserFileMsg->Display = 'None';
		} else {
			$this->getCallbackClient()->show('basic_auth_user_file_test_error');
			$this->BasicAuthUserFileMsg->Display = 'Dynamic';
		}
	}

	/**
	 * Get LDAP users and provide them to template.
	 *
	 * @param TActiveLinkButton $sender sender
	 * @param TCommandEventParameter $param event parameter object
	 * @return none
	 */
	public function getLdapUsers($sender, $param) {
		if ($param instanceof Prado\Web\UI\TCommandEventParameter && $param->getCommandParameter() === 'load') {
			// reset criteria filters when modal is open
			$this->GetUsersImportOptions->SelectedValue = self::IMPORT_OPT_ALL_USERS;
			$this->GetUsersCriteria->SelectedValue = self::IMPORT_CRIT_USERNAME;
			$this->GetUsersCriteriaFilter->Text = '';
			$this->getCallbackClient()->hide('get_users_criteria');
			$this->getCallbackClient()->hide('get_users_advanced_options');

			// set role resources
			$this->setRoles($this->GetUsersDefaultRole, WebUserRoles::NORMAL);

			// set API hosts
			$this->setAPIHosts($this->GetUsersDefaultAPIHost, HostConfig::MAIN_CATALOG_HOST);
		}

		$ldap = $this->getModule('ldap');
		$params = $this->getLdapParams();
		$ldap->setParams($params);

		// add additional parameters
		$this->addLdapExtraParams($params);

		$filter = $ldap->getFilter($params['user_attr'], '*');
		if (!empty($params['filter_attr']) && !empty($params['filter_val'])) {
			$filter = $ldap->getFilter(
				$params['filter_attr'],
				'*' . $params['filter_val'] . '*'
			);
		}

		$users = $ldap->findUserAttr($filter, $params['attrs']);
		$user_list = $this->convertLdapUsers($users, $params);
		$this->getCallbackClient()->callClientFunction('oUserSecurity.set_user_table_cb', [
			$user_list
		]);

		if (key_exists('count', $users)) {
			// Success
			$this->TestLdapGetUsersMsg->Text = '';
			$this->TestLdapGetUsersMsg->Display = 'None';
			$this->getCallbackClient()->show('ldap_get_users_ok');
		} else {
			// Error
			$this->getCallbackClient()->show('ldap_get_users_error');
			$this->TestLdapGetUsersMsg->Text = $ldap->getLdapError();
			$this->TestLdapGetUsersMsg->Display = 'Dynamic';
		}
	}

	/**
	 * Convert LDAP users from simple username list into full form.
	 * There is option to return user list in config file form or data table form.
	 *
	 * @param array $users simple user list
	 * @param array $params LDAP specific parameters (@see getLdapParams)
	 * @param boolean $config_form_result if true, sets the list in config file form
	 * @return array user list
	 */
	private function convertLdapUsers(array $users, array $params, $config_form_result = false) {
		$user_list = [];
		for ($i = 0; $i < $users['count']; $i++) {
			if (!key_exists($params['user_attr'], $users[$i])) {
				$emsg = "User attribute '{$params['user_attr']}' doesn't exist in LDAP response.";
				$this->getModule('logging')->log(
					__FUNCTION__,
					$emsg,
					Logging::CATEGORY_EXTERNAL,
					__FILE__,
					__LINE__
				);
				continue;
			}
			$username = $long_name = $email = $desc = '';
			if ($params['user_attr'] !== Ldap::DN_ATTR && $users[$i][$params['user_attr']]['count'] != 1) {
				$emsg = "Invalid user attribute count for '{$params['user_attr']}'. Is {$users[$i][$params['user_attr']]['count']}, should be 1.";
				$this->getModule('logging')->log(
					__FUNCTION__,
					$emsg,
					Logging::CATEGORY_EXTERNAL,
					__FILE__,
					__LINE__
				);
				continue;

			}
			$username = $users[$i][$params['user_attr']];
			if ($params['user_attr'] !== Ldap::DN_ATTR) {
				$username = $users[$i][$params['user_attr']][0];
			}

			if (key_exists($params['long_name_attr'], $users[$i])) {
				if ($params['long_name_attr'] === Ldap::DN_ATTR) {
					$long_name = $users[$i][$params['long_name_attr']];
				} else if($users[$i][$params['long_name_attr']]['count'] === 1) {
					$long_name = $users[$i][$params['long_name_attr']][0];
				}
			}

			if (key_exists($params['email_attr'], $users[$i])) {
				if ($params['email_attr'] === Ldap::DN_ATTR) {
					$email = $users[$i][$params['email_attr']];
				} else if ($users[$i][$params['email_attr']]['count'] === 1) {
					$email = $users[$i][$params['email_attr']][0];
				}
			}

			if (key_exists($params['desc_attr'], $users[$i])) {
				if ($params['desc_attr'] === Ldap::DN_ATTR) {
					$desc = $users[$i][$params['desc_attr']];
				} else if ($users[$i][$params['desc_attr']]['count'] === 1) {
					$desc = $users[$i][$params['desc_attr']][0];
				}
			}

			if ($config_form_result) {
				$user_list[$username] = [
					'long_name' => $long_name,
					'email' => $email,
					'description' => $desc
				];
			} else {
				$user_list[] = [
					'username' => $username,
					'long_name' => $long_name,
					'email' => $email,
					'description' => $desc
				];
			}
		}
		return $user_list;
	}


	/**
	 * Get LDAP auth specific parameters with form values.
	 *
	 * @return array array LDAP auth parameters
	 */
	private function getLdapParams() {
		$params = [];
		$params['address'] = $this->LdapAuthServerAddress->Text;
		$params['port'] = $this->LdapAuthServerPort->Text;
		$params['ldaps'] = $this->LdapAuthServerLdaps->Checked ?  1 : 0;
		$params['protocol_ver'] = $this->LdapAuthServerProtocolVersion->SelectedValue;
		$params['base_dn'] = $this->LdapAuthServerBaseDn->Text;
		if ($this->LdapAuthMethodAnonymous->Checked) {
			$params['auth_method'] = Ldap::AUTH_METHOD_ANON;
		} elseif ($this->LdapAuthMethodSimple->Checked) {
			$params['auth_method'] = Ldap::AUTH_METHOD_SIMPLE;
		}
		$params['bind_dn'] = $this->LdapAuthMethodSimpleUsername->Text;
		$params['bind_password'] = $this->LdapAuthMethodSimplePassword->Text;
		$params['user_attr'] = $this->LdapAttributesUsername->Text;
		$params['long_name_attr'] = $this->LdapAttributesLongName->Text;
		$params['desc_attr'] = $this->LdapAttributesDescription->Text;
		$params['email_attr'] = $this->LdapAttributesEmail->Text;
		return $params;
	}

	/**
	 * Add to LDAP auth params additional parameters.
	 * Note, extra parameters are not set in config.
	 *
	 * @param array $params LDAP auth parameters (passing by reference)
	 * @return none
	 */
	private function addLdapExtraParams(&$params) {
		$params['attrs'] = [$params['user_attr']]; // user attribute is obligatory
		if (key_exists('long_name_attr', $params) && !empty($params['long_name_attr'])) {
			$params['attrs'][] = $params['long_name_attr'];
		}
		if (key_exists('email_attr', $params) && !empty($params['email_attr'])) {
			$params['attrs'][] = $params['email_attr'];
		}
		if (key_exists('desc_attr', $params) && !empty($params['desc_attr'])) {
			$params['attrs'][] = $params['desc_attr'];
		}
		if ($this->GetUsersImportOptions->SelectedValue == self::IMPORT_OPT_CRITERIA) {
			$crit = intval($this->GetUsersCriteria->SelectedValue);
			switch ($crit) {
				case self::IMPORT_CRIT_USERNAME: $params['filter_attr'] = $params['user_attr']; break;
				case self::IMPORT_CRIT_LONG_NAME: $params['filter_attr'] = $params['long_name_attr']; break;
				case self::IMPORT_CRIT_DESCRIPTION: $params['filter_attr'] = $params['desc_attr']; break;
				case self::IMPORT_CRIT_EMAIL: $params['filter_attr'] = $params['email_attr']; break;
			}
			$params['filter_val'] = $this->GetUsersCriteriaFilter->Text;
		}
	}


	/**
	 * Prepare LDAP users to import.
	 *
	 * @param TCallback $sender sender object
	 * @param TCallbackEventParameter $param event parameter object
	 * @return array web users list to import
	 */
	private function prepareLdapUsers($sender, $param) {
		$ldap = $this->getModule('ldap');
		$params = $this->getLdapParams();
		$ldap->setParams($params);

		// add additional parameters
		$this->addLdapExtraParams($params);

		$import_opt = (integer)$this->GetUsersImportOptions->SelectedValue;

		$users_web = [];
		switch ($import_opt) {
			case self::IMPORT_OPT_ALL_USERS: {
				$filter = $ldap->getFilter($params['user_attr'], '*');
				$users_ldap = $ldap->findUserAttr($filter, $params['attrs']);
				$users_web = $this->convertLdapUsers($users_ldap, $params, true);
				break;
			}
			case self::IMPORT_OPT_SELECTED_USERS: {
				if ($param instanceof Prado\Web\UI\ActiveControls\TCallbackEventParameter) {
					$cb_param = $param->getCallbackParameter();
					if (is_array($cb_param)) {
						for ($i = 0; $i < count($cb_param); $i++) {
							$val = (array)$cb_param[$i];
							$users_web[$val['username']] = $val;
							unset($users_web[$val['username']]['username']);
						}
					}
				}
				break;
			}
			case self::IMPORT_OPT_CRITERIA: {
				if (!empty($params['filter_attr']) && !empty($params['filter_val'])) {
					$filter = $ldap->getFilter(
						$params['filter_attr'],
						'*' . $params['filter_val'] . '*'
					);
					$users_ldap = $ldap->findUserAttr($filter, $params['attrs']);
					$users_web = $this->convertLdapUsers($users_ldap, $params, true);

				}
				break;
			}
		}
		return $users_web;
	}

	/**
	 * Test LDAP connection.
	 *
	 * @param TActiveLinkButton $sender sender object
	 * @param TCallbackEventParameter $param event object parameter
	 * @return none
	 */
	public function testLdapConnection($sender, $param) {
		$ldap = $this->getModule('ldap');
		$params = $this->getLdapParams();
		$ldap->setParams($params);

		if ($ldap->adminBind()) {
			$this->TestLdapConnectionMsg->Text = '';
			$this->TestLdapConnectionMsg->Display = 'None';
			$this->getCallbackClient()->show('ldap_test_connection_ok');
		} else {
			$this->getCallbackClient()->show('ldap_test_connection_error');
			$this->TestLdapConnectionMsg->Text = $ldap->getLdapError();
			$this->TestLdapConnectionMsg->Display = 'Dynamic';
		}
	}

	/**
	 * Main method to import users.
	 * Supported are basic auth and LDAP auth user imports.
	 *
	 * @param TActiveLinkButton $sender sender object
	 * @param TCallbackEventParameter $param event object parameter
	 * @return none
	 */
	public function importUsers($sender, $param) {
		if (!$this->GetUsersDefaultIps->IsValid) {
			// invalid IP restriction value
			return;
		}

		$users_web = [];
		if ($this->BasicAuth->Checked) {
			$users_web = $this->prepareBasicUsers($sender, $param);
		} elseif ($this->LdapAuth->Checked) {
			$users_web = $this->prepareLdapUsers($sender, $param);
		}

		// Get default roles for imported users
		$def_roles = $this->GetUsersDefaultRole->getSelectedIndices();
		$role_list = [];
		foreach ($def_roles as $indice) {
			for ($i = 0; $i < $this->GetUsersDefaultRole->getItemCount(); $i++) {
				if ($i === $indice) {
					$role_list[] = $this->GetUsersDefaultRole->Items[$i]->Value;
				}
			}
		}
		$roles = implode(',', $role_list);

		// Get default API hosts for imported users
		$api_hosts = $this->GetUsersDefaultAPIHost->SelectedValue;

		// Get default IP address restrictions for imported users
		$ips = $this->trimIps($this->GetUsersDefaultIps->Text);

		// fill missing default values
		$add_def_user_params = function (&$user, $idx) use ($roles, $api_hosts, $ips) {
			$user['roles'] = $roles;
			$user['api_hosts'] = $api_hosts;
			$user['ips'] = $ips;
			$user['enabled'] = '1';
		};
		array_walk($users_web, $add_def_user_params);

		$user_mod = $this->getModule('user_config');
		$users = $user_mod->getConfig();

		$users_cfg = [];
		if ($this->GetUsersProtectOverwrite->Checked) {
			$users_cfg = array_merge($users_web, $users);
		} else {
			$users_cfg = array_merge($users, $users_web);
		}
		$user_mod->setConfig($users_cfg);

		// refresh user list
		$this->setUserList(null, null);

		// refresh role list
		$this->setRoleList(null, null);

		$this->getCallbackClient()->callClientFunction('oUserSecurity.show_user_modal', [
			false
		]);
	}

	/**
	 * Get users and provide them to template.
	 *
	 * @param TActiveLinkButton $sender sender object
	 * @param TCallbackEventParameter $param event object parameter
	 * @return none
	 */
	public function getUsers($sender, $param) {
		if ($this->BasicAuth->Checked) {
			$this->getBasicUsers($sender, $param);
		} elseif ($this->LdapAuth->Checked) {
			$this->getLdapUsers($sender, $param);
		}
	}

	/**
	 * Save security config.
	 *
	 * @param TActiveLinkButton $sender sender object
	 * @param TCallbackEventParameter $param event object parameter
	 * @return none
	 */
	public function saveSecurityConfig($sender, $param) {
		$config = $this->web_config;
		if (!key_exists('security', $config)) {
			$config['security'] = [];
		}
		if ($this->GeneralDefaultNoAccess->Checked) {
			$config['security']['def_access'] = WebConfig::DEF_ACCESS_NO_ACCESS;
		} elseif ($this->GeneralDefaultAccess->Checked) {
			$config['security']['def_access'] = WebConfig::DEF_ACCESS_DEFAULT_SETTINGS;
			$config['security']['def_role'] = $this->GeneralDefaultAccessRole->SelectedValue;
			$config['security']['def_api_host'] = $this->GeneralDefaultAccessAPIHost->SelectedValue;
		}
		if ($this->BasicAuth->Checked) {
			$config['security']['auth_method'] = WebConfig::AUTH_METHOD_BASIC;
			$config['auth_basic'] = $this->getBasicParams();
		} else if ($this->LdapAuth->Checked) {
			$config['security']['auth_method'] = WebConfig::AUTH_METHOD_LDAP;
			$config['auth_ldap'] = $this->getLdapParams();
		}
		$ret = $this->getModule('web_config')->setConfig($config);
		if ($ret === true) {
			$this->getCallbackClient()->hide('auth_method_save_error');
			$this->getCallbackClient()->show('auth_method_save_ok');
		} else {
			$this->getCallbackClient()->hide('auth_method_save_ok');
			$this->getCallbackClient()->show('auth_method_save_error');
		}
	}

	/**
	 * Determines if user management is enabled.
	 * This checking bases on selected auth method and permission to manage users.
	 *
	 * @return boolean true if managing users is enabled, otherwise false
	 */
	private function isManageUsersAvail() {
		$is_basic = $this->getModule('web_config')->isAuthMethodBasic();
		$allow_manage_users = (isset($this->web_config['auth_basic']['allow_manage_users']) &&
			$this->web_config['auth_basic']['allow_manage_users'] == 1);
		return ($is_basic && $allow_manage_users);
	}

	/**
	 * Validate IP restriction address value.
	 *
	 * @param TActiveCustomValidator $sender sender object
	 * @param TServerValidateEventParameter $param event object parameter
	 * @return none
	 */
	public function validateIps($sender, $param) {
		$valid = true;
		$val = trim($param->Value);
		if (!empty($val)) {
			$ips = explode(',', $val);
			for ($i = 0; $i < count($ips); $i++) {
				$ip = trim($ips[$i]);
				if (!filter_var($ip, FILTER_VALIDATE_IP) && !(strpos($ip, '*') !== false && preg_match('/^[\da-f:.*]+$/i', $ip) === 1)) {
					$valid = false;
					break;
				}
			}
		}
		$param->IsValid = $valid;
	}

	/**
	 * Simple helper that trims IP restriction address values.
	 *
	 * @param string $ips IP restriction address values
	 * @return string trimmed addresses
	 */
	public function trimIps($ips) {
		$ips = trim($ips);
		if (!empty($ips)) {
			$ips = explode(',', $ips);
			$ips = array_map('trim', $ips);
			$ips = implode(',', $ips);
		}
		return $ips;
	}
}
?>
