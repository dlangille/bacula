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

/**
 * Manage application configuration.
 * Module is responsible for get/set application config data like:
 * read/write application config and usersfiles, get application language
 * and others.
 *
 * @author Marcin Haba
 */
class ConfigurationManager extends TModule
{

	/**
	 * Application config file path
	 */
	const CONFIG_FILE = 'Application.Data.settings';

	/**
	 * Users login and password file for HTTP Basic auth.
	 */
	const USERS_FILE = 'Application.Data.baculum';

	/**
	 * PostgreSQL default params
	 */
	const PGSQL = 'pgsql';
	const PGSQL_NAME = 'PostgreSQL';
	const PGSQL_PORT = 5432;

	/**
	 * MySQL default params
	 */
	const MYSQL = 'mysql';
	const MYSQL_NAME = 'MySQL';
	const MYSQL_PORT = 3306;

	/**
	 * SQLite default params
	 */
	const SQLITE = 'sqlite';
	const SQLITE_NAME = 'SQLite';
	const SQLITE_PORT = null;

	/**
	 * Default application language
	 */
	const DEFAULT_LANGUAGE = 'en';

	/**
	 * Get database name by database type (short name).
	 * @access public
	 * @param string $type database type ('pgsql', 'mysql' ...)
	 * @return mixed database name or null if database name not found
	 */
	public function getDbNameByType($type) {
		$type = (string) $type;
		switch($type) {
			case self::PGSQL: $dbName = self::PGSQL_NAME; break;
			case self::MYSQL: $dbName = self::MYSQL_NAME; break;
			case self::SQLITE: $dbName = self::SQLITE_NAME; break;
			default: $dbName = null; break;
		}
		return $dbName;
	}

	/**
	 * Check if given database type is PostgreSQL type.
	 * @access public
	 * @param string $type database type ('pgsql', 'mysql' ...)
	 * @return boolean true if database type is PostgreSQL, otherwise false
	 */
	public function isPostgreSQLType($type) {
		return ($type === self::PGSQL);
	}

	/**
	 * Check if given database type is MySQL type.
	 * @access public
	 * @param string $type database type ('pgsql', 'mysql' ...)
	 * @return boolean true if database type is MySQL, otherwise false
	 */
	public function isMySQLType($type) {
		return ($type === self::MYSQL);
	}

	/**
	 * Check if given database type is SQLite type.
	 * @access public
	 * @param string $type database type ('sqlite', 'mysql' ...)
	 * @return boolean true if database type is SQLite, otherwise false
	 */
	public function isSQLiteType($type) {
		return ($type === self::SQLITE);
	}

	/**
	 * Get currently set application language short name.
	 * If no language set then default language is taken.
	 * @access public
	 * @return string lanuage short name
	 */
	public function getLanguage() {
		$language = self::DEFAULT_LANGUAGE;
		if ($this->isApplicationConfig() === true) {
			$config = self::getApplicationConfig();
			if (array_key_exists('lang', $config['baculum'])) {
				$language = $config['baculum']['lang'];
			}
		}
		return $language;
	}

	/**
	 * Save application configuration.
	 * @access public
	 * @param array $config structure of config file params
	 * @return boolean true if config save is successfully, false if config save is failure
	 */
	public function setApplicationConfig(array $config) {
		$cfgFile = Prado::getPathOfNamespace(self::CONFIG_FILE, '.conf');
		return ($this->Application->getModule('misc')->writeINIFile($cfgFile, $config) != false);
	}

	/**
	 * Get application configuration.
	 * @access public
	 * @return array application configuration
	 */
	public static function getApplicationConfig() {
		$cfgFile = Prado::getPathOfNamespace(self::CONFIG_FILE, '.conf');
		return Miscellaneous::parseINIFile($cfgFile);
	}

	/**
	 * Check if application configuration file exists.
	 * @access public
	 * @return boolean true if file exists, otherwise false
	 */
	public function isApplicationConfig() {
		return file_exists(Prado::getPathOfNamespace(self::CONFIG_FILE, '.conf'));
	}

	/**
	 * Get encrypted password to use in HTTP Basic auth.
	 *
	 * @access public
	 * @param string $password plain text password
	 * @return string encrypted password
	 */
	public function getCryptedPassword($password) {
		$enc_pwd = crypt($password, base64_encode($password));
		return $enc_pwd;
	}

	/**
	 * Save user to users configuration file.
	 *
	 * @access public
	 * @param string $user username
	 * @param string $password user's password
	 * @param boolean $firstUsage determine if it is first saved user during first Baculum run
	 * @param mixed $oldUser previous username before change
	 * @return boolean true if user saved successfully, otherwise false
	 */
	public function setUsersConfig($user, $password, $firstUsage = false, $oldUser = null) {
		if($firstUsage === true) {
			$this->clearUsersConfig();
		}

		$allUsers = $this->getAllUsers();
		$password = $this->getCryptedPassword($password);

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

	/**
	 * Read all users from HTTP Basic users file.
	 * Returned value is associative array with usernames as keys
	 * and encrypted passwords as values.
	 *
	 * @access public
	 * @return array users/passwords list
	 */
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

	/**
	 * Save HTTP Basic users file.
	 * Given parameter is associative array with usernames as keys
	 * and encrypted passwords as values.
	 *
	 * @access public
	 * @param array $allUsers users/passwords list
	 * @return boolean true if users file saved successfully, otherwise false
	 */
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

	/**
	 * Remove single user from HTTP Basic users file.
	 * Note, this method saves config file if username was existed
	 * before removing.
	 *
	 * @access public
	 * @param string $username user name to remove
	 * @return boolean true if users file saved successfully, otherwise false
	 */
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
	 * Check if users configuration file exists.
	 * @access public
	 * @return boolean true if file exists, otherwise false
	 */
	public function isUsersConfig() {
		return file_exists(Prado::getPathOfNamespace(self::USERS_FILE, '.users'));
	}

	/**
	 * Clear all content of users file.
	 * @access public
	 * @return boolean true if file cleared successfully, otherwise false
	 */
	public function clearUsersConfig() {
		$usersFile = Prado::getPathOfNamespace(self::USERS_FILE, '.users');
		$result = file_put_contents($usersFile, '') !== false;
		return $result;
	}

	/**
	 * Log in as specific user.
	 *
	 * Note, usually after this method call there required is using exit() just
	 * after method execution. Otherwise the HTTP redirection may be canceled on some
	 * web servers.
	 *
	 * @access public
	 * @param string $http_protocol 'http' or 'https' value
	 * @param string $host hostname without port, for example: my.own.host or localhost
	 * @param integer $port port number on which listens web server
	 * @param string $user user name to log in
	 * @param string $string plain text user's password
	 * @return none
	 */
	public function switchToUser($http_protocol, $host, $port, $user, $password) {
		$urlPrefix = $this->Application->getModule('friendly-url')->getUrlPrefix();
		$location = sprintf("%s://%s:%s@%s:%d%s", $http_protocol, $user, $password, $host, $port, $urlPrefix);
		header("Location: $location");
	}

	/**
	 * Get (pseudo)random string.
	 *
	 * Useful for log out user from HTTP Basic auth by providing random password.
	 *
	 * @access public
	 * @return string random 62 characters string from range [a-zA-Z0-9]
	 */
	public function getRandomString() {
		$characters = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		$rand_string = str_shuffle($characters);
		return $rand_string;
	}
}
?>
