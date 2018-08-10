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
 * Module used to parse and prepare director status output.
 */
class StatusDirector extends ComponentStatusModule {

	/**
	 * Get director status by raw bconsole status director output.
	 *
	 * @param array $output bconsole status director output
	 * @return array array with parsed director status values
	 */
	public function getStatus(array $output) {
		$result = array();
		$section = null;
		$line = null;
		$sections = array('header:', 'running:', 'terminated:');
		$opts = array();
		for($i = 0; $i < count($output); $i++) {
			if (in_array($output[$i], $sections)) { // check if section
				$section = rtrim($output[$i], ':');
			} elseif ($section === 'header' && count($opts) == 0 && $output[$i] === '') {
				/**
				 * special treating 'scheduled' section because this section
				 * is missing in the api status dir output.
				 */
				$section = 'scheduled';
			} elseif (!empty($section)) {
				$line = $this->parseLine($output[$i]);
				if (is_array($line)) { // check if line
					if (!key_exists($section, $result)) {
						$result[$section] = array();
						continue;
					}
					$opts[$line['key']] = $line['value'];
				} elseif (count($opts) > 0) { // dump all parameters
					$result[$section][] = $opts;
					$opts = array();
				}
			}
		}
		return $result;
	}
}
?>
