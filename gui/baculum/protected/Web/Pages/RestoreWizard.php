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
Prado::using('System.Web.UI.WebControls.TDataGrid');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TActiveImageButton');
Prado::using('System.Web.UI.ActiveControls.TDropContainer');
Prado::using('System.Web.UI.ActiveControls.TDraggable');
Prado::using('System.Web.UI.ActiveControls.TActiveDataGrid');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');

class RestoreWizard extends BaculumWebPage
{
	/**
	 * Job levels allowed to restore.
	 */

	private $joblevel = array('F', 'I', 'D');
	/**
	 * Job statuses allowed to restore.
	 */
	private $jobstatus = array('T', 'W', 'A', 'E', 'e', 'f');

	/**
	 * File browser special directories.
	 */
	private $browser_root_dir = array('name' => '.', 'type' => 'dir', 'fileid' => null);
	private $browser_up_dir = array('name' => '..', 'type' => 'dir');

	/**
	 * Used to provide in template selected by user single jobid to restore.
	 */
	public $restore_single_jobid;

	/**
	 * Stores file relocation option. Used in template.
	 */
	public $file_relocation_opt;

	/**
	 * FIle browser elements for which 'Add' button is unavailable.
	 */
	public $excluded_elements_from_add = array('.', '..');

	/**
	 * Prefix for Bvfs path.
	 */
	const BVFS_PATH_PREFIX = 'b2';

	public function onInit($param) {
		parent::onInit($param);
		if(!$this->IsPostBack && !$this->IsCallBack) {
			$this->resetWizard();
			$this->loadBackupClients();
		}
	}

	/**
	 * Wizard next button callback actions.
	 *
	 * @param TWizard $sender sender object
	 * @param TWizardNavigationEventParameter $param sender parameters
	 * @return none
	 */
	public function wizardNext($sender, $param) {
		if ($param->CurrentStepIndex === 0) {
			$this->loadBackupsForClient();
			$this->loadGroupBackupToRestore();
			$this->loadGroupFileSetToRestore();
			$this->loadRestoreClients();
			if (isset($_SESSION['restore_single_jobid'])) {
				$this->restore_single_jobid = $_SESSION['restore_single_jobid'];
			}
		} elseif ($param->CurrentStepIndex === 1) {
			if ($this->Request->contains('backup_to_restore')) {
				$_SESSION['restore_single_jobid'] = $this->Request['backup_to_restore'];
			}
			$this->setFileVersions();
			$this->loadSelectedFiles();
			$this->loadFileVersions();
			$this->goToPath();
		} elseif ($param->CurrentStepIndex === 2) {
			$this->loadRequiredVolumes();
		} elseif ($param->CurrentStepIndex === 3) {
			if (isset($_SESSION['file_relocation'])) {
				$this->file_relocation_opt = $_SESSION['file_relocation'];
			}
		} elseif ($param->CurrentStepIndex === 4) {
			if ($this->Request->contains('file_relocation')) {
				$_SESSION['file_relocation'] = $this->Request['file_relocation'];
			}
			$this->file_relocation_opt = $_SESSION['file_relocation'];
		}
	}

	/**
	 * Wizard prev button callback actions.
	 *
	 * @param TWizard $sender sender object
	 * @param TWizardNavigationEventParameter $param sender parameters
	 * @return none
	 */
	public function wizardPrev($sender, $param) {
		if ($param->CurrentStepIndex === 2) {
			$this->restore_single_jobid = $_SESSION['restore_single_jobid'];
			$this->loadBackupsForClient();
		} elseif ($param->CurrentStepIndex === 3) {
			$this->setFileVersions();
			$this->loadSelectedFiles();
			$this->loadFileVersions();
			$this->goToPath();
		} elseif ($param->CurrentStepIndex === 5) {
			$this->file_relocation_opt = $_SESSION['file_relocation'];
		}
	}

