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

Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('System.Web.UI.JuiControls.TJuiProgressbar');
Prado::using('Application.Web.Class.BaculumWebPage'); 

class JobHistoryView extends BaculumWebPage {

	const IS_RUNNING = 'IsRunning';
	const JOBID = 'JobId';
	const JOB_NAME = 'JobName';
	const JOB_UNAME = 'JobUname';
	const JOB_LEVEL = 'JobLevel';
	const JOB_TYPE = 'JobType';
	const CLIENTID = 'ClientId';

	const USE_CACHE = true;

	const SORT_ASC = 0;
	const SORT_DESC = 1;

	const RESOURCE_SHOW_PATTERN = '/^\s+--> %resource: name=(.+?(?=\s\S+\=.+)|.+$)/i';

	public $is_running = false;
	public $allow_graph_mode = false;
	public $allow_list_files_mode = false;
	private $no_graph_mode_types = array('M', 'D', 'C', 'c', 'g');
	private $no_graph_mode_verify_levels = array('O');
	private $list_files_types = array('B', 'C', 'V');
	private $list_files_mode_verify_levels = array('V');

	public function onPreInit($param) {
		parent::onPreInit($param);
		$jobid = 0;
		if ($this->Request->contains('jobid')) {
			$jobid = intval($this->Request['jobid']);
		}
		$jobdata = $this->getModule('api')->get(
			array('jobs', $jobid), null, true, self::USE_CACHE
		);
		$jobdata = $jobdata->error === 0 ? $jobdata->output : null;
		$this->setJobId($jobdata->jobid);
		$this->setJobName($jobdata->name);
		$this->setJobUname($jobdata->job);
		$this->setJobType($jobdata->type);
		$this->setJobLevel($jobdata->level);
		$this->setClientId($jobdata->clientid);
		$this->is_running = $this->getModule('misc')->isJobRunning($jobdata->jobstatus);
		$this->allow_graph_mode = ($this->is_running && !in_array($jobdata->type, $this->no_graph_mode_types));
		$this->allow_list_files_mode = (!$this->is_running && in_array($jobdata->type, $this->list_files_types));
		if ($jobdata->type === 'V') {
			// Verify job requires special treating here
			if (in_array($jobdata->level, $this->no_graph_mode_verify_levels)) {
				$this->allow_graph_mode = false;
			}
			if (!in_array($jobdata->level, $this->list_files_mode_verify_levels)) {
				$this->allow_list_files_mode = false;
			}
		}
	}

	public function onInit($param) {
		parent::onInit($param);
		$this->RunJobModal->setJobId($this->getJobId());
		$this->RunJobModal->setJobName($this->getJobName());
		$this->FileList->setJobId($this->getJobId());
	}

	public function onLoad($param) {
		parent::onLoad($param);
		if ($this->IsCallBack || $this->IsPostBack) {
			return;
		}
		$this->refreshJobLog(null, null);
	}

	public function runningJobStatus($sender, $param) {
		$running_job_status = $this->getRunningJobStatus($this->getClientId());
		$tabs = array(
			'joblog_subtab_text' => true,
			'status_running_job_subtab_graphical' => false,
			'jobfiles_subtab_text' => false
		);
		if ($this->allow_graph_mode) {
			$tabs['status_running_job_subtab_graphical'] = true;
		} elseif ($this->allow_list_files_mode) {
			$tabs['jobfiles_subtab_text'] = true;
		}
		$this->getCallbackClient()->callClientFunction('init_graphical_running_job_status', array($running_job_status, $tabs));
	}

