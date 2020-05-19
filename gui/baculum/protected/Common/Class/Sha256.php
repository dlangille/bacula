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

Prado::using('Application.Common.Class.CommonModule');

/**
 * Cryptographic SHA-256 hashing function module
 * Module is responsible for providing SHA-256 support.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Module
 * @package Baculum Common
 */
class Sha256 extends CommonModule {

	const SHA256_ROUNDS = 10000;

	/**
	 * Get hashed password using SHA-256 algorithm and salt.
	 *
	 * @param string $password plain text password
	 * @return string hashed password
	 */
	public function crypt($password) {
		// Salt string  - 16 characters for SHA-256
		$salt_str = $this->getModule('crypto')->getRandomString(16);

		$salt = sprintf(
			'$5$rounds=%d$%s$',
			self::SHA256_ROUNDS,
			$salt_str
		);
		return crypt($password, $salt);
	}
}
?>