	/**
	 * Cancel wizard.
	 *
	 * @return none
	 */
	public function wizardStop($sender, $param) {
		$this->resetWizard();
		$this->goToDefaultPage();
	}

	/**
	 * Load backup clients list (step 1).
	 *
	 * @param TActiveDropDownList $sender sender object
	 * @param TCommandParameter $param parameters object
	 * @return none
	 */
	public function loadBackupClients() {
		$client_list = array();
		$clients = $this->getModule('api')->get(array('clients'))->output;
		for ($i = 0; $i < count($clients); $i++) {
			$client_list[$clients[$i]->name] = $clients[$i]->name;
		}
		asort($client_list);
		$this->BackupClientName->dataSource = $client_list;
		$this->BackupClientName->dataBind();
	}

	/**
	 * Load restore client list.
	 *
	 * @return none
	 */
	public function loadRestoreClients() {
		$client_list = array();
		$clients = $this->getModule('api')->get(array('clients'))->output;
		for ($i = 0; $i < count($clients); $i++) {
			$client_list[$clients[$i]->name] = $clients[$i]->name;
		}
		$this->RestoreClient->SelectedValue = $this->BackupClientName->SelectedValue;
		$this->RestoreClient->dataSource = $client_list;
		$this->RestoreClient->dataBind();
	}

	/**
	 * Load backups for selected client (Step 2).
	 *
	 * @return none
	 */
	public function loadBackupsForClient() {
		$clientid = $this->getBackupClientId();
		$jobs_for_client = $this->getModule('api')->get(array('clients', 'jobs', $clientid))->output;
		$jobs = $this->getModule('misc')->objectToArray($jobs_for_client);
		$this->BackupsToRestore->DataSource = array_filter($jobs, array($this, 'isBackupJobToRestore'));
		$this->BackupsToRestore->dataBind();
	}

	/**
	 * Check if job can be used in restore.
	 *
	 * @param array $job job properties
	 * @return true if job should be listed to restore, otherwise false
	 */
	private function isBackupJobToRestore($job) {
		return ($job['type'] === 'B' && in_array($job['level'], $this->joblevel) && in_array($job['jobstatus'], $this->jobstatus));
	}

	public function loadBackupSelection($sender, $param) {
		$this->GroupBackupToRestoreField->Display = ($sender->ID == $this->GroupBackupSelection->ID) ? 'Dynamic' : 'None';
		$this->BackupToRestoreField->Display = ($sender->ID == $this->OnlySelectedBackupSelection->ID) ? 'Dynamic' : 'None';
		$this->setBrowserFiles();
		$this->setFileVersions();
		$this->setFilesToRestore();
		$this->markFileToRestore(null, null);
		$_SESSION['restore_path'] = array();
	}


	/**
	 * Get selected backup client identifier.
	 *
	 * @return mixed client identifier or null if no clientid found
	 */
	public function getBackupClientId() {
		$clientid = null;
		$clients = $this->getModule('api')->get(array('clients'))->output;
		for ($i = 0; $i < count($clients); $i++) {
			if ($clients[$i]->name === $this->BackupClientName->SelectedValue) {
				$clientid = $clients[$i]->clientid;
				break;
			}
		}
		return $clientid;
	}

	/**
	 * Load backup jobs to restore for group most recent backups feature.
	 *
	 * @return none
	 */
	public function loadGroupBackupToRestore() {
		$jobs = $this->getModule('api')->get(array('jobs'))->output;
		$jobs = $this->getModule('misc')->objectToArray($jobs);
		$clientid = $this->getBackupClientId();
		$job_group_list = array();
		for ($i = 0; $i < count($jobs); $i++) {
			$job = $this->getModule('misc')->objectToArray($jobs[$i]);
			if ($this->isBackupJobToRestore($jobs[$i]) && $jobs[$i]['clientid'] === $clientid) {
				$job_group_list[$jobs[$i]['name']] = $jobs[$i]['name'];
			}
		}

		$this->GroupBackupToRestore->dataSource = $job_group_list;
		$this->GroupBackupToRestore->dataBind();
	}

