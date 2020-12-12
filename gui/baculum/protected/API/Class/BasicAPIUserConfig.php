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

Prado::using('Application.Common.Class.BasicUserConfig');

/**
 * Manage HTTP Basic auth method users.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Config
 * @package Baculum API
 */
class BasicAPIUserConfig extends BasicUserConfig {

	/**
	 * Users login and password file for HTTP Basic auth.
	 */
	const USERS_FILE_NAME = 'Application.API.Config.baculum';
	const USERS_FILE_EXTENSION = '.users';

	public function getConfigPath() {
		// First check if custom config path is set, if not, then use default users file
		return parent::getConfigPath() ?: Prado::getPathOfNamespace(self::USERS_FILE_NAME, self::USERS_FILE_EXTENSION);
	}
}
