<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2016 Kern Sibbald
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

Prado::using('System.Web.UI.JuiControls.TJuiProgressbar');
Prado::using('System.Web.UI.ActiveControls.TActiveDataGrid');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('Application.API.Class.BaculumAPIPage');

class APIHome extends BaculumAPIPage {

	public function onInit($param) {
		parent::onInit($param);

		$config = $this->getModule('api_config')->getConfig();
		if(count($config) === 0) {
			// Config doesn't exist, go to wizard
			$this->goToPage('Panel.APIInstallWizard');
			return;
		} elseif (!$this->IsCallback) {
			$this->loadBasicUsers(null, null);
			$this->loadOAuth2Users(null, null);
			$this->setAuthParams(null, null);
		}
	}

	public function setAuthParams($sender, $param) {
		$config = $this->getModule('api_config')->getConfig();
		$base_params = array('auth_type' => $config['api']['auth_type']);
		$params = array();
		if ($config['api']['auth_type'] === 'oauth2') {
			$oauth2_cfg = $this->getModule('oauth2_config')->getConfig();
			$client_id = null;
			if (is_object($param)) {
				$client_id = $param->CallbackParameter;
			}
			if (is_string($client_id)) {
				$params = array(
					'client_id' => $oauth2_cfg[$client_id]['client_id'],
					'client_secret' =>  $oauth2_cfg[$client_id]['client_secret'],
					'redirect_uri' => $oauth2_cfg[$client_id]['redirect_uri'],
					'scope' => explode(' ', $oauth2_cfg[$client_id]['scope'])
				);
			}
		} elseif ($config['api']['auth_type'] === 'basic') {
			if (is_object($param)) {
				$params['login'] = $param->CallbackParameter;
				$params['password'] = '';
			} else {
				// no auth params, possibly no authentication
				$params['login'] = $params['password'] = '';
			}
		}
		$params = array_merge($base_params, $params);
		$this->AuthParamsInput->Value = json_encode($params);
	}

	public function loadBasicUsers($sender, $param) {
		$basic_users = $this->getBasicUsers();
		$this->BasicClientList->dataSource = $basic_users;
		$this->BasicClientList->dataBind();
		$this->BasicClientList->ensureChildControls();
	}

	public function loadOAuth2Users($sender, $param) {
		$oauth2_cfg = $this->getModule('oauth2_config')->getConfig();
		$this->OAuth2ClientList->dataSource = array_values($oauth2_cfg);
		$this->OAuth2ClientList->dataBind();
		$this->loadAuthParams(null, null);
	}

	public function loadAuthParams($sender, $param) {
		$ids = $values = array();
		$config = $this->getModule('api_config')->getConfig();
		if ($config['api']['auth_type'] === 'oauth2') {
			$oauth2_cfg = $this->getModule('oauth2_config')->getConfig();
			$ids = array_keys($oauth2_cfg);
			$values = array();
			for ($i = 0; $i < count($ids); $i++) {
				$values[] = "{$oauth2_cfg[$ids[$i]]['client_id']} ({$oauth2_cfg[$ids[$i]]['name']})";
			}
		} elseif ($config['api']['auth_type'] === 'basic') {
			$api_user_cfg = $this->getModule('basic_apiuser')->getAllUsers();
			$values = $ids = array_keys($api_user_cfg);
		}
		$this->AuthParamsCombo->DataSource = array_combine($ids, $values);
		$this->AuthParamsCombo->dataBind();
	}

	private function getBasicUsers() {
		$basic_users = array();
		$basic_cfg = $this->getModule('basic_apiuser')->getAllUsers();
		foreach($basic_cfg as $user => $pwd) {
			$basic_users[] = array('username' => $user);
		}
		return $basic_users;
	}

	public function deleteBasicItem($sender, $param) {
		$config = $this->getModule('basic_apiuser');
		$config->removeUser($param->getCommandParameter());
		$this->BasicClientList->DataSource = $this->getBasicUsers();
		$this->BasicClientList->dataBind();
	}

	public function deleteOAuth2Item($sender, $param) {
		$config = $this->getModule('oauth2_config');
		$clients = $config->getConfig();
		$client_id = $param->getCommandParameter();
		if (key_exists($client_id, $clients)) {
			unset($clients[$client_id]);
		}
		$config->setConfig($clients);
		$this->OAuth2ClientList->DataSource = array_values($config->getConfig());
		$this->OAuth2ClientList->dataBind();
		$this->loadAuthParams(null, null);
	}

	public function editOAuth2Item($sender, $param) {
		$config = $this->getModule('oauth2_config');
		$clients = $config->getConfig();
		$client_id = $param->getCommandParameter();
		if (key_exists($client_id, $clients)) {
			$this->APIOAuth2ClientId->Text = $clients[$client_id]['client_id'];
			$this->APIOAuth2ClientSecret->Text = $clients[$client_id]['client_secret'];
			$this->APIOAuth2RedirectURI->Text = $clients[$client_id]['redirect_uri'];
			$this->APIOAuth2Scope->Text = $clients[$client_id]['scope'];
			$this->APIOAuth2BconsoleCfgPath->Text = $clients[$client_id]['bconsole_cfg_path'];
			$this->APIOAuth2Name->Text = $clients[$client_id]['name'];
		}
		$this->APIOAuth2EditPopup->open();
	}

	public function saveOAuth2Item($sender, $param) {
		$config = $this->getModule('oauth2_config');
		$clients = $config->getConfig();
		$client_id = $this->APIOAuth2ClientId->Text;
		if (key_exists($client_id, $clients)) {
			$clients[$client_id]['client_id'] = $client_id;
			$clients[$client_id]['client_secret'] = $this->APIOAuth2ClientSecret->Text;
			$clients[$client_id]['redirect_uri'] = $this->APIOAuth2RedirectURI->Text;
			$clients[$client_id]['scope'] = $this->APIOAuth2Scope->Text;
			$clients[$client_id]['bconsole_cfg_path'] = $this->APIOAuth2BconsoleCfgPath->Text;
			$clients[$client_id]['name'] = $this->APIOAuth2Name->Text;
			$config->setConfig($clients);
		}
		$this->APIOAuth2EditPopup->close();
		$this->OAuth2ClientList->DataSource = array_values($clients);
		$this->OAuth2ClientList->dataBind();
		$this->loadAuthParams(null, null);
	}
}
?>
