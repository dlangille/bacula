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
 * Test cases to read/write/check Baculum configuration file (settings.conf).
 *
 * @author Marcin Haba
 */
class ConfigurationManagerApplicationSettingsTest extends PHPUnit_Framework_TestCase {

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

	public function testSetApplicationConfigPostgreSQL() {
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
		$resultContent = '[db]
type = "pgsql"
name = "my database 123"
login = "admin321"
password = "Str0NgPa$$w0Rd"
ip_addr = "127.0.0.1"
port = "5433"
path = ""

[bconsole]
bin_path = "/usr/local/bacula/sbin/bconsole"
cfg_path = "/usr/local/bacula/etc/bconsole.conf"
cfg_custom_path = "/etc/bacula/bconsole-{user}.conf"
use_sudo = "1"

[baculum]
login = "ganiuszka"
password = "AnOTHe3Str0n6 Pass W OR d"
debug = "0"
lang = "pl"

';
		// check if config does not exist
		$result = self::$application->getModule('configuration')->isApplicationConfig();
		$this->assertFalse($result);

		// check saving config
		self::$application->getModule('configuration')->setApplicationConfig($testData);
		$result = file_get_contents(BACULUM_ROOT_DIR . 'protected/Data/settings.conf');
		// compare written config with expected string
		$this->assertEquals($resultContent, $result);

		// check if config already exists
		$result = self::$application->getModule('configuration')->isApplicationConfig();
		$this->assertTrue($result);

		// check reading config
		$configRead = self::$application->getModule('configuration')->getApplicationConfig();
		// compare test data array with read application config as array
		$this->assertEquals($testData, $configRead);
	}

	public function testSetApplicationConfigMySQL() {
		$testData = array('db' => array(), 'bconsole' => array(), 'baculum' => array());
		$testData['db']['type'] = 'mysql';
		$testData['db']['name'] = 'mydatabase538';
		$testData['db']['login'] = 'administrator';
		$testData['db']['password'] = '#Str0NgPa$##$w0Rd^$#!&#*$@';
		$testData['db']['ip_addr'] = '192.168.0.4';
		$testData['db']['port'] = '3306';
		$testData['db']['path'] = '';
		$testData['bconsole']['bin_path'] = '/usr/local/bacula/sbin/bconsole';
		$testData['bconsole']['cfg_path'] = '/usr/local/bacula/etc/bconsole.conf';
		$testData['bconsole']['cfg_custom_path'] = '/etc/bacula/bconsole-{user}.conf';
		$testData['bconsole']['use_sudo'] = 0;
		$testData['baculum']['login'] = 'ganiuszka';
		$testData['baculum']['password'] = 'Str0n6 Pass4W2OR d';
		$testData['baculum']['debug'] = 1;
		$testData['baculum']['lang'] = 'en';
		$resultContent = '[db]
type = "mysql"
name = "mydatabase538"
login = "administrator"
password = "#Str0NgPa$##$w0Rd^$#!&#*$@"
ip_addr = "192.168.0.4"
port = "3306"
path = ""

[bconsole]
bin_path = "/usr/local/bacula/sbin/bconsole"
cfg_path = "/usr/local/bacula/etc/bconsole.conf"
cfg_custom_path = "/etc/bacula/bconsole-{user}.conf"
use_sudo = "0"

[baculum]
login = "ganiuszka"
password = "Str0n6 Pass4W2OR d"
debug = "1"
lang = "en"

';

		// check if config does not exist
		$result = self::$application->getModule('configuration')->isApplicationConfig();
		$this->assertFalse($result);

		// check saving config
		self::$application->getModule('configuration')->setApplicationConfig($testData);
		$result = file_get_contents(BACULUM_ROOT_DIR . 'protected/Data/settings.conf');
		// compare written config with expected string
		$this->assertEquals($resultContent, $result);

		// check if config already exists
		$result = self::$application->getModule('configuration')->isApplicationConfig();
		$this->assertTrue($result);

		// check reading config
		$configRead = self::$application->getModule('configuration')->getApplicationConfig();
		// compare test data array with read application config as array
		$this->assertEquals($testData, $configRead);
	}

	public function testSetApplicationConfigSQLite() {
		$testData = array('db' => array(), 'bconsole' => array(), 'baculum' => array());
		$testData['db']['type'] = 'sqlite';
		$testData['db']['name'] = 'bacula';
		$testData['db']['login'] = 'bacula';
		$testData['db']['password'] = '';
		$testData['db']['ip_addr'] = '';
		$testData['db']['port'] = '';
		$testData['db']['path'] = '/home/gani/mydatabase.db';
		$testData['bconsole']['bin_path'] = '/usr/local/bacula/sbin/bconsole';
		$testData['bconsole']['cfg_path'] = '/usr/local/bacula/etc/bconsole.conf';
		$testData['bconsole']['cfg_custom_path'] = '/etc/bacula/users/bconsole-{user}.conf';
		$testData['bconsole']['use_sudo'] = 0;
		$testData['baculum']['login'] = 'kekeke';
		$testData['baculum']['password'] = 'kekeke';
		$testData['baculum']['debug'] = 1;
		$testData['baculum']['lang'] = 'en';
		$resultContent = '[db]
type = "sqlite"
name = "bacula"
login = "bacula"
password = ""
ip_addr = ""
port = ""
path = "/home/gani/mydatabase.db"

[bconsole]
bin_path = "/usr/local/bacula/sbin/bconsole"
cfg_path = "/usr/local/bacula/etc/bconsole.conf"
cfg_custom_path = "/etc/bacula/users/bconsole-{user}.conf"
use_sudo = "0"

[baculum]
login = "kekeke"
password = "kekeke"
debug = "1"
lang = "en"

';
		// check if config does not exist
		$result = self::$application->getModule('configuration')->isApplicationConfig();
		$this->assertFalse($result);

		// check saving config
		self::$application->getModule('configuration')->setApplicationConfig($testData);
		$result = file_get_contents(BACULUM_ROOT_DIR . 'protected/Data/settings.conf');
		// compare written config with expected string
		$this->assertEquals($resultContent, $result);

		// check if config already exists
		$result = self::$application->getModule('configuration')->isApplicationConfig();
		$this->assertTrue($result);

		// check reading config
		$configRead = self::$application->getModule('configuration')->getApplicationConfig();
		// compare test data array with read application config as array
		$this->assertEquals($testData, $configRead);
	}
}
?>
