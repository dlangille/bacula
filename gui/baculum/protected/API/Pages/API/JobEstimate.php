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
 
class JobEstimate extends BaculumAPIServer {

	public function create($params) {
		$job = null;
		if (property_exists($params, 'id')) {
			$jobid = intval($params->id);
			$job_row = $this->getModule('job')->getJobById($jobid);
			$job = is_object($job_row) ? $job_row->name : null;
		} elseif (property_exists($params, 'name') && $this->getModule('misc')->isValidName($params->name)) {
			$job = $params->name;
		}
		$level = null;
		if (property_exists($params, 'level')) {
			$level = $params->level;
		}
		$fileset = null;
		if (property_exists($params, 'fileset') && $this->getModule('misc')->isValidName($params->fileset)) {
			$fileset = $params->fileset;
		}
		$client = null;
		if (property_exists($params, 'clientid')) {
			$clientid = intval($params->clientid);
			$client_row = $this->getModule('client')->getClientById($clientid);
			$client = is_object($client_row) ? $client_row->name : null;
		} elseif (property_exists($params, 'client') && $this->getModule('misc')->isValidName($params->client)) {
			$client = $params->client;
		}
		$accurate = null;
		if (property_exists($params, 'accurate')) {
			$accurate_job = intval($params->accurate);
			$accurate = $accurate_job === 1 ? 'yes' : 'no';
		}

		if (is_null($job)) {
			$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
			$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
			return;
		}

		$is_valid_level = $this->getModule('misc')->isValidJobLevel($params->level);
		if (!$is_valid_level) {
			$this->output = JobError::MSG_ERROR_INVALID_JOBLEVEL;
			$this->error = JobError::ERROR_INVALID_JOBLEVEL;
			return;
		}

		if (is_null($fileset)) {
			$this->output = JobError::MSG_ERROR_FILESET_DOES_NOT_EXISTS;
			$this->error = JobError::ERROR_FILESET_DOES_NOT_EXISTS;
			return;
		}

		if (is_null($client)) {
			$this->output = JobError::MSG_ERROR_CLIENT_DOES_NOT_EXISTS;
			$this->error = JobError::ERROR_CLIENT_DOES_NOT_EXISTS;
			return;
		}

		$joblevels  = $this->getModule('misc')->getJobLevels();
		$cmd = array(
			'estimate',
			'job="' . $job . '"',
			'level="' . $joblevels[$level] . '"',
			'fileset="' . $fileset. '"',
			'client="' . $client . '"',
			'accurate="' . $accurate . '"'
		);
		$result = $this->getModule('bconsole')->bconsoleCommand($this->director, $cmd);
		$this->output = $result->output;
		$this->error = $result->exitcode;
	}
}

?>
