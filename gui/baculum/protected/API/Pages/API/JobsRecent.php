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
 
class JobsRecent extends BaculumAPIServer {
	public function get() {
		$jobname = $this->Request->contains('name') ? $this->Request['name'] : '';
		$clientid = null;
		if ($this->Request->contains('clientid')) {
			$clientid = intval($this->Request['clientid']);
		} elseif ($this->Request->contains('client') && $this->getModule('misc')->isValidName($this->Request['client'])) {
			$client = $this->Request['client'];
			$client_row = $this->getModule('client')->getClientByName($client);
			$clientid = is_object($client_row) ? $client_row->clientid : null;
		}
		$filesetid = null;
		if ($this->Request->contains('filesetid')) {
			$filesetid = intval($this->Request['filesetid']);
		} elseif ($this->Request->contains('fileset') && $this->getModule('misc')->isValidName($this->Request['fileset'])) {
			$fileset = $this->Request['fileset'];
			$fileset_row = $this->getModule('fileset')->getFileSetByName($fileset);
			$filesetid = is_object($fileset_row) ? $fileset_row->filesetid : null;
		}

		if (!is_int($clientid)) {
			$this->output = ClientError::MSG_ERROR_CLIENT_DOES_NOT_EXISTS;
			$this->error = ClientError::ERROR_CLIENT_DOES_NOT_EXISTS;
		} elseif (!is_int($filesetid)) {
			$this->output = FileSetError::MSG_ERROR_FILESET_DOES_NOT_EXISTS;
			$this->error = FileSetError::ERROR_FILESET_DOES_NOT_EXISTS;
		} else {
			$result = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.jobs'));
			if ($result->exitcode === 0) {
				array_shift($result->output);
				if (in_array($jobname, $result->output)) {
					$jobs = $this->getModule('job')->getRecentJobids($jobname, $clientid, $filesetid);
					if (is_array($jobs)) {
						$this->output = $jobs;
						$this->error = JobError::ERROR_NO_ERRORS;
					} else {
						$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
						$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
					}
				} else {
					$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
					$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
				}
			} else {
				$this->output = $result->output;
				$this->error = $result->exitcode;
			}
		}
	}
}

?>
