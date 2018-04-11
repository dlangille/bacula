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
 
class JobTasks extends BaculumAPIServer {
	public function get() {
		$limit = $this->Request->contains('limit') ? intval($this->Request['limit']) : 0;
		$jobs_cmd = array('.jobs');
		if ($this->Request->contains('type') && array_key_exists($this->Request['type'], $this->getModule('misc')->job_types)) {
			array_push($jobs_cmd, 'type="' . $this->Request['type']. '"');
		}

		$directors = $this->getModule('bconsole')->getDirectors();
		if($directors->exitcode != 0) {
			$this->output = $directors->output;
			$this->error = $directors->exitcode;
			return;
		}
		$jobs = array();
		$error = false;
		$error_obj = null;
		for($i = 0; $i < count($directors->output); $i++) {
			$job_list = $this->getModule('bconsole')->bconsoleCommand($directors->output[$i], $jobs_cmd);
			if ($job_list->exitcode != 0) {
				$error_obj = $job_list;
				$error = true;
				break;
			}
			$jobs_show = $this->getModule('bconsole')->bconsoleCommand($directors->output[$i], array('show', 'jobs'));
			if ($jobs_show->exitcode != 0) {
				$error_obj = $jobs_show;
				$error = true;
				break;
			}
			$jobs[$directors->output[$i]] = array();
			for($j = 0; $j < count($job_list->output); $j++) {
				/**
				 * Checking by "show job" command is ugly way to be sure that is reading jobname but not some 
				 * random output (eg. "You have messages." or debugging).
				 * For now I did not find nothing better for be sure that output contains job.
				 * @NOTE, now is used "gui on" so it is not necessarly @TODO: Rework it
				 */
				for($k = 0; $k < count($jobs_show->output); $k++) {
					if(preg_match('/^Job: name=' . $job_list->output[$j] . '.*/', $jobs_show->output[$k]) === 1) {
						$jobs[$directors->output[$i]][] = $job_list->output[$j];
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
	}
}

?>
