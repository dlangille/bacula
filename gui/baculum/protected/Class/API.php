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

Prado::using('Application.Class.Errors');

/**
 * Internal API client module.
 *
 * @author Marcin Haba
 */
class API extends TModule {

	/**
	 * API version (used in HTTP header)
	 */
	const API_VERSION = '0.1';

	/**
	 * Store configuration data from settings file
	 *
	 * @access protected
	 */
	protected $app_cfg;

	/**
	 * These errors are allowed in API response and they do not cause
	 * disturb application working (no direction to error page)
	 *
	 * @access private
	 */
	private $allowed_errors = array(
		GenericError::ERROR_NO_ERRORS,
		BconsoleError::ERROR_INVALID_COMMAND,
		PoolError::ERROR_NO_VOLUMES_IN_POOL_TO_UPDATE
	);

	/**
	 * Get connection request handler.
	 * For data requests is used cURL interface.
	 *
	 * @access public
	 * @return resource connection handler on success, false on errors
	 */
	public function getConnection() {
		$ch = curl_init();
		$userpwd = sprintf('%s:%s', $this->app_cfg['baculum']['login'], $this->app_cfg['baculum']['password']);
		curl_setopt($ch, CURLOPT_USERPWD, $userpwd);
		curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_COOKIE, 'PHPSESSID=' . md5(session_id()));
		return $ch;
	}

	/**
	 * Get API specific headers used in HTTP requests.
	 *
	 * @access private
	 * @return API specific headers
	 */
	private function getAPIHeaders() {
		$headers = array(
			'X-Baculum-API: ' . self::API_VERSION,
			'X-Baculum-User: ' . $this->Application->User->getName(),
			'X-Baculum-Pwd: ' . $this->Application->User->getPwd(),
			'Accept: application/json'
		);
		return $headers;
	}

	/**
	 * Initializes API module (framework module constructor)
	 *
	 * @access public
	 * @param TXmlElement $config API module configuration
	 */
	public function init($config) {
		$this->initSessionCache();
		$this->app_cfg = $this->Application->getModule('configuration')->getApplicationConfig();
	}

	/**
	 * Get URL to use by internal API client's request.
	 *
	 * @access private
	 * @return string URL to internal API server
	 */
	private function getURL() {
		$protocol = !empty($_SERVER['HTTPS']) ? 'https' : 'http';
		$host = $_SERVER['SERVER_NAME'];
		$port = $_SERVER['SERVER_PORT'];

		// support for document root subdirectory
		$urlPrefix = $this->Application->getModule('friendly-url')->getUrlPrefix();

		$url = sprintf('%s://%s:%d%s/', $protocol, $host, $port, $urlPrefix);
		return $url;
	}

	/**
	 * Set URL parameters and prepare URL to request send.
	 *
	 * @access private
	 * @param string &$url reference to URL string variable
	 */
	private function setUrlParams(&$url) {
		$url .= (preg_match('/\?/', $url) === 1 ? '&' : '?');
		$url .= 'director=';
		if (array_key_exists('director', $_SESSION)) {
			$url .= $_SESSION['director'];
		}

		$this->Application->getModule('logging')->log(
			__FUNCTION__,
			PHP_EOL . PHP_EOL . 'EXECUTE URL ==> ' . $url . ' <==' . PHP_EOL . PHP_EOL,
			Logging::CATEGORY_APPLICATION,
			__FILE__,
			__LINE__
		);
	}

	/**
	 * Internal API GET request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @param bool $use_cache if true then try to use session cache, if false then always use fresh data
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function get(array $params, $use_cache = false) {
		$cached = null;
		$ret = null;
		if ($use_cache === true) {
			$cached = $this->getSessionCache($params);
		}
		if (!is_null($cached)) {
			$ret = $cached;
		} else {
			$url = $this->getURL() . implode('/', $params);
			$this->setUrlParams($url);
			$ch = $this->getConnection();
			curl_setopt($ch, CURLOPT_URL, $url);
			curl_setopt($ch, CURLOPT_HTTPHEADER, $this->getAPIHeaders());
			$result = curl_exec($ch);
			curl_close($ch);
			$ret = $this->preParseOutput($result);
			if ($use_cache === true && $ret->error === 0) {
				$this->setSessionCache($params, $ret);
			}
		}
		return $ret;
	}

	/**
	 * Internal API SET request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @param array $options POST params to send in request
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function set(array $params, array $options) {
		$url = $this->getURL() . implode('/', $params);
		$this->setUrlParams($url);
		$data = http_build_query(array('update' => $options));
		$ch = $this->getConnection();
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PUT');
		curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge(
			$this->getAPIHeaders(),
			array('X-HTTP-Method-Override: PUT', 'Content-Length: ' . strlen($data), 'Expect:')
		));
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
		$result = curl_exec($ch);
		curl_close($ch);
		return $this->preParseOutput($result);
	}

	/**
	 * Internal API CREATE request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @param array $options POST params to send in request
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function create(array $params, array $options) {
		$url = $this->getURL() . implode('/', $params);
		$this->setUrlParams($url);
		$data = http_build_query(array('create' => $options));
		$ch = $this->getConnection();
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge($this->getAPIHeaders(), array('Expect:')));
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
		$result = curl_exec($ch);
		curl_close($ch);
		return $this->preParseOutput($result);
	}

	/**
	 * Internal API REMOVE request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function remove(array $params) {
		$url = $this->getURL() . implode('/', $params);
		$this->setUrlParams($url);
		$ch = $this->getConnection();
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
		curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge($this->getAPIHeaders(), array('X-HTTP-Method-Override: DELETE')));
		$result = curl_exec($ch);
		curl_close($ch);
		return $this->preParseOutput($result);
	}

	/**
	 * Initially parse and prepare every Internal API response.
	 * If a error occurs then redirect to appropriate error page.
	 *
	 * @access private
	 * @param string $result response output as JSON string (not object yet)
	 * @return object stdClass parsed response with two top level properties 'output' and 'error'
	 */
	private function preParseOutput($result) {
		// first write log with that what comes
		$this->Application->getModule('logging')->log(
			__FUNCTION__,
			$result,
			Logging::CATEGORY_APPLICATION,
			__FILE__,
			__LINE__
		);

		// decode JSON to object
		$resource = json_decode($result);

		$error = null;

		if(is_object($resource) && property_exists($resource, 'error')) {
			if(!in_array($resource->error, $this->allowed_errors)) {
				$error = $resource->error;
			}
		} else {
			$error = AuthorizationError::ERROR_AUTHORIZATION_TO_WEBGUI_PROBLEM;
		}

		$this->Application->getModule('logging')->log(
			__FUNCTION__,
			$resource,
			Logging::CATEGORY_APPLICATION,
			__FILE__,
			__LINE__
		);

		// if other than allowed errors exist then show error page (redirect)
		if(!is_null($error)) {
			// Note! Redirection to error page takes place here.
			$this->Response->redirect(
				$this->Service->constructUrl(
					'BaculumError',
					array('error' => $error),
					false
				)
			);
		}

		return $resource;
	}

	/**
	 * Initialize session cache.
	 *
	 * @access public
	 * @param bool $force if true then cache is force initialized
	 * @return none
	 */
	public function initSessionCache($force = false) {
		if (!isset($_SESSION) || !array_key_exists('cache', $_SESSION) || !is_array($_SESSION['cache']) || $force === true) {
			$_SESSION['cache'] = array();
		}
	}

	/**
	 * Get session cache value by params.
	 *
	 * @access private
	 * @param array $params command parameters as numeric array
	 * @return mixed if cache exists then returned is cached data, otherwise null
	 */
	private function getSessionCache(array $params) {
		$cached = null;
		$key = $this->getSessionKey($params);
		if ($this->isSessionValue($key)) {
			$cached = $_SESSION['cache'][$key];
		}
		return $cached;
	}

	/**
	 * Save data to session cache.
	 *
	 * @access private
	 * @param array $params command parameters as numeric array
	 * @param mixed $value value to save in cache
	 * @return none
	 */
	private function setSessionCache(array $params, $value) {
		$key = $this->getSessionKey($params);
		$_SESSION['cache'][$key] = $value;
	}

	/**
	 * Get session key by command parameters.
	 *
	 * @access private
	 * @param array $params command parameters as numeric array
	 * @return string session key for given command
	 */
	private function getSessionKey(array $params) {
		$key = implode(';', $params);
		$key = base64_encode($key);
		return $key;
	}

	/**
	 * Check if session key exists in session cache.
	 *
	 * @access private
	 * @param string $key session key
	 * @return bool true if session key exists, otherwise false
	 */
	private function isSessionValue($key) {
		$is_value = array_key_exists($key, $_SESSION['cache']);
		return $is_value;
	}
}
?>
