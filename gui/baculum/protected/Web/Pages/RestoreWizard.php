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

Prado::using('Application.Web.Class.BaculumWebPage'); 
Prado::using('System.Exceptions.TException');
Prado::using('System.Web.UI.WebControls.TWizard');
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TActiveImageButton');
Prado::using('System.Web.UI.ActiveControls.TDropContainer');
Prado::using('System.Web.UI.ActiveControls.TDraggable');
Prado::using('System.Web.UI.ActiveControls.TActiveRadioButton');
Prado::using('System.Web.UI.ActiveControls.TActiveDataGrid');
Prado::using('System.Web.UI.ActiveControls.TCallback');

class RestoreWizard extends BaculumWebPage
{
	private $job_levels_to_restore = array('F' => 'Full', 'I' => 'Incremental', 'D'=> 'Differential');
	private $jobs = null;
	private $filesets = null;
	private $storages = null;
	private $clients = null;
	private $browser_root_dir = array('name' => '.', 'type' => 'dir');
	private $browser_up_dir = array('name' => '..', 'type' => 'dir');

	public $excluded_elements_from_add = array('.', '..');

	const BVFS_PATH_PREFIX = 'b2';

	public function onPreInit($param) {
		parent::onPreInit($param);
		$this->Application->getModule('web_users')->loginUser();
	}

	public function onInit($param) {
		parent::onInit($param);
		if(!$this->IsPostBack && !$this->IsCallBack) {
			$this->setBrowserFiles(array());
			$this->setFileVersions(array());
			$this->setFilesToRestore(array());
			$this->markFileToRestore(null, null);
			$this->setRestoreJobs();
			$_SESSION['restore_path'] = array();
		}
	}

	public function onLoad($param) {
		parent::onLoad($param);
			if($this->RestoreWizard->ActiveStepIndex == 0) {
				$this->jobs = $this->getModule('api')->get(array('jobs'))->output;
				$this->filesets = $this->getModule('api')->get(array('filesets', 'info'))->output;
			}
			$this->clients = $this->getModule('api')->get(array('clients'))->output;

			if(!$this->IsCallBack && (($this->RestoreWizard->ActiveStepIndex == 1 && $this->getPage()->BackupToRestore->ItemCount > 0) || $this->RestoreWizard->ActiveStepIndex == 3)) {
				$this->setFileVersions(array());
				$this->setBrowserPath('');
				$this->prepareFilesToRestore();
				$this->prepareVersionsToRestore();
			}
	}

	public function addFileToRestore($sender, $param) {
		$fileid = null;
		if (isset($param->callbackParameter)) {
			list(, , , $sourceElementID, , ) = explode('_', $sender->ClientID, 6);
			$fileid = $param->callbackParameter;
		} else {
			$control = $param->getDroppedControl();
		        $item = $control->getNamingContainer();
			list(, , , $sourceElementID, , ) = explode('_', $param->getDragElementID(), 6); // I know that it is ugly.
		}
		if($sourceElementID == $this->VersionsDataGrid->ID) {
			if (is_null($fileid)) {
				$fileid = $this->VersionsDataGrid->getDataKeys()->itemAt($item->getItemIndex());
			}
			$fileProperties = $this->getFileVersions($fileid);
		} else {
			if (is_null($fileid)) {
				$fileid = $this->DataGridFiles->getDataKeys()->itemAt($item->getItemIndex());
			}
			$fileProperties = $this->getBrowserFile($fileid);
		}
		if($fileProperties['name'] != $this->browser_root_dir['name'] && $fileProperties['name'] != $this->browser_up_dir['name']) {
			$this->markFileToRestore($fileid, $fileProperties);
			$this->prepareFilesToRestore();
		}
	}

	public function removeSelectedFile($sender, $param) {
		$fileid = $param->CallbackParameter;
		$this->unmarkFileToRestore($fileid);
		$this->prepareFilesToRestore();
	}

