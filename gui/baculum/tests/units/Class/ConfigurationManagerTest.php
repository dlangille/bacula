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

	public function testGetCryptedPassword() {
		$testData = array();
		$testData[] = array('password' => 'zzzzzzzzz', 'hash' => 'enEzofZKX2/wM');
		$testData[] = array('password' => 'admin', 'hash' => 'YWG41BPzVAkN6');
		$testData[] = array('password' => 'a dmin', 'hash' => 'YSz/JgtyVAHuc');
		$testData[] = array('password' => 'a', 'hash' => 'YQebj.HAzzu1c');
		$testData[] = array('password' => 'ADMIN', 'hash' => 'QUtX9W0NVx75o');
		$testData[] = array('password' => ' ', 'hash' => 'IAeLKSsdm161I');
		$testData[] = array('password' => 'a b c', 'hash' => 'YSMYQTGUwHPTE');
		$testData[] = array('password' => 'ąśćłóżźćń', 'hash' => 'xIObay0jyQnD2');
		$testData[] = array('password' => '$$$$', 'hash' => 'JCgedtNc0KHRw');
		$testData[] = array('password' => '\$$\$$', 'hash' => 'XCFJMFspzHfN6');
		$testData[] = array('password' => '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ', 'hash' => 'MD.GrjSxikZ26');
		$testData[] = array('password' => "\t\n\t\r", 'hash' => 'CQUexWT5q3vHc');
		$testData[] = array('password' => '\t\n\t\r', 'hash' => 'XHACZ9CpS6KIw');
		$testData[] = array('password' => '$a=1;print $a;', 'hash' => 'JGr9Nl2UPwz1Y');

		for ($i = 0; $i < count($testData); $i++) {
			$result = self::$application->getModule('configuration')->getCryptedPassword($testData[$i]['password']);
			$this->assertEquals($testData[$i]['hash'], $result);
		}
	}

	public function testGetRandomString() {
		$testData = array(
			'iterations' => 50,
			'pattern' => '/^[a-zA-Z0-9]{62}$/',
			'characters' => "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
		);
		for ($i = 0; $i < $testData['iterations']; $i++) {
			$str = self::$application->getModule('configuration')->getRandomString();
			$result = preg_match($testData['pattern'], $str);
			$this->assertEquals(1, $result);
		}

		$charactersCopy = $testData['characters'];
		$str = self::$application->getModule('configuration')->getRandomString();
		for ($i = 0; $i < strlen($testData['characters']); $i++) {
			$pos = strpos($str, $charactersCopy[$i]);
			$len = strlen($str);
			$str_l = substr($str, 0, $pos);
			$str_r = ($len > 1) ? substr($str, ($pos+1), $len) : '';
			$str = $str_l . $str_r;
		}
		$result = empty($str);
		$this->assertTrue($result);
	}
}

?>
