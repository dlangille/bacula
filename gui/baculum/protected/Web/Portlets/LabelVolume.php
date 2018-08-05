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
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('Application.Web.Portlets.Portlets');

class LabelVolume extends Portlets {

	public function loadValues() {
		$storages = $this->getModule('api')->get(array('storages'));
		$storage_list = array();
		if ($storages->error === 0) {
			foreach($storages->output as $storage) {
				$storage_list[$storage->storageid] = $storage->name;
			}
		}
		$this->StorageLabel->dataSource = $storage_list;
		$this->StorageLabel->dataBind();

		$pools = $this->Application->getModule('api')->get(array('pools'));
		$pool_list = array();
		if ($pools->error === 0) {
			foreach($pools->output as $pool) {
				$pool_list[$pool->poolid] = $pool->name;
			}
		}
		$this->PoolLabel->dataSource = $pool_list;
		$this->PoolLabel->dataBind();
	}

	public function labelVolumes($sender, $param) {
		$cmd = array('label');
		if ($this->Barcodes->Checked == true) {
			$cmd[] = 'barcodes';
			$cmd[] = 'slots="' . $this->SlotsLabel->Text . '"';
		} else {
			$cmd[] = 'volume="' . $this->LabelName->Text . '"';
			$cmd[] = 'slot="' . $this->SlotLabel->Text . '"';
		}
		$cmd[] = 'drive="' . $this->DriveLabel->Text . '"';
		$cmd[] = 'storage="'. $this->StorageLabel->SelectedItem->Text . '"';
		$cmd[] = 'pool="'. $this->PoolLabel->SelectedItem->Text . '"';
		$result = $this->getModule('api')->set(array('console'), $cmd);
		if ($result->error === 0) {
			$this->LabelVolumeLog->Text = implode(PHP_EOL, $result->output);
		} else {
			$this->LabelVolumeLog->Text = $result->output;
		}
	}
}
?>
