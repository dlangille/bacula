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
 
/**
 * Clients resources.
 * 
 * Data format:
 * {
 *     "output": [
 *         {
 *             "clientid": client ID,
 *             "name": "client name",
 *             "uname": "client name and environment (uname -a)",
 *             "autoprune": 0 for disabled, 1 for enabled,
 *             "fileretention": file retention period in seconds,
 *             "jobretention": job retention period in seconds,
 *         },
 *         {
 *             "clientid": client ID,
 *             "name": "client name",
 *             "uname": "client name and environment (uname -a)",
 *             "autoprune": 0 for disabled, 1 for enabled,
 *             "fileretention": file retention period in seconds,
 *             "jobretention": job retention period in seconds,
 *         }
 * 		etc...
 *     ],
 *     "error": 0 for no errors, 1 for error
 * }
 */

class Clients extends BaculumAPIServer {

	public function get() {
		$limit = intval($this->Request['limit']);
		$clients = $this->getModule('client')->getClients($limit);
		$allowed_clients = $this->getModule('bconsole')->bconsoleCommand($this->director, array('.client'), $this->user);
		if ($allowed_clients->exitcode === 0) {
			$clients_output = array();
			foreach($clients as $client) {
				if(in_array($client->name, $allowed_clients->output)) {
					$clients_output[] = $client;
				}
			}
			$this->output = $clients_output;
			$this->error = ClientError::ERROR_NO_ERRORS;
		} else {

			$this->output = $allowed_clients->output;
			$this->error = $allowed_clients->exitcode;
		}
	}
}

?>
