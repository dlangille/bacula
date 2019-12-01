<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2019 Kern Sibbald
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
 
class BVFSVersions extends BaculumAPIServer {

	public function get() {
		$jobid = $this->Request->contains('jobid') ? intval($this->Request['jobid']) : 0;
		$pathid = $this->Request->contains('pathid') ? intval($this->Request['pathid']) : 0;
		$filenameid = $this->Request->contains('filenameid') ? intval($this->Request['filenameid']) : 0;
		$copies = $this->Request->contains('copies') ? intval($this->Request['copies']) : 0;
		$client = null;
		if ($this->Request->contains('client') && $this->getModule('misc')->isValidName($this->Request['client'])) {
			$client = $this->Request['client'];
		}

		if (is_null($client)) {
			$this->output = BVFSError::MSG_ERROR_INVALID_CLIENT;
			$this->error = BVFSError::ERROR_INVALID_CLIENT;
			return;
		}

		$cmd = array(
			'.bvfs_versions',
			'client="' . $client . '"',
			'jobid="' . $jobid . '"',
			'pathid="' . $pathid . '"',
			'fnid="' . $filenameid . '"'
		);
		if ($copies == 1) {
			$cmd[] = 'copies';
		}
		$result = $this->getModule('bconsole')->bconsoleCommand($this->director, $cmd);
		$this->output = $result->output;
		$this->error = $result->exitcode;
	}
}
?>
