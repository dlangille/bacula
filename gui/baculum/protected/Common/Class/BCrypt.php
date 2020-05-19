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
 * Cryptographic BCrypt hashing function module
 * Module is responsible for providing BCrypt support.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Module
 * @package Baculum Common
 */
class BCrypt extends CommonModule {

	// bcrypt uses not standard base64 alphabet
	const BCRYPT_BASE64_CODE = './ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';

	// bcrypt cost parameter - number of iterations during hashing
	const BCRYPT_COST = 10;

	/**
	 * Get hashed password using BCrypt algorithm and salt.
	 *
	 * @param string $password plain text password
	 * @return string hashed password
	 */
	public function crypt($password) {
		// Suffle string
		$rand_string = str_shuffle(self::BCRYPT_BASE64_CODE);

		// BCrypt salt - 22 characters
		$salt_str = substr($rand_string, 0, 22);

		$salt = sprintf(
			'$2y$%d$%s$',
			self::BCRYPT_COST,
			$salt_str
		);
		return crypt($password, $salt);
	}
}
?>
