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

class Monitor extends BaculumWebPage {

	public function onInit($param) {
		parent::onInit($param);
		$_SESSION['monitor_data'] = array(
			'jobs' => array(),
			'running_jobs' => array(),
			'terminated_jobs' => array(),
			'pools' => array(),
			'jobtotals' => array(),
			'dbsize' => array()
		);

		$params = $this->Request->contains('params') ? $this->Request['params'] : array();
		if (in_array('jobs', $params)) {
			$_SESSION['monitor_data']['jobs'] = $this->getModule('api')->get(array('jobs'))->output;
		}
		$_SESSION['monitor_data']['running_jobs'] = $this->getModule('api')->get(array('jobs', '?jobstatus=CR'))->output;
		if (in_array('clients', $params)) {
			$_SESSION['monitor_data']['clients'] = $this->getModule('api')->get(array('clients'))->output;
		}
		if (in_array('pools', $params)) {
			$_SESSION['monitor_data']['pools'] = $this->getModule('api')->get(array('pools'))->output;
		}
		if (in_array('job_totals', $params)) {
			$_SESSION['monitor_data']['jobtotals'] = $this->getModule('api')->get(array('jobs', 'totals'))->output;
		}
		if ($_SESSION['admin'] && in_array('dbsize', $params)) {
			$_SESSION['monitor_data']['dbsize'] = $this->getModule('api')->get(array('dbsize'))->output;
		}

		$runningJobStates = $this->Application->getModule('misc')->getRunningJobStates();

		if (in_array('jobs', $params)) {
			for ($i = 0; $i < count($_SESSION['monitor_data']['jobs']); $i++) {
				if (!in_array($_SESSION['monitor_data']['jobs'][$i]->jobstatus, $runningJobStates)) {
					$_SESSION['monitor_data']['terminated_jobs'][] = $_SESSION['monitor_data']['jobs'][$i];
				}
			}
		}
		echo json_encode($_SESSION['monitor_data']);
		exit();
	}
}

?>
