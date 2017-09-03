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

Prado::using('Application.Common.Class.Errors');
Prado::using('Application.Common.Class.Miscellaneous');
Prado::using('Application.Web.Class.WebModule');
Prado::using('Application.Web.Class.HostConfig');
Prado::using('Application.Web.Class.OAuth2Record');
Prado::using('Application.Web.Class.HostRecord');

/**
 * Internal API client module.
 *
 * @author Marcin Haba
 */
class BaculumAPIClient extends WebModule {

	/**
	 * API version (used in HTTP header)
	 */
	const API_VERSION = '0.2';

	/**
	 * OAuth2 authorization endpoints
	 */
	const OAUTH2_AUTH_URL = 'api/auth/';
	const OAUTH2_TOKEN_URL = 'api/token/';

	/**
	 * Session params to put in URLs.
	 *
	 * @access private
	 */
	private $session_params = array('director');

	/**
	 * Host params to do API requests.
	 *
	 * @access private
	 */
	private $host_params = array();

	/**
	 * Params used to authentication.
	 */
	private $auth_params = array();

	/**
	 * Get connection request handler.
	 * For data requests is used cURL interface.
	 *
	 * @access public
	 * @param array $host_cfg host config parameters
	 * @return resource connection handler on success, false on errors
	 */
	public function getConnection(array $host_cfg) {
		$ch = curl_init();
		if (count($host_cfg) > 0 && $host_cfg['auth_type'] === 'basic') {
			$userpwd = sprintf('%s:%s', $host_cfg['login'], $host_cfg['password']);
			curl_setopt($ch, CURLOPT_USERPWD, $userpwd);
		}
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
	private function getAPIHeaders($host = null, $host_cfg = array()) {
		$headers = array(
			'X-Baculum-API: ' . self::API_VERSION,
			'Accept: application/json'
		);
		if (count($host_cfg) > 0 && !is_null($host) && $host_cfg['auth_type'] === 'oauth2') {
			$now = time();
			$auth = OAuth2Record::findByPk($host);
			if (is_null($auth) || (is_array($auth) && $now >= $auth['refresh_time'])) {
				if (is_array($auth)) {
					OAuth2Record::deleteByPk($host);
				}
				$this->authToHost($host, $host_cfg);
				$auth = OAuth2Record::findByPk($host);
			}
			if (is_array($auth) && array_key_exists('tokens', $auth)) {
				$headers[] = "Authorization: {$auth['tokens']['token_type']} {$auth['tokens']['access_token']}";
			}
		}
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
	}

	/**
	 * Get URI to use by internal API client's request.
	 *
	 * @access private
	 * @param string $host host name
	 * @param array $params GET params to send in request
	 * @return string URI to internal API server
	 */
	private function getURIResource($host, array $params) {
		$uri = null;
		$host_cfg = $this->getHostParams($host);
		if (count($host_cfg) > 0) {
			$uri = $this->getBaseURI($host_cfg);

			// API URLs start with /api/
			array_unshift($params, 'api');

			// Add GET params to URI
			$uri .= $this->prepareUrlParams($params);

			// Add special params to URI
			$this->addSpecialParams($uri);

			$this->Application->getModule('logging')->log(
				__FUNCTION__,
				PHP_EOL . PHP_EOL . 'EXECUTE URI ==> ' . $uri . ' <==' . PHP_EOL . PHP_EOL,
				Logging::CATEGORY_APPLICATION,
				__FILE__,
				__LINE__
			);
		}
		return $uri;
	}

	private function getURIAuth($host, array $params, $endpoint) {
		$uri = null;
		$host_cfg = $this->getHostParams($host);
		if (count($host_cfg) > 0) {
			$uri = $this->getBaseURI($host_cfg);
			// add auth endpoint
			$uri .= $endpoint;
			foreach ($params as $key => $value) {
				// add params separator
				$uri .= (preg_match('/\?/', $uri) === 1 ? '&' : '?');
				$params = array($key, $value);
				// add auth param
				$uri .= $this->prepareUrlParams($params, '=');
			}
		}
		return $uri;
	}

	private function getBaseURI($host_cfg) {
		$uri = sprintf('%s://%s:%d%s/',
			$host_cfg['protocol'],
			$host_cfg['address'],
			$host_cfg['port'],
			$host_cfg['url_prefix']
		);
		return $uri;
	}

	/**
	 * Get host specific params to put in URI.
	 *
	 * @access protected
	 * @param string $host host name
	 * @return array host parameters
	 */
	protected function getHostParams($host) {
		$host_params = HostRecord::findByPk($host);
		if (is_null($host_params)) {
			$host_params = $this->getModule('host_config')->getHostConfig($host);
		}
		return $host_params;
	}

	/**
	 * Set/prepare host specific params.
	 *
	 * @access public
	 * @param string $host host name
	 * @param array $params host parameters
	 * @return none
	 */
	public function setHostParams($host, $params) {
		new HostRecord($host, $params);
	}

	/**
	 * Prepare GET params to put in URI as string.
	 *
	 * @param array $params GET parameters
	 * @param string $separator char used as glue to join params
	 * @return string parameters ready to put in URI
	 */
	private function prepareUrlParams(array $params, $separator = '/') {
		$params_encoded = array_map('rawurlencode', $params);
		$params_url = implode($separator, $params_encoded);
		return $params_url;
	}

	/**
	 * Add special params to URI.
	 *
	 * @access private
	 * @param string &$uri reference to URI string variable
	 */
	private function addSpecialParams(&$uri) {
		// add special session params
		for ($i = 0; $i < count($this->session_params); $i++) {
			if (array_key_exists($this->session_params[$i], $_SESSION)) {
				// add params separator
				$uri .= (preg_match('/\?/', $uri) === 1 ? '&' : '?');
				$params = array($this->session_params[$i], $_SESSION[$this->session_params[$i]]);
				// add session param
				$uri .= $this->prepareUrlParams($params, '=');
			}
		}
	}

	/**
	 * Internal API GET request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @param string $host host name to send request
	 * @param bool $show_error if true then it shows error as HTML error page
	 * @param bool $use_cache if true then try to use session cache, if false then always use fresh data
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function get(array $params, $host = null, $show_error = true, $use_cache = false) {
		$cached = null;
		$ret = null;
		if (is_null($host)) {
			if (isset($_SESSION['api_host'])) {
				$host = $_SESSION['api_host'];
			} else {
				$host = HostConfig::MAIN_CATALOG_HOST;
			}
		}
		if ($use_cache === true) {
			$cached = $this->getSessionCache($host, $params);
		}
		if (!is_null($cached)) {
			$ret = $cached;
		} else {
			$host_cfg = $this->getHostParams($host);
			$uri = $this->getURIResource($host, $params);
			$ch = $this->getConnection($host_cfg);
			curl_setopt($ch, CURLOPT_URL, $uri);
			curl_setopt($ch, CURLOPT_HTTPHEADER, $this->getAPIHeaders($host, $host_cfg));
			$result = curl_exec($ch);
			$error = curl_error($ch);
			$errno = curl_errno($ch);
			curl_close($ch);
			$ret = $this->preParseOutput($result, $error, $errno, $show_error);
			if ($use_cache === true && $ret->error === 0) {
				$this->setSessionCache($host, $params, $ret);
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
	 * @param string $host host name to send request
	 * @param bool $show_error if true then it shows error as HTML error page
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function set(array $params, array $options, $host = null, $show_error = true) {
		if (is_null($host)) {
			if (isset($_SESSION['api_host'])) {
				$host = $_SESSION['api_host'];
			} else {
				$host = HostConfig::MAIN_CATALOG_HOST;
			}
		}
		$host_cfg = $this->getHostParams($host);
		$uri = $this->getURIResource($host, $params);
		$ch = $this->getConnection($host_cfg);
		$data = http_build_query(array('update' => $options));
		curl_setopt($ch, CURLOPT_URL, $uri);
		curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PUT');
		curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge(
			$this->getAPIHeaders($host, $host_cfg),
			array('X-HTTP-Method-Override: PUT', 'Content-Length: ' . strlen($data), 'Expect:')
		));
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
		$result = curl_exec($ch);
		$error = curl_error($ch);
		$errno = curl_errno($ch);
		curl_close($ch);
		return $this->preParseOutput($result, $error, $errno, $show_error);
	}

	/**
	 * Internal API CREATE request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @param array $options POST params to send in request
	 * @param string $host host name to send request
	 * @param bool $show_error if true then it shows error as HTML error page
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function create(array $params, array $options, $host = null, $show_error = true) {
		if (is_null($host)) {
			if (isset($_SESSION['api_host'])) {
				$host = $_SESSION['api_host'];
			} else {
				$host = HostConfig::MAIN_CATALOG_HOST;
			}
		}
		$host_cfg = $this->getHostParams($host);
		$uri = $this->getURIResource($host, $params);
		$ch = $this->getConnection($host_cfg);
		$data = http_build_query(array('create' => $options));
		curl_setopt($ch, CURLOPT_URL, $uri);
		curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge($this->getAPIHeaders($host, $host_cfg), array('Expect:')));
		curl_setopt($ch, CURLOPT_POST, true);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
		$result = curl_exec($ch);
		$error = curl_error($ch);
		$errno = curl_errno($ch);
		curl_close($ch);
		return $this->preParseOutput($result, $error, $errno, $show_error);
	}

	/**
	 * Internal API REMOVE request.
	 *
	 * @access public
	 * @param array $params GET params to send in request
	 * @param string $host host name to send request
	 * @param bool $show_error if true then it shows error as HTML error page
	 * @return object stdClass with request result as two properties: 'output' and 'error'
	 */
	public function remove(array $params, $host = null, $show_error = true) {
		if (is_null($host)) {
			if (isset($_SESSION['api_host'])) {
				$host = $_SESSION['api_host'];
			} else {
				$host = HostConfig::MAIN_CATALOG_HOST;
			}
		}
		$host_cfg = $this->getHostParams($host);
		$uri = $this->getURIResource($host, $params);
		$ch = $this->getConnection($host_cfg);
		curl_setopt($ch, CURLOPT_URL, $uri);
		curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
		curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge($this->getAPIHeaders($host, $host_cfg), array('X-HTTP-Method-Override: DELETE')));
		$result = curl_exec($ch);
		$error = curl_error($ch);
		$errno = curl_errno($ch);
		curl_close($ch);
		return $this->preParseOutput($result, $error, $errno, $show_error);
	}

