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
 
Prado::using('System.Web.UI.TPage');
Prado::using('System.Exceptions.TException');
Prado::using('Application.Common.Class.Errors');
Prado::using('Application.Common.Class.OAuth2');
Prado::using('Application.Common.Class.Logging');
Prado::using('Application.API.Class.BAPIException');
Prado::using('Application.API.Class.APIDbModule');
Prado::using('Application.API.Class.Bconsole');
Prado::using('Application.API.Class.OAuth2.TokenRecord');

/**
 * Abstract module from which inherits each of API module.
 * The module contains methods that are common for all API pages.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 */
abstract class BaculumAPIServer extends TPage {

	/**
	 * API server version (used in HTTP header)
	 */
	const API_SERVER_VERSION = 0.1;

	/**
	 * Storing output from API commands in numeric array.
	 */
	protected $output;

	/**
	 * Storing error from API commands as integer value.
	 */
	protected $error;

	/**
	 * Storing currently used Director name for bconsole commands.
	 */
	protected $director;

	/**
	 * Web interface User name that sent request to API.
	 * Null value means administrator, any other value means normal user
	 * (non-admin user).
	 */
	protected $user;

	/**
	 * Endpoints available for every authenticated client.
	 */
	private $public_endpoints = array('auth', 'token', 'welcome', 'catalog', 'dbsize', 'directors');

	/**
	 * Action methods.
	 */

	// get elements
	const GET_METHOD = 'GET';

	// create new elemenet
	const POST_METHOD = 'POST';

	// update elements
	const PUT_METHOD = 'PUT';

	// delete element
	const DELETE_METHOD = 'DELETE';

	/**
	 * API Server authentication.
	 *
	 * @return true if user is successfully authenticated, otherwise false
	 */
	private function authenticate() {
		$is_auth = false;
		$config = $this->getModule('api_config')->getConfig('api');
		if ($config['auth_type'] === 'basic' && $this->getModule('auth_basic')->isAuthRequest()) {
			$is_auth = true;
		} elseif ($config['auth_type'] === 'oauth2' && $this->getModule('auth_oauth2')->isAuthRequest()) {
			$is_auth = $this->authorize();
		}
		if (!$is_auth && is_null($this->error)) {
			$this->output = AuthenticationError::MSG_ERROR_AUTHENTICATION_TO_API_PROBLEM;
			$this->error = AuthenticationError::ERROR_AUTHENTICATION_TO_API_PROBLEM;
		}
		return $is_auth;
	}

	/**
	 * API Server authorization.
	 * Check if authenticated user is allowed to get requested API endpoint.
	 *
	 * @return true if user is successfully authorized, otherwise false
	 */
	private function authorize() {
		$is_auth = false;
		$is_token = false;

		// deleting expired tokens
		$this->getModule('oauth2_token')->deleteExpiredTokens();

		$auth_oauth2 = $this->getModule('auth_oauth2');

		// Check if token exists
		$scopes = '';
		$token = $auth_oauth2->getToken();
		$auth = TokenRecord::findByPk($token);
		if (is_array($auth)) {
			// Token found
			$scopes = $auth['scope'];
			$is_token = true;
		}

		// Check if requested scope is valid according allowed scopes assigned to token
		if ($is_token) {
			$path = $this->getRequest()->getUrl()->getPath();
			if ($auth_oauth2->isScopeValid($path, $scopes, $this->public_endpoints)) {
				// Authorization valid
				$is_auth = true;
				$this->initAuthParams($auth);
			} else {
				// Scopes error. Access attempt to not allowed resource
				$this->output = AuthorizationError::MSG_ERROR_ACCESS_ATTEMPT_TO_NOT_ALLOWED_RESOURCE .' Endpoint: ' .  $path;
				$this->error = AuthorizationError::ERROR_ACCESS_ATTEMPT_TO_NOT_ALLOWED_RESOURCE;
			}
		}
		return $is_auth;
	}

	/**
	 * Get request, login user and do request action.
	 *
	 * @access public
	 * @param mixed $params onInit action params
	 * @return none
	 */
	public function onInit($params) {
		parent::onInit($params);
		// Initialize auth modules
		$this->getModule('auth_basic')->initialize($this->Request);
		$this->getModule('auth_oauth2')->initialize($this->Request);

		// set Director to bconsole execution
		$this->director = $this->Request->contains('director') ? $this->Request['director'] : null;

		$config = $this->getModule('api_config')->getConfig('api');

		Logging::$debug_enabled = (key_exists('debug', $config) && $config['debug'] == 1);

		if ($this->authenticate() === false) {
			// Authorization error.
			header(OAuth2::HEADER_UNAUTHORIZED);
			return;
		}
		try {
			switch($_SERVER['REQUEST_METHOD']) {
				case self::GET_METHOD: {
					$this->get();
					break;
				}
				case self::POST_METHOD: {
					$this->post();
					break;
				}
				case self::PUT_METHOD: {
					$this->put();
					break;
				}
				case self::DELETE_METHOD: {
					$this->delete();
					break;
				}
			}
		} catch(Prado\Exceptions\TException $e) {
			$this->getModule('logging')->log(
				__FUNCTION__,
				"Method: {$_SERVER['REQUEST_METHOD']} $e",
				Logging::CATEGORY_APPLICATION,
				__FILE__,
				__LINE__
			);
			if ($e instanceof BAPIException) {
				$this->output = $e->getErrorMessage();
				$this->error = $e->getErrorCode();
			} else {
				$this->output = GenericError::MSG_ERROR_INTERNAL_ERROR . ' ' . $e->getErrorMessage();
				$this->error = GenericError::ERROR_INTERNAL_ERROR;
			}
		} 
	}

