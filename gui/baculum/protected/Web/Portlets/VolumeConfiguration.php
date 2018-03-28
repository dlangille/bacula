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

Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActiveDataGrid');
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.ActiveControls.TActiveCheckBox');
Prado::using('System.Web.UI.ActiveControls.TCallback');
Prado::using('Application.Web.Portlets.Portlets');

class VolumeConfiguration extends Portlets {

	private $volumeStatesByDirectorOnly = array('Recycle', 'Purged', 'Error', 'Busy');

	private $volumeStatesForSet = array('Append', 'Archive', 'Disabled', 'Full', 'Used', 'Cleaning', 'Read-Only');

	public function configure($mediaId) {
		$voldata = $this->Application->getModule('api')->get(array('volumes', $mediaId))->output;
		$this->VolumeID->Text = $voldata->mediaid;
		$this->VolumeName->Text = $voldata->volumename;
		$volstates = $this->getVolumeStates(true);
		if (!in_array($voldata->volstatus, $volstates)) {
			array_push($volstates, $voldata->volstatus);
		}
		$volstatesSelect = array();
		for($i = 0; $i < count($volstates); $i++) {
			$volstatesSelect[$volstates[$i]] = $volstates[$i];
		}
		$this->VolumeStatus->DataSource = $volstatesSelect;
		$this->VolumeStatus->SelectedValue = $voldata->volstatus;
		$this->VolumeStatus->dataBind();
		$this->RetentionPeriod->Text = intval($voldata->volretention / 3600); // conversion to hours
		$this->UseDuration->Text = intval($voldata->voluseduration / 3600);  // conversion to hours
		$this->MaxVolJobs->Text = $voldata->maxvoljobs;
		$this->MaxVolFiles->Text = $voldata->maxvolfiles;
		$this->MaxVolBytes->Text = $voldata->maxvolbytes;
		$this->Slot->Text = $voldata->slot;
		$this->Recycle->Checked = ($voldata->recycle == 1);
		$this->Enabled->Checked = ($voldata->enabled == 1);
		$this->InChanger->Checked = ($voldata->inchanger == 1);
		$pools = $this->Application->getModule('api')->get(array('pools'))->output;
		$poolList = array();
		foreach($pools as $pool) {
			$poolList[$pool->poolid] = $pool->name;
		}
		$this->Pool->dataSource = $poolList;
		$this->Pool->SelectedValue = $voldata->poolid;
		$this->Pool->dataBind();

		$jobs_on_volume = $this->Application->getModule('api')->get(array('volumes', 'jobs', $voldata->mediaid))->output;
		$this->JobsOnVolume->DataSource = $this->Application->getModule('misc')->objectToArray($jobs_on_volume);
		$this->JobsOnVolume->dataBind();
	}

	public function prune($sender, $param) {
		$this->Application->getModule('api')->get(array('volumes', 'prune', $this->VolumeID->Text));
	}

	public function purge($sender, $param) {
		$this->Application->getModule('api')->get(array('volumes', 'purge', $this->VolumeID->Text));
	}

	public function apply($sender, $param) {
		$isInvalid = $this->RetentionPeriodValidator->IsValid === false || $this->UseDurationValidator->IsValid === false || $this->MaxVolJobsValidator->IsValid === false || $this->MaxVolFilesValidator->IsValid === false || $this->MaxVolBytesValidator->IsValid === false || $this->SlotValidator->IsValid === false;
		if($isInvalid) {
			return false;
		}
		$voldata = array();
		$voldata['mediaid'] = $this->VolumeID->Text;
		$voldata['volstatus'] = $this->VolumeStatus->SelectedValue;
		$voldata['poolid'] = $this->Pool->SelectedValue;
		$voldata['volretention'] = $this->RetentionPeriod->Text * 3600; // conversion to seconds
		$voldata['voluseduration'] = $this->UseDuration->Text * 3600;  // conversion to seconds
		$voldata['maxvoljobs'] = $this->MaxVolJobs->Text;
		$voldata['maxvolfiles'] = $this->MaxVolFiles->Text;
		$voldata['maxvolbytes'] = $this->MaxVolBytes->Text;
		$voldata['slot'] = $this->Slot->Text;
		$voldata['recycle'] = (integer)$this->Recycle->Checked;
		$voldata['enabled'] = (integer)$this->Enabled->Checked;
		$voldata['inchanger'] = (integer)$this->InChanger->Checked;
		$this->Application->getModule('api')->set(array('volumes', $voldata['mediaid']), $voldata);
	}

	public function openJob($sender, $param) {
		$jobid = $param->CallbackParameter;
		$params = array(
			'prev_window' => 'VolumeWindow'
		);
		$this->getPage()->JobConfiguration->configure($jobid, $params);
	}

	public function getVolumeStates($forSetOnly = false) {
		$states = ($forSetOnly === true ) ? $this->volumeStatesForSet : array_merge($this->volumeStatesByDirectorOnly, $this->volumeStatesForSet);
		return $states;
	}

	public function retentionPeriodValidator($sender, $param) {
		$isValid = preg_match('/^\d+$/', $this->RetentionPeriod->Text) && $this->RetentionPeriod->Text >= 0;
		$param->setIsValid($isValid);
	}

	public function useDurationValidator($sender, $param) {
		$isValid = preg_match('/^\d+$/', $this->UseDuration->Text) && $this->UseDuration->Text >= 0;
		$param->setIsValid($isValid);
	}

	public function maxVolJobsValidator($sender, $param) {
		$isValid = preg_match('/^\d+$/', $this->MaxVolJobs->Text) && $this->MaxVolJobs->Text >= 0;
		$param->setIsValid($isValid);
	}

	public function maxVolFilesValidator($sender, $param) {
		$isValid = preg_match('/^\d+$/', $this->MaxVolFiles->Text) && $this->MaxVolFiles->Text >= 0;
		$param->setIsValid($isValid);
	}

	public function maxVolBytesValidator($sender, $param) {
		$isValid = preg_match('/^\d+$/', $this->MaxVolBytes->Text) && $this->MaxVolBytes->Text >= 0;
		$param->setIsValid($isValid);
	}

	public function slotValidator($sender, $param) {
		$isValid = preg_match('/^\d+$/', $this->Slot->Text) && $this->Slot->Text >= 0;
		$param->setIsValid($isValid);
	}
}
?>
