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
 
class JobLog extends BaculumAPIServer {
	public function get() {
		$jobid = $this->Request->contains('id') ? intval($this->Request['id']) : 0;
		$job = $this->getModule('job')->getJobById($jobid);
		if(is_object($job)) {
			$log = $this->getModule('joblog')->getLogByJobId($job->jobid);
			$log = array_map('trim', $log);
			// Output may contain national characters.
			$this->output = array_map('utf8_encode', $log);
			$this->error = JobError::ERROR_NO_ERRORS;
		} else {
			$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
			$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
		}
	}
}

?>
