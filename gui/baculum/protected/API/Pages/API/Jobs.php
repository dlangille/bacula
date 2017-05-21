<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2016 Kern Sibbald
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
 
class Jobs extends BaculumAPIServer {
	public function get() {
		$limit = intval($this->Request['limit']);
		$allowed = array();
		$error = false;
		if (!is_null($this->user)) {
			$allowedJobs = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.jobs'), $this->user);
			if ($allowedJobs->exitcode === 0) {
				array_shift($allowedJobs->output);
				$allowed = $allowedJobs->output;
			} else {
				$error = true;
				$this->output = $allowedJobs->output;
				$this->error = $allowedJobs->exitcode;
			}
		}

		if ($error === false) {
			$jobs = $this->getModule('job')->getJobs($limit, $allowed);
			$this->output = $jobs;
			$this->error = JobError::ERROR_NO_ERRORS;
		}
	}
}
?>
