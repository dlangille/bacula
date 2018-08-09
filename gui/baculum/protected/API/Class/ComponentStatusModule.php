<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2018 Kern Sibbald
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

Prado::using('Application.API.Class.APIModule');

/**
 * Base abstract class to inherit commonly used method
 * in work with component status.
 */
abstract class ComponentStatusModule extends APIModule {

	/**
	 * Method to get parsed status ready to show in API results.
	 *
	 * @param array $output array with parsed component status values
	 */
	abstract public function getStatus(array $output);

	/**
	 * Parse single component status line to find key=value pairs.
	 *
	 * @param string $line single line from component status
	 * @return mixed array with key and value on success, otherwise null
	 */
	protected function parseLine($line) {
		$ret = null;
		if (preg_match('/^(?P<key>\w+)=(?P<value>[\S\s]*)$/', $line, $match) === 1) {
			$ret = $match;
		}
		return $ret;
	}
}
?>
