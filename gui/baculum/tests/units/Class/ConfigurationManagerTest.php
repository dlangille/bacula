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
 * Basic Test cases to configuration manager module.
 *
 * @author Marcin Haba
 */
class ConfigurationManagerTest extends PHPUnit_Framework_TestCase {

	public static $application = null;

	public function setUp() {
		if(self::$application === null) {
			self::$application = new TApplicationTest(BACULUM_ROOT_DIR . 'protected');
			self::$application->run();
		}
	}

	public function testGetDbNameByTypeValid() {
		$testData = array(
			'pgsql' => 'PostgreSQL',
			'mysql' => 'MySQL',
			'sqlite' => 'SQLite'
		);
		foreach ($testData as $shortName => $longName) {
			$result = self::$application->getModule('configuration')->getDbNameByType($shortName);
			$this->assertEquals($longName, $result);
		}
	}

	public function testGetDbNameByTypeInvalid() {
		$testData = array("pgsql\n", ' pgsql', ' pgsql ', 'Mysql', 'MYSQL', 'SQlite', 'MySqL', null, true, 0, -1);
		for ($i = 0; $i < count($testData); $i++) {
			$result = self::$application->getModule('configuration')->getDbNameByType($testData[$i]);
			$this->assertNull($result);
		}
	}

	public function testIsPostgreSQLTypeValid() {
		$testData = 'pgsql';
		$result = self::$application->getModule('configuration')->isPostgreSQLType($testData);
		$this->assertTrue($result);
	}

	public function testIsPostgreSQLTypeInvalid() {
		$testData = array("pgsql\n", ' pgsql', ' pgsql ', 'mysql', 'MYSQL', 'sqlite', 'MySqL', null, true, 0, -1);
		for ($i = 0; $i < count($testData); $i++) {
			$result = self::$application->getModule('configuration')->isPostgreSQLType($testData[$i]);
			$this->assertFalse($result);
		}
	}

	public function testIsMySQLTypeValid() {
		$testData = 'mysql';
		$result = self::$application->getModule('configuration')->isMySQLType($testData);
		$this->assertTrue($result);
	}

	public function testIsMySQLTypeInvalid() {
		$testData = array("mysql\n", ' mysql', ' mysql ', 'm ysql', 'MYSQ', 'sqlite', 'pgsql', 'mysq', null, true, 0, -1);
		for ($i = 0; $i < count($testData); $i++) {
			$result = self::$application->getModule('configuration')->isMySQLType($testData[$i]);
			$this->assertFalse($result);
		}
	}
	public function testIsSQLiteTypeValid() {
		$testData = 'sqlite';
		$result = self::$application->getModule('configuration')->isSQLiteType($testData);
		$this->assertTrue($result);
	}

	public function testIsSQLiteTypeInvalid() {
		$testData = array("sqlite\n", ' sqlite', ' sqlite ', 's qlite', 'sqlit', 'pgsql', 'mysqs', null, true, 0, -1);
		for ($i = 0; $i < count($testData); $i++) {
			$result = self::$application->getModule('configuration')->isSQLiteType($testData[$i]);
			$this->assertFalse($result);
		}
	}

	public function testGetDefaultLanguageValid() {
		$mockData = false;
		$testData = 'en';

		$mock = $this->getMockBuilder('ConfigurationManager')->setMethods(array('isApplicationConfig'))->getMock();
		$mock->expects($this->once())->method('isApplicationConfig')->will($this->returnValue($mockData));
		$result = $mock->getLanguage();
		$this->assertEquals($testData, $result);
	}
}

?>
