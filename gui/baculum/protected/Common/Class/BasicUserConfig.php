<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2019 Kern Sibbald
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

Prado::using('Application.Common.Class.CommonModule');

/**
 * Manage basic auth method users.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Module
 * @package Baculum Common
 */
abstract class BasicUserConfig extends CommonModule {
	
	/**
	 * User name allowed characters pattern
	 */
	const USER_PATTERN = '[a-zA-Z0-9]+';

	/**
	 * Get config file path to store users' parameters.
	 * @return string config path
	 */
	abstract protected function getConfigPath();

	/**
	 * Save user to users configuration file.
	 *
	 * @access public
	 * @param string $user username
	 * @param string $password user's password
	 * @param boolean $clear_config determine if clear config before save
	 * @param mixed $old_user previous username before change
	 * @return boolean true if user saved successfully, otherwise false
	 */
	public function setUsersConfig($user, $password, $clear_config = false, $old_user = null) {
		if ($clear_config === true) {
			$this->clearUsersConfig();
		}

		$all_users = $this->getAllUsers();
		$password = $this->getModule('misc')->getHashedPassword($password);

		$userExists = array_key_exists($user, $all_users);


		if ($userExists === true) {
			// update user password;
			$all_users[$user] = $password;
		}

		if (!is_null($old_user) && $old_user !== $user) {
			// delete old username with password from configuration file
			if (array_key_exists($old_user, $all_users)) {
				unset($all_users[$old_user]);
			}
		}

		// add new user if does not exist
		if ($userExists === false) {
			$all_users[$user] = $password;
		}

		$result = $this->saveUserConfig($all_users);
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
		$all_users = array();
		if ($this->isUsersConfig() === true) {
			$users = file($this->getConfigPath(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

			for($i = 0; $i < count($users); $i++) {
				if (preg_match("/^(?P<user>\S+)\:(?P<hash>\S+)$/", $users[$i], $match) === 1) {
					$all_users[$match['user']] = $match['hash'];
				}
			}
		}
		return $all_users;
	}

	/**
	 * Save HTTP Basic users file.
	 * Given parameter is associative array with usernames as keys
	 * and encrypted passwords as values.
	 *
	 * @access public
	 * @param array $all_users users/passwords list
	 * @return boolean true if users file saved successfully, otherwise false
	 */
	public function saveUserConfig($all_users) {
		$users = array();
		foreach ($all_users as $user => $pwd) {
			$users[] = "$user:$pwd";
		}
		$usersToFile = implode("\n", $users);
		$old_umask = umask(0);
		umask(0077);
		$result = file_put_contents($this->getConfigPath(), $usersToFile) !== false;
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
		$all_users = $this->getAllUsers();
		if (array_key_exists($username, $all_users)) {
			unset($all_users[$username]);
			$result = $this->saveUserConfig($all_users);
		}
		return $result;
	}

	/**
	 * Check if users configuration file exists.
	 *
	 * @access public
	 * @return boolean true if file exists, otherwise false
	 */
	public function isUsersConfig() {
		return file_exists($this->getConfigPath());
	}

	/**
	 * Clear all content of users file.
	 *
	 * @access public
	 * @return boolean true if file cleared successfully, otherwise false
	 */
	public function clearUsersConfig() {
		$result = file_put_contents($this->getConfigPath(), '') !== false;
		return $result;
	}
}
