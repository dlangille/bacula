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
 
class Pool extends BaculumAPIServer {
	public function get() {
		$poolid = $this->Request->contains('id') ? intval($this->Request['id']) : 0;
		$pool_name = $this->Request->contains('name') ? $this->Request['name'] : '';
		$pool = null;
		if ($poolid > 0) {
			$pool = $this->getModule('pool')->getPoolById($poolid);
		} elseif (!empty($pool_name)) {
			$pool = $this->getModule('pool')->getPoolByName($pool_name);
		}
		$allowedPools = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.pool'));
		if ($allowedPools->exitcode === 0) {
			array_shift($allowedPools->output);
			if(!is_null($pool) && in_array($pool->name, $allowedPools->output)) {
				$this->output = $pool;
				$this->error = PoolError::ERROR_NO_ERRORS;
			} else {
				$this->output = PoolError::MSG_ERROR_POOL_DOES_NOT_EXISTS;
				$this->error = PoolError::ERROR_POOL_DOES_NOT_EXISTS;
			}
		} else {
			$this->output = $allowedPools->output;
			$this->error = $allowedPools->exitcode;
		}
	}
}

?>
