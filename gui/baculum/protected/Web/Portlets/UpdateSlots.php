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


Prado::using('System.Web.UI.ActiveControls.TActiveCheckBox');
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('Application.Web.Portlets.Portlets');

class UpdateSlots extends Portlets {

	public function loadValues() {
		$storages = $this->getModule('api')->get(array('storages'));
		$storage_list = array();
		if ($storages->error === 0) {
			foreach($storages->output as $storage) {
				$storage_list[$storage->storageid] = $storage->name;
			}
		}
		$this->StorageUpdate->dataSource = $storage_list;
		$this->StorageUpdate->dataBind();
	}

	public function update($sender, $param) {
		$cmd = array('label');
		$cmd = array('update');
		$cmd[] = 'slots="' . $this->SlotsUpdate->Text . '"';
		if($this->Barcodes->Checked == false) {
			$cmd[] = 'scan';
		}
		$cmd[] = 'drive="' . $this->DriveUpdate->Text . '"';
		$cmd[] = 'storage="'. $this->StorageUpdate->SelectedItem->Text . '"';
		$result = $this->getModule('api')->set(array('console'), $cmd);
		if ($result->error === 0) {
			$this->UpdateSlotsLog->Text = implode(PHP_EOL, $result->output);
		} else {
			$this->UpdateSlotsLog->Text = $result->output;
		}
	}
}
?>
