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

Prado::using('Application.Web.Class.WebModule');

/**
 * Web LDAP user manager module.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Module
 * @package Baculum Web
 */
class WebLdapUserManager extends WebModule implements UserManager {

	private $ldap = null;

	public function init($config) {
		parent::init($config);
		$web_config = $this->getModule('web_config')->getConfig();
		if (key_exists('auth_ldap', $web_config)) {
			$this->ldap = $this->getModule('ldap');
			$this->ldap->setParams($web_config['auth_ldap']);
		}
	}

	public function validateUser($username, $password) {
		return $this->ldap->login($username, $password);
	}
}
?>
