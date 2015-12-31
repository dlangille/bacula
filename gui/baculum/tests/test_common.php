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

// Set framework in test run mode
define('PRADO_TEST_RUN', true);

define('BACULUM_ROOT_DIR', dirname(__FILE__) . '/../');
define('FRAMEWORK_ROOT_DIR', BACULUM_ROOT_DIR . 'framework');
define('BACKUP_FILES_PATH', '/tmp/baculum-unittest');

set_include_path(dirname(__FILE__) . PATH_SEPARATOR . FRAMEWORK_ROOT_DIR . PATH_SEPARATOR . get_include_path());

require_once(FRAMEWORK_ROOT_DIR . '/prado.php');

class TApplicationTest extends TApplication {

	public function run() {
		$this->initApplication();
	}
}

function copy_path($source, $destination) {
	if (is_dir($source)) {
		if (!is_dir($destination)) {
			mkdir($destination);
		}
		$path = dir($source);
		while (($file = $path->read()) != false) {
			if ($file == '.' || $file == '..') {
				continue;
			}
			$pathDir = $source . '/' . $file;
			copy($pathDir, $destination . '/' . $file);
		}
		$path->close();
	}
}

function remove_path($path, $only_content = false) {
	if(is_dir($path)) {
		$dir = dir($path);
		while (($file = $dir->read()) != false) {
			if ($file == '.' || $file == '..') {
				continue;
			}
			$pathDir = $path . '/' . $file;
			unlink($pathDir);
		}
		if ($only_content === false) {
			rmdir(dirname($pathDir));
		}
	}
}

?>