	/**
	 * Initially parse and prepare every Internal API response.
	 * If a error occurs then redirect to appropriate error page.
	 *
	 * @access private
	 * @param string $result response output as JSON string (not object yet)
	 * @param string $error error message from remote host
	 * @param integer $errno error number from remote host
	 * @param bool $show_error if true then it shows error as HTML error page
	 * @return object stdClass parsed response with two top level properties 'output' and 'error'
	 */
	private function preParseOutput($result, $error, $errno, $show_error = true) {
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
		if (is_null($resource)) {
			$resource = (object) array(
				'error' => ConnectionError::ERROR_CONNECTION_TO_HOST_PROBLEM,
				'output' => ConnectionError::MSG_ERROR_CONNECTION_TO_HOST_PROBLEM . " cURL error $errno: $error"
			);
		}

		if ($show_error === true && $resource->error != 0) {
			$url = $this->Service->constructUrl('BaculumError', (array)$resource, false);
			$this->getResponse()->redirect($url);
		}

		$this->Application->getModule('logging')->log(
			__FUNCTION__,
			$resource,
			Logging::CATEGORY_APPLICATION,
			__FILE__,
			__LINE__
		);

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
	 * @param string $host host name
	 * @param array $params command parameters as numeric array
	 * @return mixed if cache exists then returned is cached data, otherwise null
	 */
	private function getSessionCache($host, array $params) {
		$cached = null;
		$key = $this->getSessionKey($host, $params);
		if ($this->isSessionValue($key)) {
			$cached = $_SESSION['cache'][$key];
		}
		return $cached;
	}

	/**
	 * Save data to session cache.
	 *
	 * @access private
	 * @param string $host host name
	 * @param array $params command parameters as numeric array
	 * @param mixed $value value to save in cache
	 * @return none
	 */
	private function setSessionCache($host, array $params, $value) {
		$key = $this->getSessionKey($host, $params);
		$_SESSION['cache'][$key] = $value;
	}

	/**
	 * Get session key by command parameters.
	 *
	 * @access private
	 * @param string $host host name
	 * @param array $params command parameters as numeric array
	 * @return string session key for given command
	 */
	private function getSessionKey($host, array $params) {
		array_unshift($params, $host);
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

	private function authToHost($host, $host_cfg) {
		if (count($host_cfg) > 0 && $host_cfg['auth_type'] === 'oauth2') {
			$state = $this->getModule('misc')->getRandomString(16);
			$params = array(
				'response_type' => 'code',
				'client_id' => $host_cfg['client_id'],
				'redirect_uri' => $host_cfg['redirect_uri'],
				'scope' => $host_cfg['scope'],
				'state' => $state
			);
			$auth = new OAuth2Record();
			$auth->host = $host;
			$auth->state = $state;
			$auth->save();
			$uri = $this->getURIAuth($host, $params, self::OAUTH2_AUTH_URL);
			$ch = $this->getConnection($host_cfg);
			curl_setopt($ch, CURLOPT_URL, $uri);
			curl_setopt($ch, CURLOPT_HTTPHEADER, $this->getAPIHeaders());
			curl_setopt($ch, CURLINFO_HEADER_OUT, true);
			curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
			$result = curl_exec($ch);
			curl_close($ch);
			OAuth2Record::forceRefresh();
		}
	}

	public function getTokens($auth_id, $state) {
		$st = OAuth2Record::findBy('state', $state);
		if (is_array($st)) {
			$host_cfg = $this->getHostParams($st['host']);
			$uri = $this->getURIAuth($st['host'], array(), self::OAUTH2_TOKEN_URL);
			$ch = $this->getConnection($host_cfg);
			$data = array(
				'grant_type' => 'authorization_code',
				'code' => $auth_id,
				'redirect_uri' => $host_cfg['redirect_uri'],
				'client_id' => $host_cfg['client_id'],
				'client_secret' => $host_cfg['client_secret']
			);
			curl_setopt($ch, CURLOPT_URL, $uri);
			curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge($this->getAPIHeaders(), array('Expect:')));
			curl_setopt($ch, CURLOPT_POST, true);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
			$result = curl_exec($ch);
			curl_close($ch);
			$tokens = json_decode($result);
			if (is_object($tokens) && isset($tokens->access_token) && isset($tokens->refresh_token)) {
				$auth = new OAuth2Record();
				$auth->host = $st['host'];
				$auth->tokens = (array)$tokens;
				// refresh token 5 seconds before average expires time
				$auth->refresh_time = time() + $tokens->expires_in - 5;
				$auth->save();
			}
			// Host config in session is no longer needed, so remove it
			HostRecord::deleteByPk($st['host']);
		}
	}
}
?>