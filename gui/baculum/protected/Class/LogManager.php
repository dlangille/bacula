<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2016 Kern Sibbald
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
 
class LogManager extends TModule {

	public function getLogByJobId($jobid) {
		$logs = LogRecord::finder()->findAllByjobid($jobid);
		$joblog = null;
		if(is_array($logs)) {
			$joblog = array();
			foreach($logs as $log) {
				$joblog[] = $log->logtext;
			}
		}
		return $joblog;
	}
}
?>
