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
 
class BVFSLsDirs extends BaculumAPIServer {

	public function set($param, $ids) {
		$limit = intval($this->Request['limit']);
		$offset = intval($this->Request['offset']);
		$is_valid = true;
		if (property_exists($ids, 'jobids')) {
			$jobids = explode(',', $ids->jobids);
			for($i = 0; $i < count($jobids); $i++) {
				$job = $this->getModule('job')->getJobById($jobids[$i]);
				if(is_null($job)) {
					$is_valid = false;
					break;
				}
			}
		} else {
			$is_valid = false;
		}

		$path = null;
		if (property_exists($ids, 'path')) {
			$path = $ids->path;
		}
		
		if($is_valid === true) {
			if (!is_null($path)) {
				$cmd = array('.bvfs_lsdirs', 'jobid="' . $ids->jobids . '"', 'path="' . $path . '"');

				if($offset > 0) {
					array_push($cmd, 'offset="' .  $offset . '"');
				}
				if($limit > 0) {
					array_push($cmd, 'limit="' .  $limit . '"');
				}
				$result = $this->getModule('bconsole')->bconsoleCommand($this->director, $cmd, $this->user);
				$this->output = $result->output;
				$this->error = $result->exitcode;
			} else {
				$this->output = BVFSError::ERROR_INVALID_RESTORE_PATH;
				$this->error = BVFSError::MSG_ERROR_INVALID_RESTORE_PATH;
			}
		} else {
			$this->output = BVFSError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
			$this->error = BVFSError::ERROR_JOB_DOES_NOT_EXISTS;
		}
	}
}
?>
