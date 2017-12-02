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

	public function set($id, $params) {
		$limit = $this->Request->contains('limit') ? intval($this->Request['limit']) : 0;
		$offset = $this->Request->contains('offset') ? intval($this->Request['offset']) : 0;
		$jobids = null;
		if (property_exists($params, 'jobids') && $this->getModule('misc')->isValidIdsList($params->jobids)) {
			$jobids = $params->jobids;
		}

		$path = null;
		if (property_exists($params, 'path') && $this->getModule('misc')->isValidPath($params->path)) {
			$path = $params->path;
		}
		
		if (is_null($jobids)) {
			$this->output = BVFSError::MSG_ERROR_INVALID_JOBID_LIST;
			$this->error = BVFSError::ERROR_INVALID_JOBID_LIST;
			return;
		}

		if (is_null($path)) {
			$this->output = BVFSError::ERROR_INVALID_RESTORE_PATH;
			$this->error = BVFSError::MSG_ERROR_INVALID_RESTORE_PATH;
			return;
		}

		$cmd = array('.bvfs_lsdirs', 'jobid="' . $jobids . '"', 'path="' . $path . '"');

		if ($offset > 0) {
			array_push($cmd, 'offset="' .  $offset . '"');
		}
		if ($limit > 0) {
			array_push($cmd, 'limit="' .  $limit . '"');
		}
		$result = $this->getModule('bconsole')->bconsoleCommand($this->director, $cmd);
		$this->output = $result->output;
		$this->error = $result->exitcode;
	}
}
?>
