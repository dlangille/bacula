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
 
class Jobs extends BaculumAPIServer {
	public function get() {
		$limit = $this->Request->contains('limit') ? intval($this->Request['limit']) : 0;
		$jobstatus = $this->Request->contains('jobstatus') ? $this->Request['jobstatus'] : '';
		$misc = $this->getModule('misc');
		$jobname = $this->Request->contains('name') && $misc->isValidName($this->Request['name']) ? $this->Request['name'] : '';
		$params = array();
		$jobstatuses = array_keys($misc->getJobState());
		$sts = str_split($jobstatus);
		for ($i = 0; $i < count($sts); $i++) {
			if (in_array($sts[$i], $jobstatuses)) {
				if (!key_exists('jobstatus', $params)) {
					$params['jobstatus'] = array('operator' => 'OR', 'vals' => array());
				}
				$params['jobstatus']['vals'][] = $sts[$i];
			}
		}
		$allowed = array();
		$result = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.jobs'));
		if ($result->exitcode === 0) {
			array_shift($result->output);
			$vals = array();
			if (!empty($jobname) && in_array($jobname, $result->output)) {
				$vals = array($jobname);
			} else {
				$vals = $result->output;
			}
			$params['name']['operator'] = 'OR';
			$params['name']['vals'] = $vals;
			$jobs = $this->getModule('job')->getJobs($limit, $params);
			$this->output = $jobs;
			$this->error = JobError::ERROR_NO_ERRORS;
		} else {
			$this->output = $result->output;
			$this->error = $result->exitcode;
		}
	}
}
?>
