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

$timezone = 'UTC';
if (!ini_get('date.timezone')) {
	if (strtoupper(substr(PHP_OS, 0, 3)) != 'WIN') {
		exec('date +%Z', $tz, $retcode);
		if ($retcode === 0 && count($tz) === 1) {
			$timezone = $tz[0];
		}
	}
	date_default_timezone_set($timezone);
}

// Support for web servers which do not provide direct info about HTTP Basic auth to PHP superglobal $_SERVER array.
if(!isset($_SERVER['PHP_AUTH_USER']) && !isset($_SERVER['PHP_AUTH_PW'])) {
    list($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']) = explode(':', base64_decode(substr($_SERVER['HTTP_AUTHORIZATION'], 6)));
}

require_once('./protected/Pages/Requirements.php');
new Requirements(dirname(__DIR__));

?>
