<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2018 Kern Sibbald
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
Prado::using('Application.Web.Class.BaculumWebPage');

/**
 * Monitor class.
 *
 * NOTE: It must inherit from BaculumPage, not from BaculumWebPage,
 * because this way it has not any redundant API request from BaculumWebPage.
 */
class Monitor extends BaculumPage {

	const DEFAULT_MAX_JOBS = 10000;

	public function onInit($param) {
		parent::onInit($param);
		$monitor_data = array(
			'jobs' => array(),
			'running_jobs' => array(),
			'terminated_jobs' => array(),
			'pools' => array(),
			'jobtotals' => array(),
			'dbsize' => array(),
			'error' => array('error' => 0, 'output' => '')
		);

		// Initialize session cache to have clear session for Monitor
		$this->getModule('api')->initSessionCache(true);

		$web_config = $this->getModule('web_config')->getConfig();
		$job_limit = self::DEFAULT_MAX_JOBS;
		if (count($web_config) > 0 && key_exists('max_jobs', $web_config['baculum'])) {
			$job_limit = $web_config['baculum']['max_jobs'];
		}

		$error = null;
		$params = $this->Request->contains('params') ? $this->Request['params'] : array();
		if (is_array($params) && key_exists('jobs', $params)) {
			$job_params = array('jobs');
			$job_query = array();
			if (is_array($params['jobs'])) {
				if (key_exists('name', $params['jobs']) && is_array($params['jobs']['name'])) {
					for ($i = 0; $i < count($params['jobs']['name']); $i++) {
						$job_query['name'] = $params['jobs']['name'][$i];
					}
				}
				if (key_exists('client', $params['jobs']) && is_array($params['jobs']['client'])) {
					for ($i = 0; $i < count($params['jobs']['client']); $i++) {
						$job_query['client'] = $params['jobs']['client'][$i];
					}
				}
			}
			if ($this->Request->contains('use_limit') && $this->Request['use_limit'] == 1) {
				$job_query['limit'] = $job_limit;
			}
			if (count($job_query) > 0) {
				$job_params[] = '?' . http_build_query($job_query);
			}
			$result = $this->getModule('api')->get($job_params);
			if ($result->error === 0) {
				$monitor_data['jobs'] = $result->output;
			} else {
				$error = $result;
			}
		}
		if (!$error) {
			$result = $this->getModule('api')->get(array('jobs', '?jobstatus=CR'));
			if ($result->error === 0) {
				$monitor_data['running_jobs'] = $result->output;
			} else {
				$error = $result;
			}
		}
		if (!$error && key_exists('clients', $params)) {
			$result = $this->getModule('api')->get(array('clients'));
			if ($result->error === 0) {
				$monitor_data['clients'] = $result->output;
			} else {
				$error = $result;
			}
		}
		if (!$error && key_exists('pools', $params)) {
			$result = $this->getModule('api')->get(array('pools'));
			if ($result->error === 0) {
				$monitor_data['pools'] = $result->output;
			} else {
				$error = $result;
			}
		}
		if (!$error && key_exists('job_totals', $params)) {
			$result = $this->getModule('api')->get(array('jobs', 'totals'));
			if ($result->error === 0) {
				$monitor_data['jobtotals'] = $result->output;
			} else {
				$error = $result;
			}
		}
		if (!$error && $_SESSION['admin'] && key_exists('dbsize', $params)) {
			$result = $this->getModule('api')->get(array('dbsize'));
			if ($result->error === 0) {
				$monitor_data['dbsize'] = $result->output;
			} else {
				$error = $result;
			}
		}

		$running_job_states = $this->Application->getModule('misc')->getRunningJobStates();

		if (key_exists('jobs', $params)) {
			for ($i = 0; $i < count($monitor_data['jobs']); $i++) {
				if (!in_array($monitor_data['jobs'][$i]->jobstatus, $running_job_states)) {
					$monitor_data['terminated_jobs'][] = $monitor_data['jobs'][$i];
				}
			}
		}
		if (is_object($error)) {
			$monitor_data['error'] = $error;
		}

		echo json_encode($monitor_data);
		exit();
	}
}

?>
