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

Prado::using('Application.API.Class.Bconsole');

class ScheduleStatus extends BaculumAPIServer {

	public function get() {
		$misc = $this->getModule('misc');
		$cmd = array('status', 'schedule');
		if ($this->Request->contains('job') && $misc->isValidName($this->Request['job'])) {
			$cmd[] = 'job="' . $this->Request['job'] . '"';
		}
		if ($this->Request->contains('client') && $misc->isValidName($this->Request['client'])) {
			$cmd[] = 'client="' . $this->Request['client'] . '"';
		}
		if ($this->Request->contains('schedule') && $misc->isValidName($this->Request['schedule'])) {
			$cmd[] = 'schedule="' . $this->Request['schedule'] . '"';
		}
		if ($this->Request->contains('days') && $misc->isValidInteger($this->Request['days'])) {
			$cmd[] = 'days="' . $this->Request['days'] . '"';
		}
		if ($this->Request->contains('limit') && $misc->isValidInteger($this->Request['limit'])) {
			$cmd[] = 'limit="' . $this->Request['limit'] . '"';
		}
		if ($this->Request->contains('time') && $misc->isValidBDateAndTime($this->Request['time'])) {
			$cmd[] = 'time="' . $this->Request['time'] . '"';
		}

		$result = $this->getModule('bconsole')->bconsoleCommand($this->director, $cmd, Bconsole::PTYPE_API_CMD);
		if ($result->exitcode === 0) {
			array_shift($result->output);
			$this->output = $this->formatSchedules($result->output);
			$this->error = PoolError::ERROR_NO_ERRORS;
		} else {
			$this->output = $result->output;
			$this->error = $result->exitcode;
		}
	}

	private function formatSchedules(array $output) {
		$items = $item = array();
		for ($i = 0; $i < count($output); $i++) {
			if (preg_match('/^(?P<key>\w+)=(?P<val>[\s\S]*)$/', $output[$i], $match) === 1) {
				$item[$match['key']] = $match['val'];
			} elseif (empty($output[$i]) && count($item) > 0) {
				$items[] = $item;
				$item = array();
			}
		}
		return $items;
	}
}
?>
