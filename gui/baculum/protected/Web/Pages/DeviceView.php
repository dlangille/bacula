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

Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('Application.Web.Class.BaculumWebPage'); 

class DeviceView extends BaculumWebPage {

	const USE_CACHE = true;

	const STORAGEID = 'StorageId';
	const STORAGE_NAME = 'StorageName';
	const DEVICE_NAME = 'DeviceName';

	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsPostBack || $this->IsCallBack) {
			return;
		}
		if ($this->Request->contains('storageid')) {
			$this->setStorageId($this->Request['storageid']);
		}
		if ($this->Request->contains('device')) {
			$this->setDeviceName($this->Request['device']);
		}
		$storageid = $this->getStorageId();
		if ($storageid > 0) {
			$storage = $this->Application->getModule('api')->get(
				array('storages', $this->getStorageId()),
				null,
				true,
				self::USE_CACHE
			);
			if ($storage->error === 0) {
				$this->setStorageName($storage->output->name);
			}
		}
	}

	/**
	 * Set storage storageid.
	 *
	 * @return none;
	 */
	public function setStorageId($storageid) {
		$storageid = intval($storageid);
		$this->setViewState(self::STORAGEID, $storageid, 0);
	}

	/**
	 * Get storage storageid.
	 *
	 * @return integer storageid
	 */
	public function getStorageId() {
		return $this->getViewState(self::STORAGEID, 0);
	}

	/**
	 * Set storage name.
	 *
	 * @return none;
	 */
	public function setStorageName($storage_name) {
		$this->setViewState(self::STORAGE_NAME, $storage_name);
	}

	/**
	 * Get storage name.
	 *
	 * @return string storage name
	 */
	public function getStorageName() {
		return $this->getViewState(self::STORAGE_NAME);
	}


	/**
	 * Set device name.
	 *
	 * @return none;
	 */
	public function setDeviceName($device_name) {
		$this->setViewState(self::DEVICE_NAME, $device_name);
	}

	/**
	 * Get device name.
	 *
	 * @return string device name
	 */
	public function getDeviceName() {
		return $this->getViewState(self::DEVICE_NAME);
	}

	public function setDevice($sender, $param) {
		$this->DeviceConfig->setComponentName($_SESSION['sd']);
		$this->DeviceConfig->setResourceName($this->getDeviceName());
		$this->DeviceConfig->setLoadValues(true);
		$this->DeviceConfig->raiseEvent('OnDirectiveListLoad', $this, null);
	}

	public function mount($sender, $param) {
		$slot = intval($this->Slot->Text);
		$mount = $this->getModule('api')->get(
			array('storages', 'mount', $this->getStorageId(), $this->getDeviceName(), $slot)
		)->output;
		$this->DeviceLog->Text = implode(PHP_EOL, $mount);
	}

	public function umount($sender, $param) {
		$umount = $this->getModule('api')->get(
			array('storages', 'umount', $this->getStorageId(), $this->getDeviceName())
		)->output;
		$this->DeviceLog->Text = implode(PHP_EOL, $umount);

	}

	public function release($sender, $param) {
		$release = $this->getModule('api')->get(
			array('storages', 'release', $this->getStorageId(), $this->getDeviceName())
		)->output;
		$this->DeviceLog->Text = implode(PHP_EOL, $release);
	}
}
?>
