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
	public $jobs;

	public $open_window = null;

	public $init_window_id = null;

	public $init_element_id = null;

	public $jobs_states = null;

	public $window_ids = array('Storage', 'Client', 'Volume', 'Pool', 'Job', 'JobRun');

	private $web_config = array();

	private $api_hosts = array();

	public function onPreInit($param) {
		parent::onPreInit($param);
		$this->web_config = $this->getModule('web_config')->getConfig();
		if (!$this->IsPostBack && !$this->IsCallBack) {
			$this->setSessionUserVars($this->web_config);
		}
	}

	public function onInit($param) {
		parent::onInit($param);
		if(count($this->web_config) === 0) {
			// Config doesn't exist
			$this->goToPage('WebConfigWizard');
		}
		if (!$this->IsPostBack && !$this->IsCallBack) {
			$this->getModule('api')->initSessionCache(true);
		}

		$this->Users->Visible = $_SESSION['admin'];
		$this->Config->Visible = $_SESSION['admin'];
		$this->SettingsWizardBtn->Visible = $_SESSION['admin'];
		$this->PoolBtn->Visible = $_SESSION['admin'];
		$this->VolumeBtn->Visible = $_SESSION['admin'];
		$this->ClearBvfsCache->Visible = $_SESSION['admin'];
		$this->Logging->Visible = $_SESSION['admin'];

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
			$this->setJobsStates();
			$this->setJobs();
			$this->setClients();
			$this->setWindowOpen();
			$this->BaculaConfig->loadConfig(null, null);
		}
	}

	private function setSessionUserVars($cfg) {
		if (count($cfg) > 0) {
			// Set administrator role
			$_SESSION['admin'] = ($_SERVER['PHP_AUTH_USER'] === $cfg['baculum']['login']);
			// Set api host for normal user
			if (!$_SESSION['admin'] && array_key_exists('users', $cfg) && array_key_exists($_SERVER['PHP_AUTH_USER'], $cfg['users'])) {
				$_SESSION['api_host'] = $cfg['users'][$_SERVER['PHP_AUTH_USER']];
			} elseif (isset($_SESSION['api_host'])) {
				unset($_SESSION['api_hosts']);
			}
		} else {
			$_SESSION['admin'] = false;
		}
	}

	public function director($sender, $param) {
		$_SESSION['director'] = $this->Director->SelectedValue;
	}

	public function setDebug($sender, $param) {
		if($_SESSION['admin']) {
			$this->enableDebug($this->Logging->Checked);
			$this->goToDefaultPage();
		}
	}

	public function enableDebug($enable) {
		$result = false;
		if(count($this->web_config) > 0) {
			$this->web_config['baculum']['debug'] = ($enable === true) ? "1" : "0";
			$result = $this->getModule('web_config')->setConfig($this->web_config);
		}
		return $result;
	}

	public function clearBvfsCache($sender, $param) {
		if($_SESSION['admin']) {
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
		$jobs_states = array();

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

	public function setWindowOpen() {
		if (isset($this->Request['open']) && in_array($this->Request['open'], $this->window_ids) && $this->Request['open'] != 'JobRun') {
			$btn = $this->Request['open'] . 'Btn';
			$this->open_window = $this->{$btn}->ClientID;
			if (isset($this->Request['id']) && (is_numeric($this->Request['id']))) {
				$this->init_window_id = $this->Request['open'];
				$this->init_element_id = $this->Request['id'];
			}
		}
	}

	public function logout($sender, $param) {
		$fake_pwd = $this->getModule('misc')->getRandomString();
		$this->switchToUser($_SERVER['PHP_AUTH_USER'], $fake_pwd);
		exit();
	}
}
?>
