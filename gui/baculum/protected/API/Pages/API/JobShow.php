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

class JobShow extends BaculumAPIServer {
	public function get() {
		$jobname = null;
		$error = false;
		$error_obj = null;

		if ($this->Request->contains('id')) {
			$jobid = intval($this->Request['id']);
			$job = $this->getModule('job')->getJobById($jobid);
			$jobname = property_exists($job, 'name') ? $job->name : null;
		} elseif ($this->Request->contains('name')) {
			$result = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.jobs'));
			if ($result->exitcode === 0) {
				array_shift($result->output);
				$jobname = in_array($this->Request['name'], $result->output) ? $this->Request['name'] : null;
			} else {
				$error_obj = $result;
				$error = true;
			}
		}

		if ($error === false) {
			if(is_string($jobname)) {
				$job_show = $this->getModule('bconsole')->bconsoleCommand(
					$this->director,
					array('show', 'job="' . $jobname . '"')
				);
				$this->output = $job_show->output;
				$this->error = $job_show->exitcode;
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
