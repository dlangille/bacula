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
 
Prado::using('System.Exceptions.TException');
Prado::using('Application.Class.Errors');

/**
 * Abstract module from which inherits each of API module.
 * The module contains methods that are common for all API pages.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 */
abstract class BaculumAPI extends TPage {

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
	 * Get request, login user and do request action.
	 *
	 * @access public
	 * @param mixed $params onInit action params
	 * @return none
	 */
	public function onInit($params) {
		parent::onInit($params);
		/*
		 * Workaround to bug in PHP 5.6 by FastCGI that caused general protection error.
		 * TODO: Check on newer PHP if it is already fixed.
		 */
		$db = new ActiveRecord();
		$db->getDbConnection();

		// set Director to bconsole execution
		$this->director = isset($this->Request['director']) ? $this->Request['director'] : null;

		/**
		 * User and password are obligatory for each request. Otherwise authorization
		 * error takes place.
		 * Password is provided in hashed form.
		 */
		$user = isset($_SERVER['HTTP_X_BACULUM_USER']) ? $_SERVER['HTTP_X_BACULUM_USER']: null;
		$pwd_hash = isset($_SERVER['HTTP_X_BACULUM_PWD']) ? $_SERVER['HTTP_X_BACULUM_PWD']: null;
		if (!is_null($user) && !is_null($pwd_hash)) {
			// try to log in user
			$logged = $this->getModule('users')->loginUser($user, $pwd_hash);
			if ($logged === true) {
				/*
				 * User and password are valid.
				 * Log in action finished successfuly.
				 * Now check if logged in user is admin or normal user.
				 * Admin value is null. Normal user value is string with the user name.
				 */
				$this->user = ($this->User->getIsAdmin() === false) ? $user : null;
			} else {
				// Invalid credentials. Authorization error.
				$this->output = AuthorizationError::MSG_ERROR_AUTHORIZATION_TO_WEBGUI_PROBLEM;
				$this->error = AuthorizationError::ERROR_AUTHORIZATION_TO_WEBGUI_PROBLEM;
				return;
			}
		} else {
			// Not provided user or password. Authorization error.
			$this->output = AuthorizationError::MSG_ERROR_AUTHORIZATION_TO_WEBGUI_PROBLEM;
			$this->error = AuthorizationError::ERROR_AUTHORIZATION_TO_WEBGUI_PROBLEM;
			return;
		}

		switch($_SERVER['REQUEST_METHOD']) {
			case self::PUT_METHOD: {
				try {
					$this->put();
				} catch(TDbException $e) {
					$this->getModule('logging')->log(
						__FUNCTION__,
						$e,
						Logging::CATEGORY_APPLICATION,
						__FILE__,
						__LINE__
					);
					$this->output = DatabaseError::MSG_ERROR_DB_CONNECTION_PROBLEM;
					$this->error = DatabaseError::ERROR_DB_CONNECTION_PROBLEM;
				}
				break;
			}
			case self::GET_METHOD: {
				try {
					$this->get();
				} catch(TDbException $e) {
					$this->getModule('logging')->log(
						__FUNCTION__,
						$e,
						Logging::CATEGORY_APPLICATION,
						__FILE__,
						__LINE__
					);
					$this->output = DatabaseError::MSG_ERROR_DB_CONNECTION_PROBLEM;
					$this->error = DatabaseError::ERROR_DB_CONNECTION_PROBLEM;
				}
				break;
			}
			case self::POST_METHOD: {
				try {
					$this->post();
				} catch(TDbException $e) {
					$this->getModule('logging')->log(
						__FUNCTION__,
						$e,
						Logging::CATEGORY_APPLICATION,
						__FILE__,
						__LINE__
					);
					$this->output = DatabaseError::MSG_ERROR_DB_CONNECTION_PROBLEM;
					$this->error = DatabaseError::ERROR_DB_CONNECTION_PROBLEM;
				}
				break;
			}
			case self::DELETE_METHOD: {
				try {
					$this->delete();
				} catch(TDbException $e) {
					$this->getModule('logging')->log(
						__FUNCTION__,
						$e,
						Logging::CATEGORY_APPLICATION,
						__FILE__,
						__LINE__
					);
					$this->output = DatabaseError::MSG_ERROR_DB_CONNECTION_PROBLEM;
					$this->error = DatabaseError::ERROR_DB_CONNECTION_PROBLEM;
				}
				break;
			}
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
		$json = json_encode($output);
		return $json;
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
	 * Each of API module should have get() method defined.
	 * Designed to getting data from API.
	 *
	 * @access protected
	 * @return none
	 */
	abstract protected function get();

	/**
	 * Changing/updating values via API.
	 *
	 * @access private
	 * @return none
	 */
	private function put() {
		$id = isset($this->Request['id']) ? $this->Request['id'] : null;

		/**
		 * Check if it is possible to read PUT method data.
		 * Note that some clients sends data in PUT request as PHP input stream which
		 * is not possible to read by $_REQUEST data. From this reason, when is
		 * not possible to ready by superglobal $_REQUEST variable, then is try to
		 * read PUT data by PHP input stream.
		 */
		if (is_array($this->Request['update']) && count($this->Request['update']) > 0) {
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
				$this->set($id, array());
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
		if (is_array($this->Request['create']) && count($this->Request['create']) > 0) {
			$params = (object)$this->Request['create'];
			$this->create($params);
		}
	}

	/**
	 * Deleting element by element ID.
	 *
	 * @access private
	 * @return none
	 */
	private function delete() {
		if (isset($this->Request['id'])) {
			$id = intval($this->Request['id']);
			$this->remove($id);
		}
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
}
?>
