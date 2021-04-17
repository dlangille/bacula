<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2021 Kern Sibbald
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


/**
 * Web user module.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Module
 * @package Baculum Web
 */
class WebUser extends TUser {

	/**
	 * Saved in session user properties.
	 */
	const LONG_NAME = 'LongName';
	const EMAIL = 'Email';
	const DESCRIPTION = 'Description';
	const API_HOSTS = 'ApiHosts';
	const DEFAULT_API_HOST = 'DefaultApiHost';
	const IPS = 'Ips';
	const ENABLED = 'Enabled';
	const IN_CONFIG = 'InConfig';

	/**
	 * Create single user instance.
	 * Used for authenticated users.
	 * If user doesn't exist in configuration then default access values can be taken into accout.
	 *
	 * @param string username user name
	 * @return WebUser user instance
	 */
	public function createUser($username) {
		$user = Prado::createComponent(__CLASS__, $this->getManager());
		$user->setUsername($username);

		$application = $this->getManager()->getApplication();
		$user_config = $application->getModule('user_config')->getUserConfig($username);
		$web_config = $application->getModule('web_config')->getConfig();

		if (count($user_config) > 0) {
			// User exists in Baculum Web users database
			$user->setInConfig(true);
			$user->setDescription($user_config['description']);
			$user->setLongName($user_config['long_name']);
			$user->setEmail($user_config['email']);
			$user->setRoles($user_config['roles']);
			$user->setAPIHosts($user_config['api_hosts']);
			$user->setIps($user_config['ips']);
			$user->setEnabled($user_config['enabled']);
		} elseif (isset($web_config['security']['def_access'])) {
			// User doesn't exist. Check if user can have access.
			$user->setInConfig(false);
			if ($web_config['security']['def_access'] === WebConfig::DEF_ACCESS_NO_ACCESS) {
				// no access, nothing to do
			} elseif ($web_config['security']['def_access'] === WebConfig::DEF_ACCESS_DEFAULT_SETTINGS) {
				if (isset($web_config['security']['def_role'])) {
					$user->setRoles($web_config['security']['def_role']);
				}
				if (isset($web_config['security']['def_api_host'])) {
					$user->setAPIHosts($web_config['security']['def_api_host']);
				}
			}
		}
		return $user;
	}

	/**
	 * Username setter.
	 *
	 * @param string $username user name
	 * @return none
	 */
	public function setUsername($username) {
		$this->setName($username);
	}

	/**
	 * Username getter.
	 *
	 * @return string user name
	 */
	public function getUsername() {
		return $this->getName();
	}

	/**
	 * Long name setter.
	 *
	 * @param string $long_name long name
	 * @return none
	 */
	public function setLongName($long_name) {
		$this->setState(self::LONG_NAME, $long_name);
	}

	/**
	 * Long name getter.
	 *
	 * @return string long name (default empty string)
	 */
	public function getLongName() {
		return $this->getState(self::LONG_NAME, '');
	}

	/**
	 * E-mail address setter.
	 *
	 * @param string $email e-mail address
	 * @return none
	 */
	public function setEmail($email) {
		$this->setState(self::EMAIL, $email);
	}

	/**
	 * E-mail address getter.
	 *
	 * @return string e-mail address
	 */
	public function getEmail() {
		return $this->getState(self::EMAIL, '');
	}

	/**
	 * Description setter.
	 *
	 * @param string $desc description
	 * @return none
	 */
	public function setDescription($desc) {
		$this->setState(self::DESCRIPTION, $desc);
	}

	/**
	 * Description getter.
	 *
	 * @return string description
	 */
	public function getDescription() {
		return $this->getState(self::DESCRIPTION, '');
	}

	/**
	 * Set API hosts.
	 *
	 * @param array $api_hosts user API hosts
	 * @return none
	 */
	public function setAPIHosts($api_hosts) {
		$this->setState(self::API_HOSTS, $api_hosts);
	}

	/**
	 * API hosts getter.
	 *
	 * @return array user API hosts
	 */
	public function getAPIHosts() {
		$api_hosts = [];
		$hosts = $this->getState(self::API_HOSTS);
		/**
		 * This checking is for backward compatibility because previously
		 * hosts were written in session as string. Now it is written as array.
		 */
		if (is_string($hosts)) {
			if (!empty($hosts)) {
				$hosts = explode(',', $hosts);
			} else {
				$hosts = [];
			}
		} elseif (is_null($hosts)) {
			$hosts = [];
		}

		if (count($hosts) > 0) {
			$api_hosts = $hosts;
		} else {
			// add default API host
			$api_hosts[] = HostConfig::MAIN_CATALOG_HOST;
		}
		return $api_hosts;
	}

	/**
	 * Set default API host for user.
	 * It determines which host will be used as default API host to login
	 * to Baculum Web interface. This host needs to have at least the catalog
	 * and the console capabilities.
	 *
	 * @param string $api_host default API host
	 * @return none
	 */
	public function setDefaultAPIHost($api_host) {
		$this->setState(self::DEFAULT_API_HOST, $api_host);
		$application = $this->getManager()->getApplication();
		$application->getModule('auth')->updateSessionUser($this);
	}

	/**
	 * Get default API host for user.
	 * If default API host is not set, there happens a try to determine
	 * this host if user has only one API host assigned.
	 *
	 * @return string|null default API host or null if no default host set
	 */
	public function getDefaultAPIHost() {
		$def_host = $this->getState(self::DEFAULT_API_HOST);
		if (!$def_host) {
			$api_hosts = $this->getAPIHosts();
			if (count($api_hosts) == 1) {
				// only one host assigned, so use it as default host
				$def_host = $api_hosts[0];
				$this->setDefaultAPIHost($def_host);
			}
		}
		return $def_host;
	}

	/**
	 * Check if given API host belongs to user API hosts.
	 *
	 * @param string $api_host API host to check
	 * @return boolean true if API host belongs to user API hosts, otherwise false
	 */
	public function isUserAPIHost($api_host) {
		$api_hosts = $this->getAPIHosts();
		return in_array($api_host, $api_hosts);
	}

	/**
	 * IP address restriction setter.
	 *
	 * @param string $ips comma separated IP addresses
	 * @return none
	 */
	public function setIps($ips) {
		$this->setState(self::IPS, $ips);
	}

	/**
	 * IP address restriction getter.
	 *
	 * @return string comma separated IP address list (default empty string)
	 */
	public function getIps() {
		return $this->getState(self::IPS, '');
	}

	/**
	 * Enabled setter
	 *
	 * @param boolean $enabled enabled flag state
	 * @return none
	 */
	public function setEnabled($enabled) {
		$enabled = TPropertyValue::ensureBoolean($enabled);
		$this->setState(self::ENABLED, $enabled);
	}

	/**
	 * Enabled getter.
	 *
	 * @return string enabled flag state (default false)
	 */
	public function getEnabled() {
		return $this->getState(self::ENABLED, false);
	}

	/**
	 * Set if user exists in configuration file.
	 *
	 * @param boolean $in_config in config state value
	 * @return none
	 */
	public function setInConfig($in_config) {
		$in_config = TPropertyValue::ensureBoolean($in_config);
		$this->setState(self::IN_CONFIG, $in_config);
	}

	/**
	 * In config getter.
	 *
	 * @return string in config state value (default false)
	 */
	public function getInConfig() {
		return $this->getState(self::IN_CONFIG, false);
	}
}
?>
