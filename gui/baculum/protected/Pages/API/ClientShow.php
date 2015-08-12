<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2015 Marcin Haba
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
 
class ClientShow extends BaculumAPI {
	public function get() {
		$clientid = intval($this->Request['id']);
		$client = $this->getModule('client')->getClientById($clientid);
		if(!is_null($client)) {
			$clientShow = $this->getModule('bconsole')->bconsoleCommand($this->director, array('show', 'client="' . $client->name . '"'), $this->user);
			$this->output = $clientShow->output;
			$this->error = (integer)$clientShow->exitcode;
		} else {
			$this->output = ClientError::MSG_ERROR_CLIENT_DOES_NOT_EXISTS;
			$this->error = ClientError::ERROR_CLIENT_DOES_NOT_EXISTS;
		}
	}
}

?>