<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2020 Kern Sibbald
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

Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('Application.Web.Class.BaculumWebPage'); 
Prado::using('Application.Web.Portlets.RunJob');

/**
 * Job history list page.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class JobHistoryList extends BaculumWebPage {

	const USE_CACHE = true;

	public function loadRunJobModal($sender, $param) {
		$this->RunJobModal->loadData();
	}

	public function runJobAgain($sender, $param) {
		$jobid = intval($param->getCallbackParameter());
		if ($jobid > 0) {
			$jobdata = $this->getModule('api')->get(
				array('jobs', $jobid),
				null,
				true,
				self::USE_CACHE
			)->output;
			$params = array();
			$params['id'] = $jobid;
			$level = trim($jobdata->level);
			$params['level'] = !empty($level) ? $level : 'F'; // Admin job has empty level
			$job_show = $this->getModule('api')->get(
				array('jobs', $jobid, 'show'), null, true, self::USE_CACHE
			)->output;
			if ($jobdata->filesetid > 0) {
				$params['filesetid'] = $jobdata->filesetid;
			} else {
				$params['fileset'] = RunJob::getResourceName('fileset', $job_show);
			}
			$params['clientid'] = $jobdata->clientid;
			$params['storage'] = RunJob::getResourceName('storage', $job_show);
			if (empty($params['storage'])) {
				$params['storage'] = RunJob::getResourceName('autochanger', $job_show);
			}
			if ($jobdata->poolid > 0) {
				$params['poolid'] = $jobdata->poolid;
			} else {
				$params['pool'] = RunJob::getResourceName('pool', $job_show);
			}
			$job_attr = RunJob::getJobAttr($job_show);
			$params['priority'] = key_exists('priority', $job_attr) ? $job_attr['priority'] : 10;
			$params['accurate'] = (key_exists('accurate', $job_attr) && $job_attr['accurate'] == 1);

			$result = $this->getModule('api')->create(array('jobs', 'run'), $params);
			if ($result->error === 0) {
				$started_jobid = $this->getModule('misc')->findJobIdStartedJob($result->output);
				if (!is_numeric($started_jobid)) {
					$errmsg = implode('<br />', $result->output);
					$this->getPage()->getCallbackClient()->callClientFunction('show_error', array($errmsg, $result->error));
				}
			} else {
				$this->getPage()->getCallbackClient()->callClientFunction('show_error', array($result->output, $result->error));
			}
		}
	}

	public function cancelJob($sender, $param) {
		$jobid = intval($param->getCallbackParameter());
		$result = $this->getModule('api')->set(
			array('jobs', $jobid, 'cancel'),
			array()
		);
	}

	/**
	 * Cancel multiple jobs.
	 * Used for bulk actions.
	 *
	 * @param TCallback $sender callback object
	 * @param TCallbackEventPrameter $param event parameter
	 * @return none
	 */
	public function cancelJobs($sender, $param) {
		$result = [];
		$jobids = explode('|', $param->getCallbackParameter());
		for ($i = 0; $i < count($jobids); $i++) {
			$ret = $this->getModule('api')->set(
				['jobs', intval($jobids[$i]), 'cancel']
			);
			if ($ret->error !== 0) {
				$result[] = $ret->output;
				break;
			}
			$result[] = implode(PHP_EOL, $ret->output);
		}
		$this->getCallbackClient()->update($this->BulkActions->BulkActionsOutput, implode(PHP_EOL, $result));
	}

	/**
	 * Delete multiple jobs.
	 * Used for bulk actions.
	 *
	 * @param TCallback $sender callback object
	 * @param TCallbackEventPrameter $param event parameter
	 * @return none
	 */
	public function deleteJobs($sender, $param) {
		$result = [];
		$jobids = explode('|', $param->getCallbackParameter());
		for ($i = 0; $i < count($jobids); $i++) {
			$ret = $this->getModule('api')->remove(
				['jobs', intval($jobids[$i])]
			);
			if ($ret->error !== 0) {
				$result[] = $ret->output;
				break;
			}
			$result[] = implode(PHP_EOL, $ret->output);
		}
		$this->getCallbackClient()->update($this->BulkActions->BulkActionsOutput, implode(PHP_EOL, $result));
	}
}
