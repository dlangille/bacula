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

Prado::using('System.Exceptions.TException');

class BException extends TException {

	private $error_code;
	private $error_message;

	public function __construct($error_message, $error_code) {
		$this->setErrorMessage($error_message);
		$this->setErrorCode($error_code);
		parent::__construct($error_message);
	}

	public function getErrorCode() {
		return $this->error_code;
	}

	public function setErrorCode($error_code) {
		$this->error_code = $error_code;
	}

	public function getErrorMessage() {
		return $this->error_message;
	}

	public function setErrorMessage($error_message) {
		$this->error_message = $error_message;
	}

	public function __toString() {
		$msg = sprintf('Error: %d, Message: %s', $this->getErrorCode(), $this->getErrorMessage());
		return $msg;
	}
}

class BCatalogException extends BException {
}

class BConsoleException extends BException {
}

class BConfigException extends BException {
}
