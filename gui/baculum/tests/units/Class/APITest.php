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

require_once('test_common.php');

/**
 * Test cases to internal API client module.
 *
 * @author Marcin Haba
 */
class APITest extends PHPUnit_Framework_TestCase {

	public static $application = null;

	public function setUp() {
		if (self::$application === null) {
			self::$application = new TApplicationTest(BACULUM_ROOT_DIR . 'protected');
			self::$application->run();
		}
		if (file_exists(BACULUM_ROOT_DIR . 'protected/Data')) {
			copy_path(BACULUM_ROOT_DIR . 'protected/Data', BACKUP_FILES_PATH);
			remove_path(BACULUM_ROOT_DIR . 'protected/Data', true);
		}
	}

	public function tearDown() {
		if (file_exists(BACKUP_FILES_PATH)) {
			remove_path(BACKUP_FILES_PATH . 'protected/Data', true);
			copy_path(BACKUP_FILES_PATH, BACULUM_ROOT_DIR . 'protected/Data');
			remove_path(BACKUP_FILES_PATH);
		}
	}

	public function testApiVersion() {
		$testData = array('result' => '0.1');
		$apiModule = self::$application->getModule('api');
		$this->assertEquals($testData['result'], $apiModule::API_VERSION);
	}

	public function testGetConnection() {
		// prepare example config
		$testData = array('db' => array(), 'bconsole' => array(), 'baculum' => array());
		$testData['db']['type'] = 'pgsql';
		$testData['db']['name'] = 'my database 123';
		$testData['db']['login'] = 'admin321';
		$testData['db']['password'] = 'Str0NgPa$$w0Rd';
		$testData['db']['ip_addr'] = '127.0.0.1';
		$testData['db']['port'] = '5433';
		$testData['db']['path'] = '';
		$testData['bconsole']['bin_path'] = '/usr/local/bacula/sbin/bconsole';
		$testData['bconsole']['cfg_path'] = '/usr/local/bacula/etc/bconsole.conf';
		$testData['bconsole']['cfg_custom_path'] = '/etc/bacula/bconsole-{user}.conf';
		$testData['bconsole']['use_sudo'] = 1;
		$testData['baculum']['login'] = 'ganiuszka';
		$testData['baculum']['password'] = 'AnOTHe3Str0n6 Pass W OR d';
		$testData['baculum']['debug'] = 0;
		$testData['baculum']['lang'] = 'pl';

		$testResult = array('type' => 'resource');

		// save example config
		self::$application->getModule('configuration')->setApplicationConfig($testData);

		$result = self::$application->getModule('api')->getConnection();
		$this->assertEquals($testResult['type'], gettype($result));
	}
}
?>