	public function getVersions($sender, $param) {
		list($filename, $pathid, $filenameid, $jobid) = explode('|', $param->CallbackParameter, 4);
		if($filenameid == 0) {
			$this->setBrowserPath($filename);
			return;
		}
		$clientname = $this->BackupClientName->SelectedValue;
		$versions = $this->getModule('api')->get(array('bvfs', 'versions', $clientname, $jobid, $pathid, $filenameid))->output;
		$fileVersions = $this->getModule('misc')->parseFileVersions($filename, $versions);
		$this->setFileVersions($fileVersions);
		$this->VersionsDataGrid->dataSource = $fileVersions;
		$this->VersionsDataGrid->dataBind();
		$this->prepareFilesToRestore();
	}

	public function refreshSelectedFiles($sender, $param) {
		$this->prepareFilesToRestore();
		$this->SelectedVersionsDropper->render($param->NewWriter);
	}

	public function NextStep($sender, $param) {
	}
	
	public function PreviousStep($sender, $param) {
	}

	public function wizardStop($sender, $param) {
		$this->goToDefaultPage();
	}

	public function setJobs($sender, $param) {
		$jobs_list = $jobs_group_list = array();
		$clientid = null;
		for ($i = 0; $i < count($this->clients); $i++) {
			if ($this->clients[$i]->name === $this->BackupClientName->SelectedValue) {
				$clientid = $this->clients[$i]->clientid;
				break;
			}
		}
		if(is_array($this->jobs)) {
			foreach($this->jobs as $job) {
				if(array_key_exists($job->level, $this->job_levels_to_restore) && $job->type == 'B' && $job->jobstatus == 'T' && $job->clientid == $clientid) {
					$jobs_list[$job->jobid] = sprintf('[%s] %s, %s, %s', $job->jobid, $job->name, $this->job_levels_to_restore[$job->level], $job->endtime);
					$jobs_group_list[$job->name] = $job->name;
				}
			}
		}

		$fileset_list = array();
		for ($i = 0; $i < count($this->filesets); $i++) {
			$fileset_list[$this->filesets[$i]->filesetid] = $this->filesets[$i]->fileset . ' (' . $this->filesets[$i]->createtime . ')';
		}
		asort($fileset_list);

		$this->BackupToRestore->dataSource = $jobs_list;
		$this->BackupToRestore->dataBind();
		$this->GroupBackupToRestore->dataSource = $jobs_group_list;
		$this->GroupBackupToRestore->dataBind();
		$this->GroupBackupFileSet->dataSource = $fileset_list;
		$this->GroupBackupFileSet->dataBind();
		$this->setRestoreClients($sender, $param);
	}

	public function setBackupClients($sender, $param) {
		if(!$this->IsPostBack) {
			$client_list = array();
			foreach($this->clients as $client) {
				$client_list[$client->name] = $client->name;
			}
			$this->BackupClientName->dataSource = $client_list;
			$this->BackupClientName->dataBind();
		}
	}

	public function setRestoreClients($sender, $param) {
		if(!$this->IsPostBack || $sender->ID == $this->BackupClientName->ID) {
			$client_list = array();
			foreach($this->clients as $client) {
				$client_list[$client->name] = $client->name;
			}
			$this->RestoreClient->SelectedValue = $this->BackupClientName->SelectedValue;
			$this->RestoreClient->dataSource = $client_list;
			$this->RestoreClient->dataBind();
		}
	}

	public function setBackupClient($sender, $param) {
		$this->ClientToRestore->SelectedValue = $param->SelectedValue;
	} 

	public function setBackupSelection($sender, $param) {
		$this->GroupBackupToRestoreField->Display = ($sender->ID == $this->GroupBackupSelection->ID) ? 'Dynamic' : 'None';
		$this->BackupToRestoreField->Display = ($sender->ID == $this->OnlySelectedBackupSelection->ID) ? 'Dynamic' : 'None';
		$this->setBrowserFiles(array());
		$this->setFileVersions(array());
		$this->setFilesToRestore(array());
		$this->markFileToRestore(null, null);
		$_SESSION['restore_path'] = array();
	}

