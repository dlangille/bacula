<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2017 Kern Sibbald
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

Prado::using('System.Web.UI.WebControls.TCallback');
Prado::using('System.Web.UI.WebControls.TLabel');
Prado::using('System.Web.UI.WebControls.TPanel');
Prado::using('System.Web.UI.WebControls.TValidationSummary');
Prado::using('System.Web.UI.ActiveControls.TActiveCheckBox');
Prado::using('System.Web.UI.ActiveControls.TActiveCustomValidator');
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('Application.Web.Portlets.Portlets');

class JobRunConfiguration extends Portlets {

	const USE_CACHE = true;

	const DEFAULT_JOB_PRIORITY = 10;

	public $job_to_verify = array('C', 'O', 'd');

	public $verify_options = array('jobname' => 'Verify by Job Name', 'jobid' => 'Verify by JobId');

	public function configure($jobname) {
		$job_show = $this->getModule('api')->get(
			array('jobs', 'show', 'name', $jobname),
			null,
			true,
			self::USE_CACHE
		)->output;

		$this->JobName->Text = $jobname;
		$this->Estimation->Text = '';

		$levels = $this->getModule('misc')->getJobLevels();
		$job_attr = $this->getPage()->JobConfiguration->getJobAttr($job_show);
		$levels_flip = array_flip($levels);

		$is_verify = false;
		if (array_key_exists('level', $job_attr) && array_key_exists($job_attr['level'], $levels_flip)) {
			$this->Level->SelectedValue = $levels_flip[$job_attr['level']];
			$is_verify = in_array($levels_flip[$job_attr['level']], $this->job_to_verify);
		}
		$this->JobToVerifyOptionsLine->Display = ($is_verify === true) ? 'Dynamic' : 'None';
		$this->JobToVerifyJobNameLine->Display = ($is_verify === true) ? 'Dynamic' : 'None';
		$this->JobToVerifyJobIdLine->Display = 'None';

		$this->Level->dataSource = $levels;
		$this->Level->dataBind();

		$this->AccurateLine->Display = 'Dynamic';
		$this->EstimateLine->Display = 'Dynamic';

		$verify_values = array();

		foreach($this->verify_options as $value => $text) {
			$verify_values[$value] = Prado::localize($text);
		}

		$this->JobToVerifyOptions->dataSource = $verify_values;
		$this->JobToVerifyOptions->dataBind();

		$jobTasks = $this->getModule('api')->get(
			array('jobs', 'tasks'),
			null,
			true,
			self::USE_CACHE
		)->output;

		$jobs_all_dirs = array();
		foreach($jobTasks as $director => $tasks) {
			$jobs_all_dirs = array_merge($jobs_all_dirs, $tasks);
		}

		$this->JobToVerifyJobName->dataSource = array_combine($jobs_all_dirs, $jobs_all_dirs);
		$this->JobToVerifyJobName->dataBind();

		$clients = $this->getModule('api')->get(
			array('clients'),
			null,
			true,
			self::USE_CACHE
		)->output;
		$selected_client = $this->getPage()->JobConfiguration->getResourceName('client', $job_show);
		$selected_client_id = null;
		$client_list = array();
		foreach($clients as $client) {
			if ($client->name === $selected_client) {
				$selected_client_id = $client->clientid;
			}
			$client_list[$client->clientid] = $client->name;
		}
		$this->Client->dataSource = $client_list;
		$this->Client->SelectedValue = $selected_client_id;
		$this->Client->dataBind();

		$filesets_all = $this->getModule('api')->get(
			array('filesets'),
			null,
			true,
			self::USE_CACHE
		)->output;
		$selected_fileset = $this->getPage()->JobConfiguration->getResourceName('fileset', $job_show);
		$fileset_list = array();
		foreach($filesets_all as $director => $filesets) {
			$fileset_list = array_merge($filesets, $fileset_list);
		}
		$this->FileSet->dataSource = array_combine($fileset_list, $fileset_list);
		$this->FileSet->SelectedValue = $selected_fileset;
		$this->FileSet->dataBind();

		$pools = $this->getModule('api')->get(
			array('pools'),
			null,
			true,
			self::USE_CACHE
		)->output;
		$selected_pool = $this->getPage()->JobConfiguration->getResourceName('pool', $job_show);
		$selected_pool_id = null;
		$pool_list = array();
		foreach($pools as $pool) {
			if ($pool->name === $selected_pool) {
				$selected_pool_id = $pool->poolid;
			}
			$pool_list[$pool->poolid] = $pool->name;
		}
		$this->Pool->dataSource = $pool_list;
		$this->Pool->SelectedValue = $selected_pool_id;
		$this->Pool->dataBind();

		$storages = $this->getModule('api')->get(
			array('storages'),
			null,
			true,
			self::USE_CACHE
		)->output;
		$selected_storage = $this->getPage()->JobConfiguration->getResourceName('storage', $job_show);
		$selected_storage_id = null;
		$storage_list = array();
		foreach($storages as $storage) {
			if ($storage->name === $selected_storage) {
				$selected_storage_id = $storage->storageid;
			}
			$storage_list[$storage->storageid] = $storage->name;
		}
		$this->Storage->dataSource = $storage_list;
		$this->Storage->SelectedValue = $selected_storage_id;
		$this->Storage->dataBind();

		$this->Priority->Text = array_key_exists('priority', $job_attr) ? $job_attr['priority'] : self::DEFAULT_JOB_PRIORITY;
	}

