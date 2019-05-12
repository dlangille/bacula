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
		$result = null;
		if ($this->Barcodes->Checked == true) {
			$params = array(
				'slots' => $this->SlotsLabel->Text,
				'drive' => $this->DriveLabel->Text,
				'storageid' => $this->StorageLabel->SelectedValue,
				'poolid' => $this->PoolLabel->SelectedValue
			);
			$result = $this->getModule('api')->create(array('volumes', 'label', 'barcodes'), $params);
			if ($result->error === 0 && count($result->output) === 1) {
				$out = json_decode($result->output[0]);
				if (is_object($out) && property_exists($out, 'out_id')) {
					$result = $this->getLabelBarcodesOutput($out->out_id);
					$this->getPage()->getCallbackClient()->callClientFunction('label_volume_output_refresh', array($out->out_id));
				}
			}
		} else {
			$params = array(
				'slot' => $this->SlotLabel->Text,
				'volume' => $this->LabelName->Text,
				'drive' => $this->DriveLabel->Text,
				'storageid' => $this->StorageLabel->SelectedValue,
				'poolid' => $this->PoolLabel->SelectedValue
			);
			$result = $this->getModule('api')->create(array('volumes', 'label'), $params);
			if ($result->error === 0 && count($result->output) === 1) {
				$out = json_decode($result->output[0]);
				if (is_object($out) && property_exists($out, 'out_id')) {
					$result = $this->getLabelOutput($out->out_id);
					$this->getPage()->getCallbackClient()->callClientFunction('label_volume_output_refresh', array($out->out_id));
				}
			}
		}
		if ($result->error === 0) {
			$this->getPage()->getCallbackClient()->callClientFunction('set_labeling_status', array('loading'));
			$this->LabelVolumeLog->Text = implode('', $result->output);
		} else {
			$this->LabelVolumeLog->Text = $result->output;
		}
	}

	private function getLabelOutput($out_id) {
		$result = $this->getModule('api')->get(
			array('volumes', 'label', '?out_id=' . rawurlencode($out_id))
		);
		return $result;
	}

	private function getLabelBarcodesOutput($out_id) {
		$result = $this->getModule('api')->get(
			array('volumes', 'label', 'barcodes', '?out_id=' . rawurlencode($out_id))
		);
		return $result;
	}

	public function refreshOutput($sender, $param) {
		$out_id = $param->getCallbackParameter();
		$result = null;
		if ($this->Barcodes->Checked == true) {
			$result = $this->getLabelBarcodesOutput($out_id);
		} else {
			$result = $this->getLabelOutput($out_id);
		}

		if ($result->error === 0) {
			if (count($result->output) > 0) {
				$this->LabelVolumeLog->Text = implode('', $result->output);
				$this->getPage()->getCallbackClient()->callClientFunction('label_volume_output_refresh', array($out_id));
			} else {
				$this->getPage()->getCallbackClient()->callClientFunction('set_labeling_status', array('finish'));
			}
		} else {
			$this->LabelVolumeLog->Text = $result->output;
		}
	}
}
?>
