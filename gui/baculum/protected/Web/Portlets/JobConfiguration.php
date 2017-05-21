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

Prado::using('System.Web.UI.ActiveControls.TActiveHiddenField');
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('Application.Web.Portlets.Portlets');

class JobConfiguration extends Portlets {

	const USE_CACHE = true;

	const DEFAULT_JOB_PRIORITY = 10;

	const RESOURCE_SHOW_PATTERN = '/^\s+--> %resource: name=(.+?(?=\s\S+\=.+)|.+$)/i';
	const JOB_SHOW_PATTERN = '/^Job:\sname=(?P<jobname>.+)\sJobType=\d+\slevel=(?P<level>\w+)\sPriority=(?P<priority>\d+)/i';

	public $jobToVerify = array('C', 'O', 'd');

	public $verifyOptions = array('jobname' => 'Verify by Job Name', 'jobid' => 'Verify by JobId');

	public function configure($jobId, $params = array()) {
		$jobdata = $this->Application->getModule('api')->get(array('jobs', $jobId))->output;
		$this->JobName->Text = $jobdata->job;
		$this->JobID->Text = $jobdata->jobid;
		$joblog = $this->Application->getModule('api')->get(array('joblog', $jobdata->jobid))->output;
		$runningJobStates = $this->Application->getModule('misc')->getRunningJobStates();
		if (in_array($jobdata->jobstatus, $runningJobStates)) {
			$this->Estimation->CssClass = 'textbox-auto wheel-loader';
		} else {
			$this->Estimation->CssClass = 'textbox-auto';
		}
		$this->Estimation->Text = is_array($joblog) ? implode(PHP_EOL, $joblog) : Prado::localize("Output for selected job is not available yet or you do not have enabled logging job logs to catalog database. For watching job log there is need to add to the job Messages resource next directive: console = all, !skipped, !saved");

		$this->Level->dataSource = $this->Application->getModule('misc')->getJobLevels();
		$this->Level->SelectedValue = $jobdata->level;
		$this->Level->dataBind();

		$isVerifyOption = in_array($jobdata->level, $this->jobToVerify);
		$this->JobToVerifyOptionsLine->Display = ($isVerifyOption === true) ? 'Dynamic' : 'None';
		$this->JobToVerifyJobNameLine->Display = ($isVerifyOption === true) ? 'Dynamic' : 'None';
		$this->JobToVerifyJobIdLine->Display = 'None';
		$this->AccurateLine->Display = ($isVerifyOption === true) ? 'None' : 'Dynamic';
		$this->EstimateLine->Display = ($isVerifyOption === true) ? 'None' : 'Dynamic';

		$verifyValues = array();

		foreach($this->verifyOptions as $value => $text) {
			$verifyValues[$value] = Prado::localize($text);
		}

		$this->JobToVerifyOptions->dataSource = $verifyValues;
		$this->JobToVerifyOptions->dataBind();

		$jobTasks = $this->Application->getModule('api')->get(array('jobs', 'tasks'), null, true, self::USE_CACHE)->output;
		$jobsAllDirs = array();
		foreach($jobTasks as $director => $tasks) {
			$jobsAllDirs = array_merge($jobsAllDirs, $tasks);
		}

		$this->JobToVerifyJobName->dataSource = array_combine($jobsAllDirs, $jobsAllDirs);
		$this->JobToVerifyJobName->dataBind();

		$clients = $this->Application->getModule('api')->get(array('clients'), null, true, self::USE_CACHE)->output;
		$clientsList = array();
		foreach($clients as $client) {
			$clientsList[$client->clientid] = $client->name;
		}
		$this->Client->dataSource = $clientsList;
		$this->Client->SelectedValue = $jobdata->clientid;
		$this->Client->dataBind();

		$filesetsAll = $this->Application->getModule('api')->get(array('filesets'), null, true, self::USE_CACHE)->output;
		$filesetsList = array();
		foreach($filesetsAll as $director => $filesets) {
			$filesetsList = array_merge($filesets, $filesetsList);
		}
		$selectedFileset = '';
		if($jobdata->filesetid != 0) {
			$selectedFileset = $this->Application->getModule('api')->get(array('filesets', $jobdata->filesetid), null, true, self::USE_CACHE)->output->fileset;
		}
		$this->FileSet->dataSource = array_combine($filesetsList, $filesetsList);
		$this->FileSet->SelectedValue = array_key_exists('fileset', $params) ? $params['fileset'] : $selectedFileset;
		$this->FileSet->dataBind();

		$pools = $this->Application->getModule('api')->get(array('pools'), null, true, self::USE_CACHE)->output;
		$poolList = array();
		foreach($pools as $pool) {
			$poolList[$pool->poolid] = $pool->name;
		}
		$this->Pool->dataSource = $poolList;
		$this->Pool->SelectedValue = array_key_exists('poolid', $params) ? $params['poolid'] : $jobdata->poolid;
		$this->Pool->dataBind();

		$jobshow = $this->Application->getModule('api')->get(array('jobs', 'show', $jobdata->jobid), null, true, self::USE_CACHE)->output;
		$storageShow = $this->getResourceName('storage', $jobshow);
		$storagesList = array();
		$selectedStorageId = null;
		$storages = $this->Application->getModule('api')->get(array('storages'), null, true, self::USE_CACHE)->output;
		foreach($storages as $storage) {
			if ($storage->name == $storageShow) {
				$selectedStorageId = $storage->storageid;
			}
			$storagesList[$storage->storageid] = $storage->name;
		}
		$this->Storage->dataSource = $storagesList;
		if (!is_null($selectedStorageId)) {
			$this->Storage->SelectedValue = $selectedStorageId;
		}
		$this->Storage->dataBind();

		$runningJobStates = $this->Application->getModule('misc')->getRunningJobStates();
		$isJobRunning = in_array($jobdata->jobstatus, $runningJobStates);

		$this->Priority->Text = ($jobdata->priorjobid == 0) ? self::DEFAULT_JOB_PRIORITY : $jobdata->priorjobid;
		$this->DeleteButton->Visible = true;
		$this->CancelButton->Visible = $isJobRunning;
		$this->RefreshStart->Value = $isJobRunning;
		$this->Run->Display = 'Dynamic';
		$this->EstimateLine->Display = 'Dynamic';
		$this->Status->Visible = true;
	}

