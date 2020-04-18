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

Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TActiveRepeater');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('Application.Web.Class.BaculumWebPage'); 

/**
 * Users page.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class Users extends BaculumWebPage {

	protected $admin = true;

	public function setUsers() {
		$all_users = $this->getModule('basic_webuser')->getAllUsers();
		$users = array_keys($all_users);
		sort($users);
		$users_list = array();
		$users_feature = (array_key_exists('users', $this->web_config) && is_array($this->web_config['users']));
		for ($i = 0; $i < count($users); $i++) {
			$host = null;
			if ($users_feature && array_key_exists($users[$i], $this->web_config['users'])) {
				$host = $this->web_config['users'][$users[$i]];
			}
			$users_list[] = array(
				'user' => $users[$i],
				'host' => $host,
				'admin' => ($users[$i] === $this->web_config['baculum']['login'])
			);
		}
		$this->UsersList->dataSource = $users_list;
		$this->UsersList->dataBind();
	}

	public function initHosts($sender, $param) {
		$api_hosts = array_keys($this->getModule('host_config')->getConfig());
		$sender->DataSource = array_combine($api_hosts, $api_hosts);
		$sender->dataBind();
	}

	public function userAction($sender, $param) {
		$this->UsersList->ActiveControl->EnableUpdate = true;
		list($action, $user, $value) = explode(';', $param->CallbackParameter, 3);
		switch($action) {
			case 'newuser':
			case 'chpwd': {
					$this->getModule('basic_webuser')->setUsersConfig($user, $value);
					if ($user === $this->web_config['baculum']['login']) {
						// if admin password changed then try to auto-login by async request
						$http_protocol = isset($_SERVER['HTTPS']) && !empty($_SERVER['HTTPS']) ? 'https' : 'http';
						$this->switchToUser($user, $value);
						exit();
					} else {
						// if normal user's password changed then update users grid
						$this->setUsers();
					}
				}
				break;
			case 'rmuser': {
					if ($user != $_SERVER['PHP_AUTH_USER']) {
						$this->getModule('basic_webuser')->removeUser($user);
						if (array_key_exists('users', $this->web_config) && array_key_exists($user, $this->web_config['users'])) {
							unset($this->web_config['users'][$user]);
						}
						$this->getModule('web_config')->setConfig($this->web_config);
						$this->setUsers();
					}
				break;
				}
			case 'set_host': {
					if (empty($value) && array_key_exists($user, $this->web_config['users'])) {
						unset($this->web_config['users'][$user]);
					} else {
						$this->web_config['users'][$user] = $value;
					}
					$this->getModule('web_config')->setConfig($this->web_config);
				break;
				}
		}
	}

	public function isSelectedHost($user, $host) {
		if (!key_exists('users', $this->web_config)) {
			return;
		}
		return (key_exists($user, $this->web_config['users']) && $this->web_config['users'][$user] === $host);
	}
}
?>
