<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2019 Kern Sibbald
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

// @TODO: move this page to API part before any release.
 
Prado::using('Application.Web.Class.BaculumWebPage'); 
Prado::using('Application.Web.Class.HostConfig');
Prado::using('Application.Web.Class.BasicWebUserConfig'); 
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');

/**
 * Web config wizard page.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class WebConfigWizard extends BaculumWebPage
{

	protected $admin = false;

	public $first_run;
	public $web_config;
	public $host_config;

	public function onInit($param) {
		parent::onInit($param);
		$this->Lang->SelectedValue = $this->getLanguage();
		$config = $this->getModule('web_config');
		$this->web_config = $config->getConfig();
		$this->host_config = $this->getModule('host_config')->getConfig();
		$this->first_run = (count($this->host_config) == 0 || !key_exists(HostConfig::MAIN_CATALOG_HOST, $this->host_config));
		Logging::$debug_enabled = Logging::$debug_enabled ?: $this->first_run;
		if($this->first_run === false && !$_SESSION['admin']) {
			parent::accessDenied();
		}
	}

	public function onLoad($param) {
		parent::onLoad($param);
		if($this->IsPostBack || $this->IsCallBack) {
			return;
		}
		if ($this->first_run === false) {
			$host = HostConfig::MAIN_CATALOG_HOST;
			$this->AddNewHost->APIProtocol->SelectedValue = $this->host_config[$host]['protocol'];
			$this->AddNewHost->APIAddress->Text = $this->host_config[$host]['address'];
			$this->AddNewHost->APIPort->Text = $this->host_config[$host]['port'];
			$this->AddNewHost->APIBasicLogin->Text = $this->host_config[$host]['login'];
			if ($this->host_config[$host]['auth_type'] === 'basic') {
				$this->AddNewHost->AuthOAuth2->Checked = false;
				$this->AddNewHost->AuthBasic->Checked = true;
				$this->AddNewHost->APIBasicLogin->Text = $this->host_config[$host]['login'];
				$this->AddNewHost->APIBasicPassword->Text = $this->host_config[$host]['password'];
			} elseif ($this->host_config[$host]['auth_type'] === 'oauth2') {
				$this->AddNewHost->AuthBasic->Checked = false;
				$this->AddNewHost->AuthOAuth2->Checked = true;
				$this->AddNewHost->APIOAuth2ClientId->Text = $this->host_config[$host]['client_id'];
				$this->AddNewHost->APIOAuth2ClientSecret->Text = $this->host_config[$host]['client_secret'];
				$this->AddNewHost->APIOAuth2RedirectURI->Text = $this->host_config[$host]['redirect_uri'];
				$this->AddNewHost->APIOAuth2Scope->Text = $this->host_config[$host]['scope'];
			}
			$this->WebLogin->Text = $this->web_config['baculum']['login'];
		} else {
			$this->AddNewHost->APIProtocol->SelectedValue = 'http';
			$this->AddNewHost->APIAddress->Text = 'localhost';
			$this->AddNewHost->APIPort->Text = 9096;
			$this->AddNewHost->APIBasicLogin->Text = 'admin';
		}
	}

	public function NextStep($sender, $param) {
	}
	
	public function PreviousStep($sender, $param) {
	}

	public function wizardStop($sender, $param) {
		$this->goToDefaultPage();
	}

	public function wizardCompleted() {
		$host = HostConfig::MAIN_CATALOG_HOST;
		$cfg_host = array(
			'auth_type' => '',
			'login' => '',
			'password' => '',
			'client_id' => '',
			'client_secret' => '',
			'redirect_uri' => '',
			'scope' => ''
		);
		$cfg_host['protocol'] = $this->AddNewHost->APIProtocol->Text;
		$cfg_host['address'] = $this->AddNewHost->APIAddress->Text;
		$cfg_host['port'] = $this->AddNewHost->APIPort->Text;
		$cfg_host['url_prefix'] = '';
		if ($this->AddNewHost->AuthBasic->Checked == true) {
			$cfg_host['auth_type'] = 'basic';
			$cfg_host['login'] = $this->AddNewHost->APIBasicLogin->Text;
			$cfg_host['password'] = $this->AddNewHost->APIBasicPassword->Text;
		} elseif($this->AddNewHost->AuthOAuth2->Checked == true) {
			$cfg_host['auth_type'] = 'oauth2';
			$cfg_host['client_id'] = $this->AddNewHost->APIOAuth2ClientId->Text;
			$cfg_host['client_secret'] = $this->AddNewHost->APIOAuth2ClientSecret->Text;
			$cfg_host['redirect_uri'] = $this->AddNewHost->APIOAuth2RedirectURI->Text;
			$cfg_host['scope'] = $this->AddNewHost->APIOAuth2Scope->Text;
		}
		$host_config = $this->getModule('host_config')->getConfig();
		$host_config[$host] = $cfg_host;
		$ret = $this->getModule('host_config')->setConfig($host_config);
		if($ret === true) {
			$web_config = $this->getModule('web_config')->getConfig();
			$cfg_web = array('baculum' => array(), 'users' => array());
			if (count($web_config) > 0) {
				$cfg_web = $web_config;
			}
			$cfg_web['baculum']['login'] = $this->WebLogin->Text;
			$cfg_web['baculum']['debug'] = 0;
			$cfg_web['baculum']['lang'] = $this->Lang->SelectedValue;
			if (array_key_exists('users', $cfg_web) && array_key_exists($this->WebLogin->Text, $cfg_web)) {
				// Admin shoudn't be added to users section, only regular users
				unset($cfg_web['users'][$this->WebLogin->Text]);
			}
			$ret = $this->getModule('web_config')->setConfig($cfg_web);
			if($ret && $this->getModule('basic_webuser')->isUsersConfig() === true) {
				$previous_user = $this->first_run ? parent::DEFAULT_AUTH_USER : $this->web_config['baculum']['login'];
				$this->getModule('basic_webuser')->setUsersConfig(
					$cfg_web['baculum']['login'],
					$this->WebPassword->Text,
					false,
					$previous_user
				);
			}
			$this->goToDefaultPage();
		}
	}

	public function setLogin($db) {
		$this->Login->Enabled = ($this->isSQLiteType($db) === false);
	}

	public function setPassword($db) {
		$this->Password->Enabled = ($this->isSQLiteType($db) === false);
	}

	public function setLang($sender, $param) {
		$_SESSION['language'] = $sender->SelectedValue;
	}

	public function validateAdministratorPassword($sender, $param) {
		if ($this->RetypeWebPasswordRequireValidator->IsValid && $this->RetypeWebPasswordRegexpValidator->IsValid) {
			$sender->Display = 'Dynamic';
		} else {
			$sender->Display = 'None';
		}
		$param->IsValid = ($param->Value === $this->WebPassword->Text);
	}
}
?>
