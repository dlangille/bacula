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

Prado::using('Application.Common.Class.Interfaces');
Prado::using('Application.Common.Class.SessionRecord');

class HostRecord extends SessionRecord implements SessionItem {

	public $host;
	public $protocol;
	public $address;
	public $port;
	public $url_prefix;
	public $auth_type;
	public $login;
	public $password;
	public $client_id;
	public $client_secret;
	public $redirect_uri;
	public $scope;

	public function __construct($host = null, $params = array()) {
		parent::__construct();
		if (!is_null($host) && is_array($params)) {
			$this->setHost($host, $params);
		}
	}

	public static function getRecordId() {
		return 'host_params';
	}

	public static function getPrimaryKey() {
		return 'host';
	}

	public static function getSessionFile() {
		return Prado::getPathOfNamespace('Application.Web.Config.session', '.dump');
	}

	/**
	 * Set host in session.
	 *
	 * @public
	 * @param string $host host name in config
	 * @param array $params host parameters in associative array
	 * @return none
	 */
	public function setHost($host, array $params) {
		$this->host = $host;
		$this->protocol = array_key_exists('protocol', $params) ? $params['protocol'] : 'https';
		$this->address = array_key_exists('address', $params) ? $params['address'] : '';
		$this->port = array_key_exists('port', $params) ? $params['port'] : null;
		$this->url_prefix = array_key_exists('url_prefix', $params) ? $params['url_prefix'] : '';
		if (array_key_exists('auth_type', $params)) {
			$this->auth_type =  $params['auth_type'];
			if ($params['auth_type'] === 'basic') {
				$this->login = array_key_exists('login', $params) ? $params['login'] : '';
				$this->password = array_key_exists('password', $params) ? $params['password'] : '';
			} elseif ($params['auth_type'] === 'oauth2') {
				$this->client_id = array_key_exists('client_id', $params) ? $params['client_id'] : '';
				$this->client_secret = array_key_exists('client_secret', $params) ? $params['client_secret'] : '';
				$this->redirect_uri = array_key_exists('redirect_uri', $params) ? $params['redirect_uri'] : '';
				$this->scope = array_key_exists('scope', $params) ? $params['scope'] : '';
			}
		}
		$this->save();
	}
}
