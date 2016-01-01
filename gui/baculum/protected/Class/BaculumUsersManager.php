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

Prado::using('System.Security.IUserManager');
Prado::using('Application.Class.BaculumUser');

class BaculumUsersManager extends TModule implements IUserManager {

	private $config;
	private $configMod;

	public function init($config) {
		$this->configMod = $this->Application->getModule('configuration');
		$this->config = $this->configMod->isApplicationConfig() ? $this->configMod->getApplicationConfig() : null;
	}

	public function getGuestName() {
		return 'guest';
	}

	public function validateUser($username, $password) {
		$valid = false;
		if(!empty($username) && !empty($password)) {
			$users = $this->configMod->getAllUsers();
			$valid = (array_key_exists($username, $users) && $password === $users[$username]);
		}
		return $valid;
	}

	public function getUser($username = null) {
		$user = new BaculumUser($this);
		$user->setIsGuest(false);
		$id = sha1(time());
		$user->setID($id);
		$user->setName($username);
		if (!is_null($username)) {
			$users = $this->configMod->getAllUsers();
			$user->setPwd($users[$username]);
		}
		if(is_null($this->config) || $this->config['baculum']['login'] === $username) {
			$user->setRoles('admin');
		} else {
			$user->setRoles('user');
		}
		return $user;
	}

	public function getUserFromCookie($cookie) {
		$data = $cookie->Value;
		if (!empty($data)) {
			$data = $this->Application->SecurityManager->validateData($data);
			if ($data != false) {
				$data = unserialize($data);
				if (is_array($data) && count($data) === 3) {
					list($username, $address, $token) = $data;
					return $this->getUser($username);
				}
			}
		}
	}

	public function saveUserToCookie($cookie) {
		$address = $this->Application->Request->UserHostAddress;
		$username = $this->User->getName();
		$token = $this->User->getID();
		$data = array($username, $address, $token);
		$data = serialize($data);
		$data = $this->Application->SecurityManager->hashData($data);
		$cookie->setValue($data);
	}

	public function loginUser($user = null, $pwd = null) {
		if (is_null($user) && is_null($pwd) && isset($_SERVER['PHP_AUTH_USER']) && isset($_SERVER['PHP_AUTH_PW'])) {
			$user = $_SERVER['PHP_AUTH_USER'];
			$pwd = $this->Application->getModule('configuration')->getCryptedPassword($_SERVER['PHP_AUTH_PW']);
		}
		$logged = $this->Application->getModule('auth')->login($user, $pwd, 86400);
		return $logged;
	}
}
?>