	public function status($sender, $param) {
		$jobdata = $this->Application->getModule('api')->get(array('jobs', $this->JobID->Text))->output;
		$runningJobStates = $this->Application->getModule('misc')->getRunningJobStates();
		if (in_array($jobdata->jobstatus, $runningJobStates)) {
			$this->RefreshStart->Value = true;
		} else {
			$this->RefreshStart->Value = false;
			$this->CancelButton->Visible = false;
			$this->Estimation->CssClass = 'textbox-auto';
		}

		$joblog = $this->Application->getModule('api')->get(array('joblog', $this->JobID->Text))->output;
		$this->Estimation->Text = is_array($joblog) ? implode(PHP_EOL, $joblog) : Prado::localize("Output for selected job is not available yet or you do not have enabled logging job logs to catalog database. For watching job log there is need to add to the job Messages resource next directive: console = all, !skipped, !saved");
	}

	public function delete($sender, $param) {
		$this->Application->getModule('api')->remove(array('jobs', $this->JobID->Text));
		$this->Status->Visible = false;
		$this->Run->Display = 'None';
		$this->DeleteButton->Visible = false;
		$this->EstimateLine->Display = 'None';
		$this->Estimation->CssClass = 'textbox-auto';
	}

	public function cancel($sender, $param) {
		$this->Application->getModule('api')->set(array('jobs', 'cancel', $this->JobID->Text), array('a' => 'b'));
		$this->CancelButton->Visible = false;
		$this->status(null, null);
		$this->Estimation->CssClass = 'textbox-auto';
	}

