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
 
class JobTasks extends BaculumAPI {
	public function get() {
		$limit = intval($this->Request['limit']);
		$directors = $this->getModule('bconsole')->getDirectors();
		if($directors->exitcode === 0) {
			$jobs = array();
			$error = false;
			$error_obj = null;
			for($i = 0; $i < count($directors->output); $i++) {
				$jobsList = $this->getModule('bconsole')->bconsoleCommand($directors->output[$i], array('.jobs'), $this->user);
				if ($jobsList->exitcode != 0) {
					$error_obj = $jobsList;
					$error = true;
					break;
				}
				$jobsshow = $this->getModule('bconsole')->bconsoleCommand($directors->output[$i], array('show', 'jobs'), $this->user);
				if ($jobsshow->exitcode != 0) {
					$error_obj = $jobsshow;
					$error = true;
					break;
				}
				$jobs[$directors->output[$i]] = array();
				for($j = 0; $j < count($jobsList->output); $j++) {
					/**
					 * Checking by "show job" command is ugly way to be sure that is reading jobname but not some 
					 * random output (eg. "You have messages." or debugging).
					 * For now I did not find nothing better for be sure that output contains job.
					 */
					for($k = 0; $k < count($jobsshow->output); $k++) {
						if(preg_match('/^Job: name=' . $jobsList->output[$j] . '.*/', $jobsshow->output[$k]) === 1) {
							$jobs[$directors->output[$i]][] = $jobsList->output[$j];
							break;
						}
					}
					// limit per director, not for entire elements
					if($limit > 0 && count($jobs[$directors->output[$i]]) === $limit) {
						break;
					}
				}
			}
			if ($error === true) {
				$this->output = $error_obj->output;
				$this->error = $error_obj->exitcode;
			} else {
				$this->output = $jobs;
				$this->error =  BconsoleError::ERROR_NO_ERRORS;
			}
		} else {
			$this->output = $directors->output;
			$this->error = $directors->exitcode;
		}
	}
}

?>
