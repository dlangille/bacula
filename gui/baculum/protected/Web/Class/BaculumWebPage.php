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

session_start();

Prado::using('Application.Common.Class.BaculumPage');
Prado::using('Application.Web.Class.WebConfig');

class BaculumWebPage extends BaculumPage {

	private $config;

	public function onPreInit($param) {
		parent::onPreInit($param);
		$this->config = $this->getModule('web_config')->getConfig('baculum');
		Logging::$debug_enabled = (array_key_exists('debug', $this->config) && $this->config['debug'] == 1);
		$this->Application->getGlobalization()->Culture = $this->getLanguage();
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
			if (array_key_exists('lang', $this->config)) {
				$language = $this->config['lang'];
			}
			if (is_null($language)) {
				$language = WebConfig::DEFAULT_LANGUAGE;
			}
			$_SESSION['language'] = $language;
		}
		return $language;
	}

}
?>
