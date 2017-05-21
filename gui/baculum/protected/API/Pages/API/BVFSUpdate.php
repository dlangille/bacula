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

class BVFSUpdate extends BaculumAPIServer {

	public function get() {}

	public function set($param, $ids) {
		$isValid = true;
		if (property_exists($ids, 'jobids')) {
			$jobids = explode(',', $ids->jobids);
			for($i = 0; $i < count($jobids); $i++) {
				$job = $this->getModule('job')->getJobById($jobids[$i]);
				if(is_null($job)) {
					$isValid = false;
					break;
				}
			}
		} else {
			$isValid = false;
		}
		
		if($isValid === true) {
			$result = $this->getModule('bconsole')->bconsoleCommand(
				$this->director,
				array('.bvfs_update', 'jobid="' . $ids->jobids . '"'),
				$this->user
			);
			$this->output = $result->output;
			$this->error = $result->exitcode;
		} else {
			$this->output = BVFSError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
			$this->error = BVFSError::ERROR_JOB_DOES_NOT_EXISTS;
		}
	}
}
?>