	public function getRunningJobStatus($clientid) {
		$running_job_status = array(
			'header' => array(),
			'job' => array(),
			'is_running' => $this->is_running
		);
		if ($this->is_running) {
			// Don't call client if job isn't running
			$client_name = null;
			$client = $this->getModule('api')->get(
				array('clients', $clientid)
			);
			if ($client->error === 0) {
				$client_name = $client->output->name;
			}

			if (is_string($client_name)) {
				$query_str = '?name=' . rawurlencode($client_name) . '&type=header';
				$graph_status = $this->getModule('api')->get(
					array('status', 'client', $query_str)
				);

				if ($graph_status->error === 0) {
					$running_job_status['header'] = $graph_status->output;
				}

				$query_str = '?name=' . rawurlencode($client_name) . '&type=running';
				$graph_status = $this->getModule('api')->get(
					array('status', 'client', $query_str)
				);
				if ($graph_status->error === 0) {
					$jobid = $this->getJobId();
					for ($i = 0; $i < count($graph_status->output); $i++) {
						foreach ($graph_status->output[$i] as $key => $val) {
							$prop = strtolower($key);
							if ($prop === 'jobid' && intval($val) == $jobid) {
								$running_job_status['job'] = $graph_status->output[$i];
								break 2;
							}
						}
					}
				}
			}
		}
		return $running_job_status;
	}

	/**
	 * Set jobid to run job again.
	 *
	 * @return none
	 */
	public function setJobId($jobid) {
		$jobid = intval($jobid);
		$this->setViewState(self::JOBID, $jobid, 0);
	}

	/**
	 * Get jobid to run job again.
	 *
	 * @return integer jobid
	 */
	public function getJobId() {
		return $this->getViewState(self::JOBID, 0);
	}

	/**
	 * Set job clientid
	 *
	 * @return none
	 */
	public function setClientId($clientid) {
		$clientid = intval($clientid);
		$this->setViewState(self::CLIENTID, $clientid, 0);
	}

	/**
	 * Get client jobid.
	 *
	 * @return integer clientid
	 */
	public function getClientId() {
		return $this->getViewState(self::CLIENTID, 0);
	}

	/**
	 * Set job name to run job again.
	 *
	 * @return none;
	 */
	public function setJobName($job_name) {
		$this->setViewState(self::JOB_NAME, $job_name);
	}

	/**
	 * Get job name to run job again.
	 *
	 * @return string job name
	 */
	public function getJobName() {
		return $this->getViewState(self::JOB_NAME);
	}

	/**
	 * Set job uname.
	 *
	 * @return none;
	 */
	public function setJobUname($job_uname) {
		$this->setViewState(self::JOB_UNAME, $job_uname);
	}

	/**
	 * Get job uname.
	 *
	 * @return string job uname
	 */
	public function getJobUname() {
		return $this->getViewState(self::JOB_UNAME);
	}

	/**
	 * Set job type.
	 *
	 * @return none;
	 */
	public function setJobType($job_type) {
		$this->setViewState(self::JOB_TYPE, $job_type);
	}

	/**
	 * Get job type.
	 *
	 * @return string job type
	 */
	public function getJobType() {
		return $this->getViewState(self::JOB_TYPE);
	}

	/**
	 * Set job level.
	 *
	 * @return none;
	 */
	public function setJobLevel($job_level) {
		$this->setViewState(self::JOB_LEVEL, $job_level);
	}

	/**
	 * Get job level.
	 *
	 * @return string job level
	 */
	public function getJobLevel() {
		return $this->getViewState(self::JOB_LEVEL);
	}