	public function run_again($sender, $param) {
		if($this->PriorityValidator->IsValid === false) {
			return false;
		}

		$params = array();
		if (is_null($sender) && is_numeric($param)) {
			$jobdata = $this->Application->getModule('api')->get(array('jobs', $param))->output;
			$jobshow = $this->Application->getModule('api')->get(array('jobs', 'show', $jobdata->jobid))->output;
			$params['id'] = $jobdata->jobid;
			$params['level'] = $jobdata->level;
			$params['fileset'] = $this->getResourceName('fileset', $jobshow);
			$params['clientid'] = $jobdata->clientid;
			$storage = $this->getResourceName('storage', $jobshow);
			if (!is_null($storage)) {
				$params['storageid'] = $this->getStorageByName($storage)->storageid;
			} else {
				$autochanger = $this->getResourceName('autochanger', $jobshow);
				if (!is_null($autochanger)) {
					$params['storageid'] = $this->getStorageByName($autochanger)->storageid;
				}
			}
			$pool = $this->getResourceName('pool', $jobshow);
			if (!is_null($pool)) {
				$params['poolid'] = $this->getPoolByName($pool)->poolid;
			}
		} else {
			$params['id'] = $this->JobID->Text;
			$params['level'] = $this->Level->SelectedValue;
			$params['fileset'] = $this->FileSet->SelectedValue;
			$params['clientid'] = $this->Client->SelectedValue;
			$params['storageid'] = $this->Storage->SelectedValue;
			$params['poolid'] = $this->Pool->SelectedValue;
			$params['priority'] = $this->Priority->Text;

			if (in_array($this->Level->SelectedItem->Value, $this->jobToVerify)) {
				$verifyVals = $this->getVerifyVals();
				if ($this->JobToVerifyOptions->SelectedItem->Value == $verifyVals['jobname']) {
					$params['verifyjob'] = $this->JobToVerifyJobName->SelectedValue;
				} elseif ($this->JobToVerifyOptions->SelectedItem->Value == $verifyVals['jobid']) {
					$params['jobid'] = $this->JobToVerifyJobId->Text;
				}
			}
		}
		$result = $this->Application->getModule('api')->create(array('jobs', 'run'), $params)->output;
		$startedJobId = $this->Application->getModule('misc')->findJobIdStartedJob($result);
		if (is_numeric($startedJobId)) {
			$params['jobid'] = $startedJobId;
			$this->configure($startedJobId, $params);
		} else {
			$this->Estimation->Text = implode(PHP_EOL, $result);
		}
	}

	public function estimate($sender, $param) {
		$params = array();
		$params['id'] = $this->JobID->Text;
		$params['level'] = $this->Level->SelectedValue;
		$params['fileset'] = $this->FileSet->SelectedValue;
		$params['clientid'] = $this->Client->SelectedValue;
		$params['accurate'] = (integer)$this->Accurate->Checked;
		$result = $this->Application->getModule('api')->create(array('jobs', 'estimate'), $params)->output;
		$this->Estimation->Text = implode(PHP_EOL, $result);
	}

	public function priorityValidator($sender, $param) {
		$isValid = preg_match('/^[0-9]+$/',$this->Priority->Text) === 1 && $this->Priority->Text > 0;
		$param->setIsValid($isValid);
	}

	public function jobIdToVerifyValidator($sender, $param) {
		$verifyVals = $this->getVerifyVals();
		if(in_array($this->Level->SelectedValue, $this->jobToVerify) && $this->JobToVerifyOptions->SelectedItem->Value == $verifyVals['jobid']) {
			$isValid = preg_match('/^[0-9]+$/',$this->JobToVerifyJobId->Text) === 1 && $this->JobToVerifyJobId->Text > 0;
		} else {
			$isValid = true;
		}
		$param->setIsValid($isValid);
		return $isValid;
	}

	private function getVerifyVals() {
		$verifyOpt = array_keys($this->verifyOptions);
		$verifyVals = array_combine($verifyOpt, $verifyOpt);
		return $verifyVals;
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

	public function getJobAttr($jobshow) {
		$attr = array();
		for ($i = 0; $i < count($jobshow); $i++) {
			if (preg_match(self::JOB_SHOW_PATTERN, $jobshow[$i], $match) === 1) {
				$attr['jobname'] = $match['jobname'];
				$attr['level'] = $match['level'];
				$attr['priority'] = $match['priority'];
				break;
			}
		}
		return $attr;
	}

	private function getPoolByName($name) {
		$pool = null;
		$pools = $this->Application->getModule('api')->get(array('pools'), null, true, self::USE_CACHE)->output;
		for ($i = 0; $i < count($pools); $i++) {
			if ($pools[$i]->name == $name) {
				$pool = $pools[$i];
				break;
			}
		}
		return $pool;
	}

	private function getStorageByName($name) {
		$storage = null;
		$storages = $this->Application->getModule('api')->get(array('storages'), null, true, self::USE_CACHE)->output;
		for ($i = 0; $i < count($storages); $i++) {
			if ($storages[$i]->name == $name) {
				$storage = $storages[$i];
				break;
			}
		}
		return $storage;
	}
}
?>
