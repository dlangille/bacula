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

class JSONToolsTest extends BaculumAPIServer {

	public function set($id, $params) {
		$type = property_exists($params, 'type') ? $params->type : '';
		$path = property_exists($params, 'path') ? $params->path : '';
		$cfg = property_exists($params, 'cfg') ? $params->cfg : '';
		$use_sudo = property_exists($params, 'use_sudo') ? $params->use_sudo : false;
		$result = $this->getModule('json_tools')->testJSONTool($type, $path, $cfg, $use_sudo);
		if ($result === true) {
			$this->output = JSONToolsError::MSG_ERROR_NO_ERRORS;
			$this->error = JSONToolsError::ERROR_NO_ERRORS;
		} else {
			$this->output = JSONToolsError::MSG_ERROR_JSON_TOOLS_CONNECTION_PROBLEM;
			$this->error = JSONToolsError::ERROR_JSON_TOOLS_CONNECTION_PROBLEM;
		}
	}
}
?>