	/**
	 * Load filesets to restore for group most recent backups feature.
	 *
	 * @return none
	 */
	public function loadGroupFileSetToRestore() {
		$filesets = $this->getModule('api')->get(array('filesets', 'info'))->output;
		$fileset_list = array();
		for ($i = 0; $i < count($filesets); $i++) {
			$fileset_list[$filesets[$i]->filesetid] = $filesets[$i]->fileset . ' (' . $filesets[$i]->createtime . ')';
		}
		asort($fileset_list);

		$this->GroupBackupFileSet->dataSource = $fileset_list;
		$this->GroupBackupFileSet->dataBind();
	}

	/**
	 * Prepare left file browser content.
	 *
	 * @return none
	 */
	private function prepareBrowserContent() {
		$jobids = $this->getElementaryBackup();
		$elements = array();
		if (!empty($jobids)) {
			// generating Bvfs may take a moment
			$this->generateBvfsCache($jobids);

			// get directories list
			$bvfs_dirs = $this->getModule('api')->set(
				array('bvfs', 'lsdirs'),
				array('jobids' => $jobids, 'path' => implode($_SESSION['restore_path']))
			);
			$dirs = $this->getModule('misc')->parseBvfsList($bvfs_dirs->output);

			// get files list
			$bvfs_files = $this->getModule('api')->set(
				array('bvfs', 'lsfiles'),
				array('jobids' => $jobids, 'path' =>  implode($_SESSION['restore_path']))
			);
			$files = $this->getModule('misc')->parseBvfsList($bvfs_files->output);

			$elements = array_merge($dirs, $files);
			if(count($_SESSION['restore_path']) > 0) {
				array_unshift($elements, $this->browser_root_dir);
			}
		}
		$this->loadBrowserFiles($elements);
	}