	/**
	 * Initialize auth parameters.
	 *
	 * @param array $auth token params stored in TokenRecord session
	 * @return none
	 */
	private function initAuthParams(array $auth) {
		// if client has own bconsole config, assign it here
		if (array_key_exists('bconsole_cfg_path', $auth) && !empty($auth['bconsole_cfg_path'])) {
			Bconsole::setCfgPath($auth['bconsole_cfg_path'], true);
		}
	}

	/**
	 * Get request result data and pack it in JSON format.
	 * JSON values are: {
	 * "output": (list) output values
	 * "error" : (integer) result exit code (0 - OK, non-zero - error)
	 *
	 * @access private
	 * @return string JSON value with output and error values
	 */
	private function getOutput() {
		$output = array('output' => $this->output, 'error' => $this->error);
		$this->setOutputHeaders();
		$json = json_encode($output);
		return $json;
	}

	/**
	 * Set output headers to send in response.
	 */
	private function setOutputHeaders() {
		$response = $this->getResponse();
		$response->setContentType('application/json');
		$response->appendHeader('Baculum-API-Version: ' . strval(self::API_SERVER_VERSION));
	}

	/**
	 * Return action result which was realized in onInit() method.
	 * On standard output is printed JSON value with request results.
	 *
	 * @access public
	 * @param mixed $params onInit action params
	 * @return none
	 */
	public function onLoad($params) {
		parent::onLoad($params);
		echo $this->getOutput();
	}

	/**
	 * Changing/updating values via API.
	 *
	 * @access private
	 * @return none
	 */
	private function put() {
		$id = $this->Request->contains('id') ? intval($this->Request['id']) : 0;

		/**
		 * Check if it is possible to read PUT method data.
		 * Note that some clients sends data in PUT request as PHP input stream which
		 * is not possible to read by $_REQUEST data. From this reason, when is
		 * not possible to ready by superglobal $_REQUEST variable, then is try to
		 * read PUT data by PHP input stream.
		 */
		if ($this->Request->contains('update') && is_array($this->Request['update']) && count($this->Request['update']) > 0) {
			// $_REQUEST available to read
			$params = (object)$this->Request['update'];
			$this->set($id, $params);
		} else {
			// no possibility to read data from $_REQUEST. Try to load from input stream.
			$inputstr = file_get_contents("php://input");

			/**
			 * Read using chunks for case large updates (over 1000 values).
			 * Otherwise max_input_vars limitation in php.ini can be reached (usually
			 * set to 1000 variables)
			 * @see http://php.net/manual/en/info.configuration.php#ini.max-input-vars
			 */
			$chunks = explode('&', $inputstr);

			$response_data = array();
			for($i = 0; $i<count($chunks); $i++) {
				// if chunks would not be used, then here occurs reach max_input_vars limit
				parse_str($chunks[$i], $response_el);
				if (is_array($response_el) && array_key_exists('update', $response_el) && is_array($response_el['update'])) {
					$key = key($response_el['update']);
					$response_data['update'][$key] = $response_el['update'][$key];
				}
			}
			if (is_array($response_data) && array_key_exists('update', $response_data)) {
				$params = (object)$response_data['update'];
				$this->set($id, $params);
			} else {
				/**
				 * This case should never occur because it means that there is
				 * given nothing to update.
				 */
				$params = new stdClass;
				$this->set($id, $params);
			}
		}
	}

	/**
	 * Creating new elements.
	 *
	 * @access private
	 * @return none
	 */
	private function post() {
		$params = new stdClass;
		if ($this->Request->contains('create') && is_array($this->Request['create']) && count($this->Request['create']) > 0) {
			$params = (object)$this->Request['create'];
		}
		$this->create($params);
	}

	/**
	 * Deleting element by element ID.
	 *
	 * @access private
	 * @return none
	 */
	private function delete() {
		$id = null;
		if ($this->Request->contains('id')) {
			$id = intval($this->Request['id']);
		}
		$this->remove($id);
	}

	/**
	 * Shortcut method for getting application modules instances by
	 * module name.
	 *
	 * @access public
	 * @param string $name application module name
	 * @return object module class instance
	 */
	public function getModule($name) {
		return $this->Application->getModule($name);
	}

	/**
	 * Get Baculum web client version.
	 *
	 * @return float client version
	 */
	public function getClientVersion() {
		$version = 0;
		$headers = $this->getRequest()->getHeaders(CASE_LOWER);
		if (array_key_exists('x-baculum-api', $headers)) {
			$version = floatval($headers['x-baculum-api']);
		}
		return $version;
	}
}
?>