	public function run_job($sender, $param) {
		if($this->PriorityValidator->IsValid === false) {
			return false;
		}
		$params = array();
		$params['name'] = $this->JobName->Text;
		$params['level'] = $this->Level->SelectedValue;
		$params['fileset'] = $this->FileSet->SelectedValue;
		$params['clientid'] = $this->Client->SelectedValue;
		$params['storageid'] = $this->Storage->SelectedValue;
		$params['poolid'] = $this->Pool->SelectedValue;
		$params['priority'] = $this->Priority->Text;

		if (in_array($this->Level->SelectedItem->Value, $this->job_to_verify)) {
			$verify_vals = $this->getVerifyVals();
			if ($this->JobToVerifyOptions->SelectedItem->Value == $verify_vals['jobname']) {
				$params['verifyjob'] = $this->JobToVerifyJobName->SelectedValue;
			} elseif ($this->JobToVerifyOptions->SelectedItem->Value == $verify_vals['jobid']) {
				$params['jobid'] = $this->JobToVerifyJobId->Text;
			}
		}

		$result = $this->getModule('api')->create(array('jobs', 'run'), $params)->output;

		$started_job_id = $this->getModule('misc')->findJobIdStartedJob($result);
		if ($this->GoToStartedJob->Checked === true && is_numeric($started_job_id)) {
			$params['jobid'] = $started_job_id;
			$this->getPage()->JobConfiguration->configure($started_job_id, $params);
		} else {
			$this->Estimation->Text = implode(PHP_EOL, $result);
		}
	}

	public function estimate($sender, $param) {
		$params = array();
		$params['name'] = $this->JobName->Text;
		$params['level'] = $this->Level->SelectedValue;
		$params['fileset'] = $this->FileSet->SelectedValue;
		$params['clientid'] = $this->Client->SelectedValue;
		$params['accurate'] = (integer)$this->Accurate->Checked;
		$result = $this->getModule('api')->create(array('jobs', 'estimate'), $params)->output;
		$this->Estimation->Text = implode(PHP_EOL, $result);
	}

	public function priorityValidator($sender, $param) {
		$is_valid = preg_match('/^[0-9]+$/', $this->Priority->Text) === 1 && $this->Priority->Text > 0;
		$param->setIsValid($is_valid);
	}

	public function jobIdToVerifyValidator($sender, $param) {
		$verify_vals = $this->getVerifyVals();
		if(in_array($this->Level->SelectedValue, $this->job_to_verify) && $this->JobToVerifyOptions->SelectedItem->Value == $verify_vals['jobid']) {
			$is_valid = preg_match('/^[0-9]+$/',$this->JobToVerifyJobId->Text) === 1 && $this->JobToVerifyJobId->Text > 0;
		} else {
			$is_valid = true;
		}
		$param->setIsValid($is_valid);
		return $is_valid;
	}

	private function getVerifyVals() {
		$verify_opt = array_keys($this->verify_options);
		$verify_vals = array_combine($verify_opt, $verify_opt);
		return $verify_vals;
	}
}
?>
