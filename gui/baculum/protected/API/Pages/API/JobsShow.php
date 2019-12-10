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

/**
 * Show jobs command endpoint.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category API
 * @package Baculum API
 */
class JobsShow extends BaculumAPIServer {

	public function get() {
		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			array('.jobs')
		);
		$job = null;
		if ($result->exitcode === 0) {
			array_shift($result->output);
			if ($this->Request->contains('name')) {
				if (in_array($this->Request['name'], $result->output)) {
					$job = $this->Request['name'];
				} else {
					$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
					$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
					return;
				}
			}
		} else {
			$this->output = $result->output;
			$this->error = $result->exitcode;
			return;
		}
		$cmd = array('show');
		if (is_string($job)) {
			$cmd[] = 'job="' . $job . '"';
		} else {
			$cmd[] = 'jobs';
		}
		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			$cmd
		);
		$this->output = $result->output;
		$this->error = $result->exitcode;
	}
}

?>
