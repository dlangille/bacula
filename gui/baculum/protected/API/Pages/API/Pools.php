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
 
class Pools extends BaculumAPIServer {
	public function get() {
		$limit = intval($this->Request['limit']);
		$pools = $this->getModule('pool')->getPools($limit);
		$allowedPools = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.pool'), $this->user);
		if ($allowedPools->exitcode === 0) {
			$poolsOutput = array();
			foreach($pools as $pool) {
				if(in_array($pool->name, $allowedPools->output)) {
					$poolsOutput[] = $pool;
				}
			}
			$this->output = $poolsOutput;
			$this->error = PoolError::ERROR_NO_ERRORS;
		} else {
			$this->output = $allowedPools->output;
			$this->error = $allowedPools->exitcode;
		}
	}
}
?>
