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

Prado::using('Application.Common.Class.BasicUserConfig');
Prado::using('Application.Common.Class.Logging');
Prado::using('Application.Web.Init'); 
Prado::using('Application.Web.Class.BaculumWebPage'); 
Prado::using('System.Web.UI.ActiveControls.TActiveButton');
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActiveCheckBox');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');

class WebHome extends BaculumWebPage
{
	protected $app_config;

	public $jobs;

	public $openWindow = null;

	public $initWindowId = null;

	public $initElementId = null;

	public $jobs_states = null;

	public $dbtype = '';

	public $windowIds = array('Storage', 'Client', 'Volume', 'Pool', 'Job', 'JobRun');


	public function onInit($param) {
		parent::onInit($param);
		$this->Application->getModule('web_users')->loginUser();

		if (!$this->IsPostBack && !$this->IsCallBack) {
			$this->getModule('api')->initSessionCache(true);
		}

		$config = $this->getModule('web_config')->getConfig();
		if(count($config) === 0) {
			// Config doesn't exist
			$this->goToPage('WebConfigWizard');
		}

		$this->Users->Visible = $this->User->getIsAdmin();
		$this->SettingsWizardBtn->Visible = $this->User->getIsAdmin();
		$this->PoolBtn->Visible = $this->User->getIsAdmin();
		$this->VolumeBtn->Visible = $this->User->getIsAdmin();
		$this->ClearBvfsCache->Visible = $this->User->getIsAdmin();
		$this->Logging->Visible = $this->User->getIsAdmin();

		if(!$this->IsPostBack && !$this->IsCallBack) {
			$this->Logging->Checked = Logging::$debug_enabled;
		}

		if(!$this->IsPostBack && !$this->IsCallBack) {
			$directors = $this->getModule('api')->get(array('directors'))->output;
			if(!array_key_exists('director', $_SESSION) || $directors[0] != $_SESSION['director']) {
				$_SESSION['director'] = $directors[0];
			}
			$this->Director->dataSource = array_combine($directors, $directors);
			$this->Director->SelectedValue = $_SESSION['director'];
			$this->Director->dataBind();
			// Web doesn't store any info about db and it is OK
			/*if ($this->User->getIsAdmin() === true) {
				$this->dbtype = $this->app_config['db']['type'];
			}*/
			$this->setJobsStates();
			$this->setJobs();
			$this->setClients();
			$this->setUsers();
			$this->setWindowOpen();
			$this->BaculaConfig->loadConfig(null, null);
		}
	}

	public function director($sender, $param) {
		$_SESSION['director'] = $this->Director->SelectedValue;
	}

	public function setDebug($sender, $param) {
		if($this->User->getIsAdmin() === true) {
			$this->enableDebug($this->Logging->Checked);
			$this->goToDefaultPage();
		}
	}

	public function enableDebug($enable) {
		$result = false;
		$config = $this->getModule('web_config')->getConfig();
		if(count($config) > 0) {
			$config['baculum']['debug'] = ($enable === true) ? "1" : "0";
			$result = $this->getModule('web_config')->setConfig($config);
		}
		return $result;
	}

	public function clearBvfsCache($sender, $param) {
		if($this->User->getIsAdmin() === true) {
			$this->getModule('api')->set(array('bvfs', 'clear'), array());
		}
	}

	public function getJobs() {
		return json_encode($this->jobs);
	}

	public function setJobsStates() {
		$jobs_summary = array(
			'ok' => array(),
			'error' => array(),
			'warning' => array(),
			'cancel' => array(),
			'running' => array()
		);
		$job_types = $jobs_summary;
		$job_states = array();

		$misc = $this->getModule('misc');
		foreach($job_types as $type => $arr) {
			$states = $misc->getJobStatesByType($type);
			foreach($states as $state => $desc) {
				$desc['type'] = $type;
				$jobs_states[$state] = $desc;
			}
		}

		$this->jobs_states = json_encode($jobs_states);
	}

	public function setJobs() {
		$this->jobs = $this->getModule('api')->get(array('jobs'));
		$jobs = array('@' => Prado::localize('select job'));
		foreach($this->jobs->output as $key => $job) {
			$jobs[$job->name] = $job->name;
		}
		$this->Jobs->dataSource = $jobs;
		$this->Jobs->dataBind();
	}

	public function setClients() {
		$clients_obj = $this->getModule('api')->get(array('clients'));
		$clients = array('@' => Prado::localize('select client'));
		foreach($clients_obj->output as $key => $client) {
			$clients[$client->clientid] = $client->name;
		}
		$this->Clients->dataSource = $clients;
		$this->Clients->dataBind();
	}

	public function setUsers() {
		if($this->User->getIsAdmin() === true) {
			$allUsers = $this->getModule('basic_webuser')->getAllUsers();
			$users = array_keys($allUsers);
			sort($users);
			$this->UsersList->dataSource = $users;
			$this->UsersList->dataBind();
		}
	}

	public function userAction($sender, $param) {
		$config = $this->getModule('web_config');
		$cfg = $config->getConfig();

		if($this->User->getIsAdmin() === true) {
			list($action, $user, $value) = explode(';', $param->CallbackParameter, 3);
			switch($action) {
				case 'newuser':
				case 'chpwd': {
						$admin = false;
						$valid = true;
						if ($user === $cfg['baculum']['login']) {
							$cfg['baculum']['password'] = $value;
							$valid = $config->setConfig($cfg);
							$admin = true;
						}
						if ($valid === true) {
							$this->getModule('basic_webuser')->setUsersConfig($user, $value);
						}
						if ($admin === true) {
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
						if ($user != $this->User->getName()) {
							$this->getModule('basic_webuser')->removeUser($user);
							$this->setUsers();
						}
					break;
					}
			}
		}
	}

	public function setWindowOpen() {
		if (isset($this->Request['open']) && in_array($this->Request['open'], $this->windowIds) && $this->Request['open'] != 'JobRun') {
			$btn = $this->Request['open'] . 'Btn';
			$this->openWindow = $this->{$btn}->ClientID;
			if (isset($this->Request['id']) && (is_numeric($this->Request['id']))) {
				$this->initWindowId = $this->Request['open'];
				$this->initElementId = $this->Request['id'];
			}
		}
	}

	public function logout($sender, $param) {
		$http_protocol = isset($_SERVER['HTTPS']) && !empty($_SERVER['HTTPS']) ? 'https' : 'http';
		$fake_pwd = $this->getModule('web_config')->getRandomString();
		$this->switchToUser($http_protocol, $_SERVER['SERVER_NAME'], $_SERVER['SERVER_PORT'], $this->User->getName(), $fake_pwd);
		exit();
	}
}
?>