	public function resetFileBrowser($sender, $param) {
		$this->markFileToRestore(null, null);
		$this->setBrowserPath($this->browser_root_dir['name']);
	}

	private function prepareBrowserFiles($files) {
		$this->setBrowserFiles($files);
		$this->DataGridFiles->dataSource = $files;
		@$this->DataGridFiles->dataBind();
	}

	private function prepareVersionsToRestore() {
		$this->VersionsDataGrid->dataSource = $_SESSION['files_versions'];
		$this->VersionsDataGrid->dataBind();
	}

	private function prepareFilesToRestore() {
		$this->SelectedVersionsDataGrid->dataSource = $_SESSION['restore'];
		$this->SelectedVersionsDataGrid->dataBind();
	}

	private function setBrowserPath($path) {
		if(!empty($path)) {
			if($path == $this->browser_up_dir['name']) {
				array_pop($_SESSION['restore_path']);
			} elseif($path == $this->browser_root_dir['name']) {
				$_SESSION['restore_path'] = array();
			} else {
				array_push($_SESSION['restore_path'], $path);
			}
		}
		$this->setBrowserLocalizator();
		$this->prepareBrowserContent();
	}

	private function getBrowserPath($stringFormat = false) {
		return ($stringFormat === true) ? implode($_SESSION['restore_path']) : $_SESSION['restore_path'];
	}

	private function setBrowserLocalizator() {
		$localization = $this->getBrowserPath(true);
		$this->PathField->HeaderText = mb_strlen($localization) > 56 ? '...' . mb_substr($localization, -56) : $localization;
	}

	private function prepareBrowserContent() {
		$jobids = $this->getElementaryBackup();
		$elements = array();
		if (!is_null($jobids)) {
			// generating Bvfs may takes a moment
			$this->generateBvfsCacheByJobids($jobids);
			$bvfs_dirs_list = $this->getModule('api')->set(array('bvfs', 'lsdirs'), array('jobids' => $jobids, 'path' => $this->getBrowserPath(true)));
			$bvfs_dirs = is_object($bvfs_dirs_list) ? $bvfs_dirs_list->output : array();
			$dirs = $this->getModule('misc')->parseBvfsList($bvfs_dirs);
			$bvfs_files_list = $this->getModule('api')->set(array('bvfs', 'lsfiles'), array('jobids' => $jobids, 'path' => $this->getBrowserPath(true)));
			$bvfs_files = is_object($bvfs_files_list) ? $bvfs_files_list->output : array();
			$files = $this->getModule('misc')->parseBvfsList($bvfs_files);
			$elements = array_merge($dirs, $files);
			if(count($this->getBrowserPath()) > 0) {
				array_unshift($elements, $this->browser_root_dir);
			}
		}
		$this->prepareBrowserFiles($elements);
	}

	private function getElementaryBackup() {
		$jobids = null;
		if($this->OnlySelectedBackupSelection->Checked === true) {
			$jobs = $this->getModule('api')->get(array('bvfs', 'getjobids', $this->BackupToRestore->SelectedValue));
			$ids = is_object($jobs) ? $jobs->output : array();
			foreach($ids as $jobid) {
				if(preg_match('/^([\d\,]+)$/', $jobid, $match) == 1) {
					$jobids = $match[1];
					break;
				}
			}
		} else {
			$params = array(
				'jobs',
				'recent',
				$this->GroupBackupToRestore->SelectedValue,
				'client',
				$this->BackupClientName->SelectedValue,
				'filesetid',
				$this->GroupBackupFileSet->SelectedValue
			);
			$jobs_recent = $this->getModule('api')->get($params);
			if(count($jobs_recent->output) > 0) {
				$ids = $jobs_recent->output;
				$jobids = implode(',', $ids);
			}
		}
		return $jobids;
	}

