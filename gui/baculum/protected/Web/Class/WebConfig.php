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

Prado::using('Application.Common.Class.ConfigFileModule');

/**
 * Manage webGUI configuration.
 * Module is responsible for get/set webGUI config data.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Module
 * @package Baculum Web
 */
class WebConfig extends ConfigFileModule {

	/**
	 * Web config file path
	 */
	const CONFIG_FILE_PATH = 'Application.Web.Config.settings';

	/**
	 * Web config file format
	 */
	const CONFIG_FILE_FORMAT = 'ini';

	/**
	 * Default application language
	 */
	const DEFAULT_LANGUAGE = 'en';

	/**
	 * These options are obligatory for web config.
	 */
	private $required_options = array(
		'baculum' => array('login', 'debug', 'lang')
	);

	/**
	 * Get (read) web config.
	 *
	 * @access public
	 * @param string $section config section name
	 * @return array config
	 */
	public function getConfig($section = null) {
		$config = $this->readConfig(self::CONFIG_FILE_PATH, self::CONFIG_FILE_FORMAT);
		if ($this->validateConfig($config) === true) {
			if (!is_null($section)) {
				$config = array_key_exists($section, $this->required_options) ? $config[$section] : array();
			}
		} else {
			$config = array();
		}
		return $config;
	}

	/**
	 * Set (save) web config.
	 *
	 * @access public
	 * @param array $config config
	 * @return boolean true if config saved successfully, otherwise false
	 */
	public function setConfig(array $config) {
		$result = false;
		if ($this->validateConfig($config) === true) {
			$result = $this->writeConfig($config, self::CONFIG_FILE_PATH, self::CONFIG_FILE_FORMAT);
		}
		return $result;
	}
	
	/**
	 * Validate web config.
	 * Config validation should be used as early as config data is available.
	 * Validation is done in read/write config methods.
	 *
	 * @access private
	 * @param array $config config
	 * @return boolean true if config valid, otherwise false
	 */
	private function validateConfig(array $config = array()) {
		$is_valid = $this->isConfigValid($this->required_options, $config, self::CONFIG_FILE_FORMAT, self::CONFIG_FILE_PATH);
		return $is_valid;
	}

	/**
	 * Get web administrator name.
	 * Web interface supports one admin and many normal users.
	 *
	 * @return string web admin name
	 */
	public function getWebAdmin() {
		$config = $this->getConfig();
		$web_admin = $config['baculum']['login'];
		return $web_admin;
	}

	/**
	 * Get application language short name.
	 * If no language set then returned is default language.
	 *
	 * @access public
	 * @return string lanuage short name
	 */
	public function getLanguage() {
		$language = null;
		$config = $this->getConfig();
		if (array_key_exists('baculum', $config) && array_key_exists('lang', $config['baculum'])) {
			$language = $config['baculum']['lang'];
		}
		if (is_null($language)) {
			$language = self::DEFAULT_LANGUAGE;
		}
		return $language;
	}
}
?>
