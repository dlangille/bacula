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
Prado::using('Application.API.Class.BaculumAPIPage');

class APIHome extends BaculumAPIPage {

	public $auth_params;
	public $main_client_id;

	public function onInit($param) {
		parent::onInit($param);

		$config = $this->getModule('api_config')->getConfig();
		if(count($config) === 0) {
			// Config doesn't exist, go to wizard
			$this->goToPage('Panel.APIInstallWizard');
		} else {
			$base_params = array('auth_type' => $config['api']['auth_type']);
			$params = array();
			$oauth2_cfg = $this->getModule('oauth2_config')->getConfig();
			if ($config['api']['auth_type'] === 'oauth2') {
				if (array_key_exists($config['api']['client_id'], $oauth2_cfg)) {
					$this->main_client_id = $config['api']['client_id'];
					$params = array(
						'client_id' => $config['api']['client_id'],
						'client_secret' =>  $oauth2_cfg[$config['api']['client_id']]['client_secret'],
						'redirect_uri' => $oauth2_cfg[$config['api']['client_id']]['redirect_uri'],
						'scope' => explode(' ', $oauth2_cfg[$config['api']['client_id']]['scope'])
					);
				}
			} elseif ($config['api']['auth_type'] === 'basic') {
				$params = array(
					'login' => $config['api']['login'],
					'password' => $config['api']['password']
				);
				// login and password are not needed because user is already logged in
			}
			$params = array_merge($base_params, $params);
			$this->auth_params = json_encode($params);
			$this->loadBasicUsers(null, null);
			$this->loadOAuth2Users(null, null);
		}
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
		if (array_key_exists($client_id, $clients)) {
			unset($clients[$client_id]);
		}
		$config->setConfig($clients);
		$this->OAuth2ClientList->DataSource = array_values($config->getConfig());
		$this->OAuth2ClientList->dataBind();
	}
}
?>
