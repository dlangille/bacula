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

session_start();

Prado::using('Application.Web.Pages.Requirements');
Prado::using('Application.Common.Class.BaculumPage');
Prado::using('Application.Web.Init');
Prado::using('Application.Web.Class.WebConfig');

class BaculumWebPage extends BaculumPage {

	/**
	 * It is first application user pre-defined for first login.
	 * It is removed just after setup application.
	 */
	const DEFAULT_AUTH_USER = 'admin';

	private $config = array();

	public function onPreInit($param) {
		parent::onPreInit($param);
		$this->config = $this->getModule('web_config')->getConfig();
		$this->Application->getGlobalization()->Culture = $this->getLanguage();
		if (count($this->config) === 0) {
			if (isset($_SERVER['PHP_AUTH_USER'])) {
				if ($this->Service->getRequestedPagePath() != 'WebConfigWizard') {
					$this->goToPage('WebConfigWizard');
				}
				// without config there is no way to call api below
				return;
			} else {
				self::accessDenied();
			}
		}
		Logging::$debug_enabled = (isset($this->config['baculum']['debug']) && $this->config['baculum']['debug'] == 1);
		if (!$this->IsPostBack && !$this->IsCallBack) {
			$this->getModule('api')->initSessionCache(true);
			$this->setSessionUserVars();
		}
		$this->checkPrivileges();
	}

	/**
	 * Get curently set language short name (for example: en, pl).
	 * If language short name is not set in session then the language value
	 * is taken from Baculum config file, saved in session and returned.
	 * If the language setting is set in session, then the value from
	 * session is returned.
	 *
	 * @access public
	 * @return string currently set language short name
	 */
	public function getLanguage() {
		$language = null;
		if (isset($_SESSION['language']) && !empty($_SESSION['language'])) {
			$language =  $_SESSION['language'];
		} else {
			if (isset($this->config['baculum']) && key_exists('lang', $this->config['baculum'])) {
				$language = $this->config['baculum']['lang'];
			}
			if (is_null($language)) {
				$language = WebConfig::DEFAULT_LANGUAGE;
			}
			$_SESSION['language'] = $language;
		}
		return $language;
	}

	/**
	 * Set page session values.
	 *
	 * @return none
	 */
	private function setSessionUserVars() {
		// NOTE. For oauth2 callback, the PHP_AUTH_USER is empty because no user/pass.
		if (count($this->config) > 0 && isset($_SERVER['PHP_AUTH_USER'])) {
			// Set administrator role
			$_SESSION['admin'] = ($_SERVER['PHP_AUTH_USER'] === $this->config['baculum']['login']);

			// Set api host for normal user
			if (!$_SESSION['admin'] && key_exists('users', $this->config) && array_key_exists($_SERVER['PHP_AUTH_USER'], $this->config['users'])) {
				$_SESSION['api_host'] = $this->config['users'][$_SERVER['PHP_AUTH_USER']];
			} elseif ($_SESSION['admin']) {
				$_SESSION['api_host'] = 'Main';
			}
		} else {
			$_SESSION['admin'] = false;
		}

		// Set director
		$directors = $this->getModule('api')->get(array('directors'), null, false);
		if ($directors->error === 0 && count($directors->output) > 0 && (!key_exists('director', $_SESSION) || $directors->output[0] != $_SESSION['director'])) {
			$_SESSION['director'] = $directors->output[0];
		}

		// Set config main component names
		$config = $this->getModule('api')->get(array('config'), null, false);
		$_SESSION['dir'] = $_SESSION['sd'] = $_SESSION['fd'] = $_SESSION['bcons'] = '';
		if ($config->error === 0) {
			for ($i = 0; $i < count($config->output); $i++) {
				$component = (array)$config->output[$i];
				if (key_exists('component_type', $component) && key_exists('component_name', $component)) {
					$_SESSION[$component['component_type']] = $component['component_name'];
				}
			}
		}
	}

	private function checkPrivileges() {
		if (property_exists($this, 'admin') && $this->admin === true && !$_SESSION['admin']) {
			self::accessDenied();
		}
	}

	public static function accessDenied() {
		die('Access denied');
	}
}
?>