	/**
	 * Refresh job log page and load latest logs.
	 *
	 * @param $sender TActiveLabel sender object
	 * @param $param TCallbackParameter parameter object
	 */
	public function refreshJobLog($sender, $param) {
		$log = $this->getModule('api')->get(array('joblog', $this->getJobId()));
		if (!is_array($log->output) || count($log->output) == 0) {
			$msg = Prado::localize("Output for selected job is not available yet or you do not have enabled logging job logs to the catalog database.\n\nTo watch job log you need to add to the job Messages resource the following directive:\n\nCatalog = all, !debug, !skipped, !saved");
			$joblog = array($msg);

		} else {
			$joblog = $log->output;
		}
		if ($this->is_running) {
			$this->RunningIcon->Display = 'Dynamic';
			$this->FinishedIcon->Display = 'None';
			$this->CancelBtn->Display = 'Dynamic';
			$this->DeleteBtn->Display = 'None';
			$this->RestoreBtn->Display = 'None';
		} else {
			$this->FinishedIcon->Display = 'Dynamic';
			$this->RunningIcon->Display = 'None';
			$this->CancelBtn->Display = 'None';
			$this->DeleteBtn->Display = 'Dynamic';
			$this->RestoreBtn->Display = $this->getJobType() == 'B' ? 'Dynamic' : 'None';
		}
		if ($this->getJobLogOrder() === self::SORT_DESC) {
			$joblog = array_reverse($joblog);
		}
		$joblog = $this->getModule('log_parser')->parse($joblog);

		$this->JobLog->Text = implode(PHP_EOL, $joblog);
	}

	public function loadRunJobModal($sender, $param) {
		$this->RunJobModal->loadData();
	}

	public function loadJobConfig($sender, $param) {
		if (!empty($_SESSION['dir'])) {
			$this->JobConfig->setComponentName($_SESSION['dir']);
			$this->JobConfig->setResourceName($this->getJobName());
			$this->JobConfig->setLoadValues(true);
			$this->JobConfig->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}

	public function getResourceName($resource, $jobshow) {
		$resource_name = null;
		$pattern = str_replace('%resource', $resource, self::RESOURCE_SHOW_PATTERN);
		for ($i = 0; $i < count($jobshow); $i++) {
			if (preg_match($pattern, $jobshow[$i], $match) === 1) {
				$resource_name = $match[1];
				break;
			}
		}
		return $resource_name;
	}

	public function loadFileSetConfig($sender, $param) {
		if (!empty($_SESSION['dir'])) {
			$jobshow = $this->getModule('api')->get(array(
				'jobs', $this->getJobId(), 'show'
			))->output;
			$fileset = $this->getResourceName('fileset', $jobshow);
			$this->FileSetConfig->setComponentName($_SESSION['dir']);
			$this->FileSetConfig->setResourceName($fileset);
			$this->FileSetConfig->setLoadValues(true);
			$this->FileSetConfig->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}

	public function loadScheduleConfig($sender, $param) {
		if (!empty($_SESSION['dir'])) {
			$jobshow = $this->getModule('api')->get(array(
				'jobs', $this->getJobId(), 'show'
			))->output;
			$schedule = $this->getResourceName('schedule', $jobshow);
			$this->ScheduleConfig->setComponentName($_SESSION['dir']);
			$this->ScheduleConfig->setResourceName($schedule);
			$this->ScheduleConfig->setLoadValues(true);
			$this->ScheduleConfig->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}

	public function cancel($sender, $param) {
		$this->getModule('api')->set(array('jobs', $this->getJobId(), 'cancel'), array('a' => 'b'));
		$this->refreshJobLog(null, null);
	}

	public function delete($sender, $param) {
		$this->getModule('api')->remove(array('jobs', $this->getJobId()));
	}

	public function setJobLogOrder($order) {
		$order = TPropertyValue::ensureInteger($order);
		setcookie('log_order', $order, time()+60*60*24*365, '/'); // set cookie for one year
		$_COOKIE['log_order'] = $order;
	}

	public function getJobLogOrder() {
		return (key_exists('log_order', $_COOKIE) ? intval($_COOKIE['log_order']) : self::SORT_DESC);
	}

	public function changeJobLogOrder($sender, $param) {
		$order = $this->getJobLogOrder();
		if ($order === self::SORT_DESC) {
			$this->setJobLogOrder(self::SORT_ASC);
		} else {
			$this->setJobLogOrder(self::SORT_DESC);
		}
		$this->refreshJobLog(null, null);
	}
}
?>