	private function generateBvfsCacheByJobids($jobids) {
		$this->getModule('api')->set(array('bvfs', 'update'), array('jobids' => $jobids));
	}

	private function setFileVersions($versions) {
		$_SESSION['files_versions'] = $versions;
	}

	private function getFileVersions($fileid) {
		$versions = array();
		foreach($_SESSION['files_versions'] as $file) {
			if(array_key_exists('fileid', $file) && $file['fileid'] == $fileid) {
				$versions = $file;
				break;
			}
		}
		return $versions;
	}

	private function setBrowserFiles($files) {
		$_SESSION['files_browser'] = $files;
	}

	private function getBrowserFile($fileid) {
		$element = array();
		foreach($_SESSION['files_browser'] as $file) {
			if(array_key_exists('fileid', $file) && $file['fileid'] == $fileid) {
				$element = $file;
				break;
			}
		}
		return $element;
	}

	private function markFileToRestore($fileid, $file) {
		if($fileid === null) {
			$_SESSION['restore'] = array();
		} elseif($file['name'] != $this->browser_root_dir['name'] && $file['name'] != $this->browser_up_dir['name']) {
			$_SESSION['restore'][$fileid] = $file;
		}
	}

	private function unmarkFileToRestore($fileid) {
		if(array_key_exists($fileid, $_SESSION['restore'])) {
			unset($_SESSION['restore'][$fileid]);
		}
	}

	public function getFilesToRestore() {
		return $_SESSION['restore'];
	}

	public function setFilesToRestore($files) {
		$_SESSION['restore'] = $files;
	}

	public function getRestoreElements($asObject = false) {
		$fileids = array();
		$dirids = array();
		foreach($this->getFilesToRestore() as $fileid => $properties) {
			if($properties['type'] == 'dir') {
				$dirids[] = $properties['pathid'];
			} elseif($properties['type'] == 'file') {
				$fileids[] = $fileid;
			}
		}
		$ret = array('fileid' => $fileids, 'dirid' => $dirids);
		if($asObject === true) {
			$ret = (object)$ret;
		}
		return $ret;
	}

	public function wizardCompleted() {
		$jobids = $this->getElementaryBackup();
		$path = self::BVFS_PATH_PREFIX . getmypid();
		$restore_elements = $this->getRestoreElements();
		$cmd_props = array('jobids' => $jobids, 'path' => $path);
		if(count($restore_elements['fileid']) > 0) {
			$cmd_props['fileid'] = implode(',', $restore_elements['fileid']);
		}
		if(count($restore_elements['dirid']) > 0) {
			$cmd_props['dirid'] = implode(',', $restore_elements['dirid']);
		}

		$this->getModule('api')->create(array('bvfs', 'restore'), $cmd_props);
		$restore_props = array();
		$restore_props['rpath'] = $path;
		$restore_props['client'] = $this->RestoreClient->SelectedValue;
		$restore_props['priority'] = intval($this->RestoreJobPriority->Text);
		$restore_props['where'] =  $this->RestorePath->Text;
		$restore_props['replace'] = $this->ReplaceFiles->SelectedValue;
		$restore_props['restorejob'] = $this->RestoreJob->SelectedValue;
		
		$ret = $this->getModule('api')->create(array('jobs', 'restore'), $restore_props);
		$jobid = $this->getModule('misc')->findJobIdStartedJob($ret->output);
		$url_params = array('open' => 'Job');
		if (is_numeric($jobid)) {
			$url_params['id'] = $jobid;
		}
		$this->goToDefaultPage($url_params);
	}

	private function setRestoreJobs() {
		$restore_job_tasks = $this->Application->getModule('api')->get(array('jobs', 'tasks', 'type', 'R'))->output;
		$jobs = array();
		foreach ($restore_job_tasks as $director => $restore_jobs) {
			$jobs = array_merge($jobs, $restore_jobs);
		}
		$this->RestoreJob->DataSource = array_combine($jobs, $jobs);
		$this->RestoreJob->dataBind();
	}
}
?>
