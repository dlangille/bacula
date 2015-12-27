<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2015 Marcin Haba
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

class JobShow extends BaculumAPI {
	public function get() {
		$jobname = null;
		$error = false;
		$error_obj = null;

		if (isset($this->Request['id'])) {
			$jobid = intval($this->Request['id']);
			$job = $this->getModule('job')->getJobById($jobid);
			$jobname = property_exists($job, 'name') ? $job->name : null;
		} elseif (isset($this->Request['name'])) {
			$allowedJobs = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.jobs'), $this->user);
			if ($allowedJobs->exitcode === 0) {
				$jobname = in_array($this->Request['name'], $allowedJobs->output) ? $this->Request['name'] : null;
			} else {
				$error_obj = $allowedJobs;
				$error = true;
			}
		}

		if ($error === false) {
			if(!is_null($jobname)) {
				$jobShow = $this->getModule('bconsole')->bconsoleCommand($this->director, array('show', 'job="' . $jobname . '"'), $this->user);
				$this->output = $jobShow->output;
				$this->error = (integer)$jobShow->exitcode;
			} else {
				$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
				$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
			}
		} else {
			$this->output = $error_obj->output;
			$this->error = $error_obj->exitcode;
		}
	}
}

?>
