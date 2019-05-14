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


Prado::using('System.Web.UI.ActiveControls.TCallback');
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
		$url_params = array();
		if($this->Barcodes->Checked == true) {
			$url_params = array('volumes', 'update', 'barcodes');
		} else {
			$url_params = array('volumes', 'update');
		}
		$params = array(
			'slots' => $this->SlotsUpdate->Text,
			'drive' => $this->DriveUpdate->Text,
			'storageid' => $this->StorageUpdate->SelectedValue
		);

		$result = $this->getModule('api')->set($url_params, $params);

		if ($result->error === 0 && count($result->output) === 1) {
			$out = json_decode($result->output[0]);
			if (is_object($out) && property_exists($out, 'out_id')) {
				$result = $this->getUpdateSlotsOutput($out->out_id);
				$this->getPage()->getCallbackClient()->callClientFunction('update_slots_output_refresh', array($out->out_id));
			}
		}
		if ($result->error === 0) {
			$this->getPage()->getCallbackClient()->callClientFunction('set_updating_status', array('loading'));
			$this->UpdateSlotsLog->Text = implode('', $result->output);
		} else {
			$this->UpdateSlotsLog->Text = $result->output;
		}
	}

	private function getUpdateSlotsOutput($out_id) {
		$result = $this->getModule('api')->get(
			array('volumes', 'update', '?out_id=' . rawurlencode($out_id))
		);
		return $result;
	}

	private function getUpdateSlotsBarcodesOutput($out_id) {
		$result = $this->getModule('api')->get(
			array('volumes', 'update', 'barcodes', '?out_id=' . rawurlencode($out_id))
		);
		return $result;
	}

	public function refreshOutput($sender, $param) {
		$out_id = $param->getCallbackParameter();
		$result = null;
		if ($this->Barcodes->Checked == true) {
			$result = $this->getUpdateSlotsBarcodesOutput($out_id);
		} else {
			$result = $this->getUpdateSlotsOutput($out_id);
		}

		if ($result->error === 0) {
			if (count($result->output) > 0) {
				$this->UpdateSlotsLog->Text = implode('', $result->output);
				$this->getPage()->getCallbackClient()->callClientFunction('update_slots_output_refresh', array($out_id));
			} else {
				$this->getPage()->getCallbackClient()->callClientFunction('set_updating_status', array('finish'));
			}
		} else {
			$this->UpdateSlotsLog->Text = $result->output;
		}
	}
}
?>
