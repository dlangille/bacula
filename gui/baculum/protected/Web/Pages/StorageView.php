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

Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.ActiveControls.TActiveRepeater');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('Application.Web.Class.BaculumWebPage'); 

class StorageView extends BaculumWebPage {

	const STORAGEID = 'StorageId';
	const STORAGE_NAME = 'StorageName';
	const IS_AUTOCHANGER = 'IsAutochanger';
	const DEVICE_NAME = 'DeviceName';

	const USE_CACHE = true;

	protected $admin = true;

	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsPostBack || $this->IsCallBack) {
			return;
		}
		$storageid = 0;
		if ($this->Request->contains('storageid')) {
			$storageid = intval($this->Request['storageid']);
		}
		$storage = $this->Application->getModule('api')->get(
			array('storages', $storageid),
			null,
			true,
			self::USE_CACHE
		)->output;
		$this->setStorageId($storage->storageid);
		$this->setStorageName($storage->name);
		$is_autochanger = ($storage->autochanger == 1);
		$this->setIsAutochanger($is_autochanger);
		$this->Autochanger->Display = $is_autochanger ? 'Dynamic': 'None';
		$storageshow = $this->Application->getModule('api')->get(
			array('storages', 'show', $storage->storageid)
		)->output;
		$this->StorageLog->Text = implode(PHP_EOL, $storageshow);
		$this->setStorageDevice($storageshow);
		$this->setDevices();
	}

	public function setStorageDevice($storageshow) {
		/**
		 * Note, it cannot be get by api config because user can have bdirjson not configured.
		 */
		for ($i = 0; $i < count($storageshow); $i++) {
			if (preg_match('/^\s+DeviceName=(?P<device>[\s\S]+)\sMediaType=/', $storageshow[$i], $match) === 1) {
				$this->setDeviceName($match['device']);
			}
		}
	}

	public function setDevices() {
		$devices = array();
		if ($this->getIsAutochanger() && !empty($_SESSION['sd'])) {
			$result = $this->Application->getModule('api')->get(
				array(
					'config',
					'sd',
					'Autochanger',
					$this->getDeviceName()
				)
			);
			if ($result->error === 0) {
				$devices = $result->output->Device;
			}
		} else {
			$devices = array($this->getDeviceName());
		}
		$this->Devices->DataSource = $devices;
		$this->Devices->dataBind();
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

	/**
	 * Check if storage is autochanger
	 *
	 * @return bool true if autochanger, otherwise false
	 */
	public function getIsAutochanger() {
		return $this->getViewState(self::IS_AUTOCHANGER, false);
	}

	/**
	 * Set autochanger value for storage
	 *
	 * @return none;
	 */
	public function setIsAutochanger($is_autochanger) {
		settype($is_autochanger, 'bool');
		$this->setViewState(self::IS_AUTOCHANGER, $is_autochanger);
	}

	public function status($sender, $param) {
		$status = $this->getModule('api')->get(
			array('storages', 'status', $this->getStorageId())
		)->output;
		$this->StorageLog->Text = implode(PHP_EOL, $status);
	}

	public function mount($sender, $param) {
		$drive = $this->getIsAutochanger() ? intval($this->Drive->Text) : 0;
		$slot = $this->getIsAutochanger() ? intval($this->Slot->Text) : 0;
		$mount = $this->getModule('api')->get(
			array('storages', 'mount', $this->getStorageId(), $drive, $slot)
		)->output;
		$this->StorageLog->Text = implode(PHP_EOL, $mount);
	}

	public function umount($sender, $param) {
		$drive = $this->getIsAutochanger() ? intval($this->Drive->Text) : 0;
		$umount = $this->getModule('api')->get(
			array('storages', 'umount', $this->getStorageId(), $drive)
		)->output;
		$this->StorageLog->Text = implode(PHP_EOL, $umount);

	}

	public function release($sender, $param) {
		$drive = $this->getIsAutochanger() ? intval($this->Drive->Text) : 0;
		$release = $this->getModule('api')->get(
			array('storages', 'release', $this->getStorageId(), $drive)
		)->output;
		$this->StorageLog->Text = implode(PHP_EOL, $release);
	}

	public function setAutochanger($sender, $param) {
		if ($this->getIsAutochanger()) {
			$this->AutochangerConfig->setComponentName($_SESSION['sd']);
			$this->AutochangerConfig->setResourceName($this->getDeviceName());
			$this->AutochangerConfig->setLoadValues(true);
			$this->AutochangerConfig->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}

	public function setStorage($sender, $param) {
		$this->StorageConfig->setComponentName($_SESSION['sd']);
		$this->StorageConfig->setResourceName($this->getStorageName());
		$this->StorageConfig->setLoadValues(true);
		$this->StorageConfig->raiseEvent('OnDirectiveListLoad', $this, null);
	}
}
?>
