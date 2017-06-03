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

Prado::using('Application.API.Class.APIDbModule');
 
class RestoreRun extends BaculumAPIServer {

	public function get() {}

	public function create($params) {
		$rfile = property_exists($params, 'rpath') ? $params->rpath : null;

		$fileset = null;
		if (property_exists($params, 'fileset') && $this->getModule('misc')->isValidName($params->fileset)) {
			$fileset = $params->fileset;
		}
		$client = null;
		if (property_exists($params, 'clientid')) {
			$clientid = intval($params->clientid);
			$client_row = $this->getModule('client')->getClientById($clientid);
			$client = is_object($client_row) ? $client_row->name : null;
		} elseif (property_exists($params, 'client') && $this->getModule('misc')->isValidName($params->client)) {
			$client = $params->client;
		}
		$priority = property_exists($params, 'priority') ? intval($params->priority) : 10; // default priority is set to 10
		$where = property_exists($params, 'where') ? $params->where : null;
		$replace = property_exists($params, 'replace') ? $params->replace : null;
		$misc = $this->getModule('misc');

		if(!is_null($fileset)) {
			if(!is_null($client)) {
				if(preg_match($misc::RPATH_PATTERN, $rfile) === 1) {
					if(!is_null($where)) {
						if(!is_null($replace)) {
							$command = array('restore',
								'file="?' . $rfile . '"',
								'client="' . $client . '"',
								'where="' . $where . '"',
								'replace="' . $replace . '"',
								'fileset="' . $fileset . '"',
								'priority="' . $priority . '"',
								'yes'
							);
							$restore = $this->getModule('bconsole')->bconsoleCommand($this->director, $command, $this->user);
							$this->removeTmpRestoreTable($rfile);
							$this->output = $restore->output;
							$this->error = (integer)$restore->exitcode;
						} else {
							$this->output = JobError::MSG_ERROR_INVALID_REPLACE_OPTION;
							$this->error = JobError::ERROR_INVALID_REPLACE_OPTION;
						}
					} else {
						$this->output = JobError::MSG_ERROR_INVALID_WHERE_OPTION;
						$this->error = JobError::ERROR_INVALID_WHERE_OPTION;
					}
				} else {
					$this->output = JobError::MSG_ERROR_INVALID_RPATH;
					$this->error = JobError::ERROR_INVALID_RPATH;
				}
			} else {
				$this->output = JobError::MSG_ERROR_CLIENT_DOES_NOT_EXISTS;
				$this->error = JobError::ERROR_CLIENT_DOES_NOT_EXISTS;
			}
		} else {
			$this->output = JobError::MSG_ERROR_FILESET_DOES_NOT_EXISTS;
			$this->error = JobError::ERROR_FILESET_DOES_NOT_EXISTS;
		}
	}

	private function removeTmpRestoreTable($tableName) {
		$misc = $this->getModule('misc');
		if (preg_match($misc::RPATH_PATTERN, $tableName) === 1) {
			// @TODO: Move it to API module. It shouldn't look like here.
			$db_params = $this->getModule('api_config')->getConfig('db');
			$connection = APIDbModule::getAPIDbConnection($db_params);
			$connection->setActive(true);
			$sql = "DROP TABLE $tableName";
			$pdo = $connection->getPdoInstance();
			try {
				$pdo->exec($sql);
			} catch(PDOException $e) {
				$emsg = 'Problem during delete temporary Bvfs table. ' . $e->getMessage();
				$this->getModule('logging')->log(
					__FUNCTION__,
					$emsg,
					Logging::CATEGORY_APPLICATION,
					__FILE__,
					__LINE__
				);
			}
			$pdo = null;
		}
	}
}

?>