	/*
	 * Get single elementary backup job identifiers.
	 *
	 * @return string comma separated job identifiers
	 */
	private function getElementaryBackup() {
		$jobids = '';
		if($this->OnlySelectedBackupSelection->Checked && isset($_SESSION['restore_single_jobid'])) {
			$jobs = $this->getModule('api')->get(
				array('bvfs', 'getjobids', $_SESSION['restore_single_jobid'])
			);
			$ids = is_object($jobs) ? $jobs->output : array();
			foreach ($ids as $jobid) {
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
			if (count($jobs_recent->output) > 0) {
				$ids = $jobs_recent->output;
				$jobids = implode(',', $ids);
			}
		}
		return $jobids;
	}

	/**
	 * Load path callback method.
	 * Used for manually typed paths in path field.
	 *
	 * @param TActiveLinkButton $sender sender object
	 * @param TEventParameter $param events parameter
	 * @return none
	 */
	public function loadPath($sender, $param) {
		$path = explode('/', $this->PathField->Text);
		$path_len = count($path);
		for ($i = 0; $i < count($path); $i++) {
			if ($i == ($path_len - 1) && empty($path[$i])) {
				// last path dir is slash so not add slash to last element
				break;
			}
			$path[$i] .= '/';
		}
		$this->goToPath($path, true);
	}

	/**
	 * Go to specific path in the file browser.
	 * There is possible to pass both single directory 'somedir'
	 * or whole path '/etc/somedir'.
	 *
	 * @param string $path path to go
	 * @param bool $full_path determines if $path param is full path or relative path (singel directory)
	 * @return none
	 */
	private function goToPath($path = '', $full_path = false) {
		if(!empty($path) && !$full_path) {
			if($path == $this->browser_up_dir['name']) {
				array_pop($_SESSION['restore_path']);
			} elseif($path == $this->browser_root_dir['name']) {
				$_SESSION['restore_path'] = array();
			} else {
				array_push($_SESSION['restore_path'], $path);
			}
		}
		if ($full_path && is_array($path)) {
			$_SESSION['restore_path'] = $path;
		}
		$this->setBrowserPath();
		$this->prepareBrowserContent();
	}

	/**
	 * Add/mark file to restore.
	 * Used as callback to drag&drop browser elements.
	 *
	 * @param object $sender sender object
	 * @param object $param param object
	 * @return none
	 */
	public function addFileToRestore($sender, $param) {
		$fileid = null;
		$source_element_id = null;
		$file_prop = array();
		if (isset($param->CallbackParameter)) {
			$id_parts = explode('_', $sender->ClientID, 6);
			$source_element_id = $id_parts[3];
			$fileid = $param->CallbackParameter;
		} else {
			$control = $param->getDroppedControl();
		        $item = $control->getNamingContainer();
			$id_parts = explode('_', $param->getDragElementID(), 6);
			$source_element_id = $id_parts[3];
		}
		if($source_element_id == $this->VersionsDataGrid->ID) {
			if (is_null($fileid)) {
				$fileid = $this->VersionsDataGrid->getDataKeys()->itemAt($item->getItemIndex());
			}
			$file_prop = $this->getFileVersions($fileid);
		} else {
			if (is_null($fileid)) {
				$fileid = $this->DataGridFiles->getDataKeys()->itemAt($item->getItemIndex());
			}
			$file_prop = $this->getBrowserFile($fileid);
		}
		if($file_prop['name'] != $this->browser_root_dir['name'] && $file_prop['name'] != $this->browser_up_dir['name']) {
			$this->markFileToRestore($fileid, $file_prop);
			$this->loadSelectedFiles();
		}
	}

	/**
	 * Remove file from files marked to restre.
	 *
	 * @param TActiveImageButton $sender remove button object
	 * @param TEventParameter $param param object
	 * @return none
	 */
	public function removeSelectedFile($sender, $param) {
		$fileid = $param->CallbackParameter;
		$this->unmarkFileToRestore($fileid);
		$this->loadSelectedFiles();
	}

	/**
	 * Get file backed up versions.
	 * Called as callback on file element click.
	 *
	 * @param TCallback $sender sender object
	 * @param object $param param object
	 * @return none
	 */
	public function getVersions($sender, $param) {
		list($filename, $pathid, $filenameid, $jobid) = explode('|', $param->CallbackParameter, 4);
		if($filenameid == 0) {
			$this->goToPath($filename);
			return;
		}
		$clientname = $this->BackupClientName->SelectedValue;
		$versions = $this->getModule('api')->get(array('bvfs', 'versions', $clientname, $jobid, $pathid, $filenameid))->output;
		$file_versions = $this->getModule('misc')->parseFileVersions($filename, $versions);
		$this->setFileVersions($file_versions);
		$this->VersionsDataGrid->dataSource = $file_versions;
		$this->VersionsDataGrid->dataBind();
		$this->loadSelectedFiles();
	}

	/**
	 * Refresh/re-render selected files list.
	 *
	 * @param TDropContainer $sender sender object
	 * @param TEventParameter $param param object
	 * @return none
	 */
	public function refreshSelectedFiles($sender, $param) {
		$this->loadSelectedFiles();
		$this->SelectedVersionsDropper->render($param->NewWriter);
	}

	/*
	 * Load file browser files to list.
	 *
	 * @param array $files files to list.
	 * @return none
	 */
	private function loadBrowserFiles($files) {
		$this->setBrowserFiles($files);
		$this->DataGridFiles->dataSource = $files;
		$this->DataGridFiles->dataBind();
	}

	/**
	 * Load file versions area.
	 *
	 * @return none;
	 */
	private function loadFileVersions() {
		$this->VersionsDataGrid->dataSource = $_SESSION['files_versions'];
		$this->VersionsDataGrid->dataBind();
	}

	/**
	 * Load selected files in drop area.
	 *
	 * @return none
	 */
	private function loadSelectedFiles() {
		$this->SelectedVersionsDataGrid->dataSource = $_SESSION['restore'];
		$this->SelectedVersionsDataGrid->dataBind();
	}

	/**
	 * Set file browser path field.
	 *
	 * @return none
	 */
	private function setBrowserPath() {
		$this->PathField->Text = implode($_SESSION['restore_path']);
	}

	/**
	 * Generate Bvfs cache by job identifiers.
	 *
	 * @param string $jobids comma separated job identifiers
	 * @return none
	 */
	private function generateBvfsCache($jobids) {
		$this->getModule('api')->set(
			array('bvfs', 'update'),
			array('jobids' => $jobids)
		);
	}

	/**
	 * Set versions for selected file.
	 *
	 * @param array $versions file versions data
	 * @return none
	 */
	private function setFileVersions($versions = array()) {
		$_SESSION['files_versions'] = $versions;
	}

	/**
	 * Get file versions for specified fileid.
	 *
	 * @param integer $fileid file identifier
	 * @return none
	 */
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

	/**
	 * Set browser files.
	 *
	 * @param array $files file list
	 * @return none
	 */
	private function setBrowserFiles($files = array()) {
		$_SESSION['files_browser'] = $files;
	}

	/**
	 * Get browser file by fileid.
	 *
	 * @param integer $fileid file identifier
	 * @return none
	 */
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

	/**
	 * Mark file to restore.
	 *
	 * @param integer $fileid file identifier
	 * @param array $file file properties to mark
	 * @return none
	 */
	private function markFileToRestore($fileid, $file) {
		if($fileid === null) {
			$_SESSION['restore'] = array();
		} elseif($file['name'] != $this->browser_root_dir['name'] && $file['name'] != $this->browser_up_dir['name']) {
			$_SESSION['restore'][$fileid] = $file;
		}
	}

	/**
	 * Unmark file to restore.
	 *
	 * @param integer $fileid file identifier
	 * @return none
	 */
	private function unmarkFileToRestore($fileid) {
		if(array_key_exists($fileid, $_SESSION['restore'])) {
			unset($_SESSION['restore'][$fileid]);
		}
	}

	/**
	 * Get files to restore.
	 *
	 * @return array list with files to restore
	 */
	public function getFilesToRestore() {
		return $_SESSION['restore'];
	}

	/**
	 * Set files to restore
	 *
	 * @param array $files files to restore
	 * @return none
	 */
	public function setFilesToRestore($files = array()) {
		$_SESSION['restore'] = $files;
	}

	/**
	 * Get all restore elements (fileids and dirids).
	 *
	 * @param bool $as_object return result as object
	 * @return array list fileids and dirids
	 */
	public function getRestoreElements($as_object = false) {
		$fileids = array();
		$dirids = array();
		$findexes = array();
		foreach ($this->getFilesToRestore() as $fileid => $properties) {
			if ($properties['type'] == 'dir') {
				$dirids[] = $properties['pathid'];
			} elseif ($properties['type'] == 'file') {
				$fileids[] = $fileid;
				if ($properties['lstat']['linkfi'] !== 0) {
					$findexes[] = $properties['jobid'] . ',' . $properties['lstat']['linkfi'];
				}
			}
		}
		$ret = array('fileid' => $fileids, 'dirid' => $dirids, 'findex' => $findexes);
		if($as_object === true) {
			$ret = (object)$ret;
		}
		return $ret;
	}

	/**
	 * Wizard finish method.
	 *
	 * @return none
	 */
	public function wizardCompleted() {
		$jobids = $this->getElementaryBackup();
		$path = self::BVFS_PATH_PREFIX . getmypid();
		$restore_elements = $this->getRestoreElements();
		$cmd_props = array('jobids' => $jobids, 'path' => $path);
		$is_element = false;
		if(count($restore_elements['fileid']) > 0) {
			$cmd_props['fileid'] = implode(',', $restore_elements['fileid']);
			$is_element = true;
		}
		if(count($restore_elements['dirid']) > 0) {
			$cmd_props['dirid'] = implode(',', $restore_elements['dirid']);
			$is_element = true;
		}
		if (count($restore_elements['findex']) > 0) {
			$cmd_props['findex'] = implode(',', $restore_elements['findex']);
			$is_element = true;
		}

		$jobid = null;
		if ($is_element) {
			$this->getModule('api')->create(array('bvfs', 'restore'), $cmd_props);
			$restore_props = array();
			$restore_props['rpath'] = $path;
			$restore_props['client'] = $this->RestoreClient->SelectedValue;
			$restore_props['priority'] = intval($this->RestoreJobPriority->Text);
			if ($_SESSION['file_relocation'] == 2) {
				if (!empty($this->RestoreStripPrefix->Text)) {
					$restore_props['strip_prefix'] = $this->RestoreStripPrefix->Text;
				}
				if (!empty($this->RestoreAddPrefix->Text)) {
					$restore_props['add_prefix'] = $this->RestoreAddPrefix->Text;
				}
				if (!empty($this->RestoreAddSuffix->Text)) {
					$restore_props['add_suffix'] = $this->RestoreAddSuffix->Text;
				}
			} elseif ($_SESSION['file_relocation'] == 3) {
				if (!empty($this->RestoreRegexWhere->Text)) {
					$restore_props['regex_where'] = $this->RestoreRegexWhere->Text;
				}
			}
			if (!array_key_exists('add_prefix', $restore_props)) {
				$restore_props['where'] =  $this->RestorePath->Text;
			}
			$restore_props['replace'] = $this->ReplaceFiles->SelectedValue;
			$restore_props['restorejob'] = $this->RestoreJob->SelectedValue;

			$ret = $this->getModule('api')->create(array('jobs', 'restore'), $restore_props);
			$jobid = $this->getModule('misc')->findJobIdStartedJob($ret->output);
		}
		$url_params = array('open' => 'Job');
		if (is_numeric($jobid)) {
			$url_params['id'] = $jobid;
		}
		$this->goToDefaultPage($url_params);
	}

	/**
	 * Load restore jobs on the list.
	 *
	 * @return none
	 */
	private function loadRestoreJobs() {
		$restore_job_tasks = $this->getModule('api')->get(array('jobs', 'tasks', 'type', 'R'))->output;
		$jobs = array();
		foreach ($restore_job_tasks as $director => $restore_jobs) {
			$jobs = array_merge($jobs, $restore_jobs);
		}
		$this->RestoreJob->DataSource = array_combine($jobs, $jobs);
		$this->RestoreJob->dataBind();
	}

	private function loadRequiredVolumes() {
		$volumes = array();
		foreach ($this->getFilesToRestore() as $fileid => $props) {
			// it can be expensive for many restore paths
			$result = $this->getModule('api')->get(array('volumes', 'required', $props['jobid'], $fileid));
			if ($result->error === 0) {
				for ($i = 0; $i < count($result->output); $i++) {
					$volumes[$result->output[$i]->volume] = array(
						'volume' => $result->output[$i]->volume,
						'inchanger' => $result->output[$i]->inchanger
					);
				}
			}
		}
		$this->RestoreVolumes->DataSource = array_values($volumes);
		$this->RestoreVolumes->dataBind();
	}

	/**
	 * Reset wizard.
	 * All fields are back to initial form.
	 *
	 * @return none
	 */
	private function resetWizard() {
		$this->setBrowserFiles();
		$this->setFileVersions();
		$this->setFilesToRestore();
		$this->markFileToRestore(null, null);
		$this->loadRestoreJobs();
		$_SESSION['restore_path'] = array();
		$_SESSION['restore_single_jobid'] = null;
		unset($_SESSION['file_relocation']);
	}
}
?>
