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

class ConfigDirTest extends BaculumAPIServer {

	public function get() {}

	public function set($id, $params) {
		$config_dir = property_exists($params, 'path') ? $params->path : '';
		$result = $this->getModule('bacula_config')->testConfigDir($config_dir);
		if ($result === true) {
			$this->output = BaculaConfigError::MSG_ERROR_NO_ERRORS;
			$this->error = BaculaConfigError::ERROR_NO_ERRORS;
		} else {
			$this->output = BaculaConfigError::MSG_ERROR_CONFIG_DIR_NOT_WRITABLE;
			$this->error = BaculaConfigError::ERROR_CONFIG_DIR_NOT_WRITABLE;
		}
	}
}
