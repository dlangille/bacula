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

Prado::using('Application.API.Class.ComponentStatusModule');

/**
 * Module used to parse and prepare storage status output.
 */
class StatusStorage extends ComponentStatusModule {

	/**
	 * Get director status by raw bconsole status director output.
	 *
	 * @param array $output bconsole status director output
	 * @return array array with parsed storage status values
	 */
	public function getStatus(array $output) {
		$result = array();
		$line = null;
		$opts = array();
		$header = false;
		for($i = 0; $i < count($output); $i++) {
			if (empty($output[$i])) {
				if  (count($opts) > 10) {
					$result[] = $opts;
				}
				if (count($opts) > 0) {
					$opts = array();
				}
			} elseif ($output[$i] === 'header:') {
				$header = true;
			} else {
				$line = $this->parseLine($output[$i]);
				if (is_array($line)) {
					$opts[$line['key']] = $line['value'];
				}
			}
		}
		if ($header) {
			// header is only one so get it without using list
			$result = array_pop($result);
		}
		return $result;
	}
}
?>
