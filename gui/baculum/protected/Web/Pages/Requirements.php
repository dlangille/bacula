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
 
class Requirements {
	const ASSETS_DIR = 'assets';
	const CONFIG_DIR = 'Config';
	const LOG_DIR = 'Logs';
	const RUNTIME_DIR = 'protected/runtime';

	public function __construct($app_dir, $base_dir) {
		$app_dir .= '/';
		$base_dir .= '/';
		$this->validateEnvironment($app_dir, $base_dir);
	}

	private function validateEnvironment($app_dir, $base_dir) {
		$requirements = array();
		$assets_dir = $app_dir . self::ASSETS_DIR;
		if(!is_writable(self::ASSETS_DIR)) {
			$requirements[] = 'Please make writable by the web server next directory: <b>' . $assets_dir . '</b>';
		}

		$config_dir = $base_dir . self::CONFIG_DIR;
		if(!is_writable($config_dir)) {
			$requirements[] = 'Please make writable by the web server next directory: <b>' . $config_dir . '</b>';
		}

		$log_dir = $base_dir . self::LOG_DIR;
		if(!is_writable($log_dir)) {
			$requirements[] = 'Please make writable by the web server next directory: <b>' . $log_dir . '</b>';
		}

		$runtime_dir = $app_dir . self::RUNTIME_DIR;
		if(!is_writable($runtime_dir)) {
			$requirements[] = 'Please make writable by the web server next directory: <b>' . $runtime_dir . '</b>';
		}

		if(!function_exists('curl_init') || !function_exists('curl_setopt') || !function_exists('curl_exec') || !function_exists('curl_close')) {
			$requirements[] = 'Please install <b>cURL PHP module</b>.';
		}

		if(!function_exists('bcmul') || !function_exists('bcpow')) {
			$requirements[] = 'Please install <b>BCMath PHP module</b>.';
		}

		if(!function_exists('mb_strlen')) {
			$requirements[] = 'Please install <b>MB String PHP module</b> for support for multi-byte string handling to PHP.';
		}

		if(!function_exists('json_decode')) {
			$requirements[] = 'Please install <b>Module for JSON functions in PHP scripts</b>.';
		}

		if(!class_exists('DOMDocument')) {
			$requirements[] = 'Please install <b>PHP DOM XML</b> to support XML documents (usually included in php-xml binary package).';
		}

		if(count($requirements) > 0) {
			echo '<html><body><h2>Baculum - Missing dependencies</h2><ul>';
			for($i = 0; $i < count($requirements); $i++) {
				echo '<li>' . $requirements[$i] . '</li>';
				
			}
			echo '</ul>';
			echo 'To run Baculum <u>please correct above requirements</u> and refresh this page in web browser.';
			echo '</body></html>';
			exit();
		}
	}
}
?>
