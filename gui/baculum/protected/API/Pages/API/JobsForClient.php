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
 
class JobsForClient extends BaculumAPIServer {

	public function get() {
		$allowed_jobs = array();
		$clientid = $this->Request->contains('id') ? intval($this->Request['id']) : 0;
		$error = false;
		$result = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.jobs'));
		if ($result->exitcode === 0) {
			$allowed_jobs = $result->output;
		} else {
			$error = true;
			$this->output = $result->output;
			$this->error = $result->exitcode;
		}

		if ($error === false) {
			$jobs = $this->getModule('job')->getJobsForClient($clientid, $allowed_jobs);
			$this->output = $jobs;
			$this->error = JobError::ERROR_NO_ERRORS;
		}
	}
}
?>
