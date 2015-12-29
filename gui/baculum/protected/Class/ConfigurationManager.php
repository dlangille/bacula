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

Prado::using('Application.Class.Miscellaneous');

class ConfigurationManager extends TModule
{

	/**
	 * Location o application configuration file.
	 */
	const CONFIG_FILE = 'Application.Data.settings';

	/**
	 * Users login and password file for HTTP Basic auth.
	 */
	const USERS_FILE = 'Application.Data.baculum';

	/**
	 * PostgreSQL default params.
	 */
	const PGSQL = 'pgsql';
	const PGSQL_NAME = 'PostgreSQL';
	const PGSQL_PORT = 5432;

	/**
	 * MySQL default params.
	 */
	const MYSQL = 'mysql';
	const MYSQL_NAME = 'MySQL';
	const MYSQL_PORT = 3306;

	/**
	 * SQLite default params.
	 */
	const SQLITE = 'sqlite';
	const SQLITE_NAME = 'SQLite';
	const SQLITE_PORT = null;

	/**
	 * Default language for application.
	 */
	const DEFAULT_LANGUAGE = 'en';

	public function getDbNameByType($type) {
		switch($type) {
			case self::PGSQL: $dbName = self::PGSQL_NAME; break;
			case self::MYSQL: $dbName = self::MYSQL_NAME; break;
			case self::SQLITE: $dbName = self::SQLITE_NAME; break;
			default: $dbName = null; break;
		}
		return $dbName;
	}

	public function getPostgreSQLType() {
		return self::PGSQL;
	}

	public function getMySQLType() {
		return self::MYSQL;
	}

	public function getSQLiteType() {
		return self::SQLITE;
	}

	public function isPostgreSQLType($type) {
		return ($type === self::PGSQL);
	}

	public function isMySQLType($type) {
		return ($type === self::MYSQL);
	}

	public function isSQLiteType($type) {
		return ($type === self::SQLITE);
	}

	public function getLanguage() {
		$language = self::DEFAULT_LANGUAGE;
		if ($this->isApplicationConfig() === true) {
			$config = $this->getApplicationConfig();
			if (array_key_exists('lang', $config['baculum'])) {
				$language = $config['baculum']['lang'];
			}
		}
		return $language;
	}

	public function setLanguage($language) {
		
	}

	/**
	 * Saving application configuration.
	 * 
	 * @access public
	 * @param array $config structure of config file params
	 * @return boolean true if config save is successfully, false if config save is failure
	 */
	public function setApplicationConfig(array $config) {
		$cfgFile = Prado::getPathOfNamespace(self::CONFIG_FILE, '.conf');
		return ($this->Application->getModule('misc')->writeINIFile($cfgFile, $config) != false);
	}

	/**
	 * Getting application configuration.
	 * 
	 * @access public
	 * @return array application configuration
	 */
	public static function getApplicationConfig() {
		$cfgFile = Prado::getPathOfNamespace(self::CONFIG_FILE, '.conf');
		return Miscellaneous::parseINIFile($cfgFile);
	}

	/**
	 * Checking if application configuration file exists.
	 * 
	 * @access public
	 * @return boolean true if file exists, otherwise false
	 */
	public function isApplicationConfig() {
		return file_exists(Prado::getPathOfNamespace(self::CONFIG_FILE, '.conf'));
	}

	public function getCryptedPassword($password) {
		$enc_pwd = crypt($password, base64_encode($password));
		return $enc_pwd;
	}

	/**
	 * Saving user to users configuration file.
	 *
	 * NOTE!
	 * So far by webGUI is possible to set one user.
	 * For more users and restricted consoles, there is need to modify
	 * users and passwords file.
	 *
	 * TODO: Support for more than one user setting on webGUI.
	 *
	 * @access public
	 * @param string $user username
	 * @param string $password user's password
	 * @param boolean $firstUsage determine if it is first saved user during first Baculum run
	 * @param mixed $oldUser previous username before change
	 * @return boolean true if user saved successfully, otherwise false
	 */
	public function setUsersConfig($user, $password, $firstUsage = false, $oldUser = null) {
		$allUsers = $this->getAllUsers();
		$password = $this->getCryptedPassword($password);

		if($firstUsage === true) {
			$this->clearUsersConfig();
		}

		$userExists = array_key_exists($user, $allUsers);


		if ($userExists === true) {
			// update user password;
			$allUsers[$user] = $password;
		}

		if(!is_null($oldUser) && $oldUser !== $user) {
			// delete old username with password from configuration file
			if(array_key_exists($oldUser, $allUsers)) {
				unset($allUsers[$oldUser]);
			}
		}

		// add new user if does not exist
		if($userExists === false) {
			$allUsers[$user] = $password;
		}

		$result = $this->saveUserConfig($allUsers);
		return $result;
	}

	public function getAllUsers() {
		$allUsers = array();
		if ($this->isUsersConfig() === true) {
			$usersFile = Prado::getPathOfNamespace(self::USERS_FILE, '.users');
			$users = file($usersFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

			for($i = 0; $i < count($users); $i++) {
				if(preg_match("/^(?P<user>\S+)\:(?P<hash>\S+)$/", $users[$i], $match) === 1) {
					$allUsers[$match['user']] = $match['hash'];
				}
			}
		}
		return $allUsers;
	}

	public function saveUserConfig($allUsers) {
		$users = array();
		foreach ($allUsers as $user => $pwd) {
			$users[] = "$user:$pwd";
		}
		$usersFile = Prado::getPathOfNamespace(self::USERS_FILE, '.users');
		$usersToFile = implode("\n", $users);
		$old_umask = umask(0);
		umask(0077);
		$result = file_put_contents($usersFile, $usersToFile) !== false;
		umask($old_umask);
		return $result;
	}

	public function removeUser($username) {
		$result = false;
		$allUsers = $this->getAllUsers();
		if (array_key_exists($username, $allUsers)) {
			unset($allUsers[$username]);
			$result = $this->saveUserConfig($allUsers);
		}
		return $result;
	}

	/**
	 * Checking if users configuration file exists.
	 *
	 * @access public
	 * @return boolean true if file exists, otherwise false
	 */
	public function isUsersConfig() {
		return file_exists(Prado::getPathOfNamespace(self::USERS_FILE, '.users'));
	}

	/**
	 * Clear all content of users file.
	 *
	 * @access private
	 * @return boolean true if file cleared successfully, otherwise false
	 */
	private function clearUsersConfig() {
		$usersFile = Prado::getPathOfNamespace(self::USERS_FILE, '.users');
		$result = file_put_contents($usersFile, '') !== false;
		return $result;
	}

	public function switchToUser($http_protocol, $host, $port, $user, $password) {
		$urlPrefix = $this->Application->getModule('friendly-url')->getUrlPrefix();
		$location = sprintf("%s://%s:%s@%s:%d%s", $http_protocol, $user, $password, $host, $port, $urlPrefix);
		header("Location: $location");
	}

	public function getRandomString() {
		$characters = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		$rand_string = str_shuffle($characters);
		return $rand_string;
	}
}
?>
