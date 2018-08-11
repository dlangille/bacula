<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2017 Kern Sibbald
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

Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveButton');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.TTemplateControl');
Prado::using('Application.Common.Class.OAuth2');
Prado::using('Application.Common.Portlets.PortletTemplate');

class NewAuthClient extends PortletTemplate {

	private $show_buttons = true;

	private $auth_types = array('basic', 'oauth2');

	private $auth_type;

	public function addNewAuthClient($sender, $param) {
		$this->NewAuthClientAddOk->Display = 'None';
		$this->NewAuthClientAddError->Display = 'None';
		$this->NewAuthClientAddExists->Display = 'None';

		$result = false;
		$exists = false;
		$config = $this->getModule('api_config')->getConfig();
		if ($this->getAuthType() === 'basic') {
			$users = $this->getModule('basic_apiuser')->getAllUsers();
			if (!key_exists($this->APIBasicLogin->Text, $users)) {
				$result = $this->getModule('basic_apiuser')->setUsersConfig(
					$this->APIBasicLogin->Text,
					$this->APIBasicPassword->Text
				);
			} else {
				$exists = true;
			}
		} elseif ($this->getAuthType() === 'oauth2') {
			$oauth2_cfg = $this->getModule('oauth2_config')->getConfig();
			if (!key_exists($this->APIOAuth2ClientId->Text, $oauth2_cfg)) {
				$oauth2_cfg[$this->APIOAuth2ClientId->Text] = array();
				$oauth2_cfg[$this->APIOAuth2ClientId->Text]['client_id'] = $this->APIOAuth2ClientId->Text;
				$oauth2_cfg[$this->APIOAuth2ClientId->Text]['client_secret'] = $this->APIOAuth2ClientSecret->Text;
				$oauth2_cfg[$this->APIOAuth2ClientId->Text]['redirect_uri'] = $this->APIOAuth2RedirectURI->Text;
				$oauth2_cfg[$this->APIOAuth2ClientId->Text]['scope'] = $this->APIOAuth2Scope->Text;
				$oauth2_cfg[$this->APIOAuth2ClientId->Text]['bconsole_cfg_path'] = $this->APIOAuth2BconsoleCfgPath->Text;
				$oauth2_cfg[$this->APIOAuth2ClientId->Text]['name'] = $this->APIOAuth2Name->Text;
				$result = $this->getModule('oauth2_config')->setConfig($oauth2_cfg);
			} else {
				$exists = true;
			}
		}

		if ($exists === true) {
			$this->NewAuthClientAddExists->Display = 'Dynamic';
		} elseif ($result === true) {
			$this->NewAuthClientAddOk->Display = 'Dynamic';
		} else {
			$this->NewAuthClientAddError->Display = 'Dynamic';
		}
		$this->onCallback($param);
	}

	public function setShowButtons($show) {
		$show = TPropertyValue::ensureBoolean($show);
		$this->show_buttons = $show;
	}

	public function getShowButtons() {
		return $this->show_buttons;
	}

	public function setAuthType($auth_type) {
		if (in_array($auth_type, $this->auth_types)) {
			$this->auth_type = $auth_type;
		}
	}

	public function getAuthType() {
		return $this->auth_type;
	}

	public function onCallback($param) {
		$this->raiseEvent('OnCallback', $this, $param);
	}

	public function bubbleEvent($sender, $param) {
		if ($param instanceof TCommandEventParameter) {
			$this->raiseBubbleEvent($this,$param);
			return true;
		} else {
			return false;
		}
	}
}
