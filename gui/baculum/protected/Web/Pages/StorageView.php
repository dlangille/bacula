<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2020 Kern Sibbald
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
Prado::using('Application.Common.Class.Params');
Prado::using('Application.Web.Class.BaculumWebPage'); 

/**
 * Storage view page.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class StorageView extends BaculumWebPage {

	const STORAGEID = 'StorageId';
	const STORAGE_NAME = 'StorageName';
	const IS_AUTOCHANGER = 'IsAutochanger';
	const DEVICE_NAME = 'DeviceName';

	const USE_CACHE = true;

	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsPostBack || $this->IsCallBack) {
			return;
		}
		$storageid = 0;
		if ($this->Request->contains('storageid')) {
			$storageid = intval($this->Request['storageid']);
		} elseif ($this->Request->contains('storage')) {
			$result = $this->getModule('api')->get(array('storages'));
			if ($result->error === 0) {
				for ($i = 0; $i < count($result->output); $i++) {
					if ($this->Request['storage'] === $result->output[$i]->name) {
						$storageid = $result->output[$i]->storageid;
						break;
					}
				}
			}
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
			array('storages', $storage->storageid, 'show')
		)->output;
		$this->StorageActionLog->Text = implode(PHP_EOL, $storageshow);
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
			/**
			 * NOTE: Here is called only Main API host. For storage daemons
			 * on other hosts it can cause a problem. So far, there
			 * is no 100% way to unambiguously determine basing on storage daemon
			 * configuration if autochanger comes from Main or from other API host.
			 * The problem will be if on Main host is defined autochanger
			 * with the same name as autochanger from requested Storage here.
			 * @TODO: Find a way to solve it.
			 */
			$result = $this->Application->getModule('api')->get(
				array(
					'config',
					'sd',
					'Autochanger',
					$this->getDeviceName()
				)
			);
			if ($result->error === 0 && is_object($result->output)) {
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
		$raw_status = $this->getModule('api')->get(
			['storages', $this->getStorageId(), 'status']
		)->output;
		$this->StorageLog->Text = implode(PHP_EOL, $raw_status);

		$query_str = '?name=' . rawurlencode($this->getStorageName()) . '&type=header';
		$graph_status = $this->getModule('api')->get(
			['status', 'storage', $query_str]
		);
		$storage_status = [
			'header' => [],
			'devices' => [],
			'running' => [],
			'terminated' => [],
			'version' => Params::getComponentVersion($raw_status)
		];
		if ($graph_status->error === 0) {
			$storage_status['header'] = $graph_status->output;
		}

		// running
		$query_str = '?name=' . rawurlencode($this->getStorageName()) . '&type=running';
		$graph_status = $this->getModule('api')->get(
			array('status', 'storage', $query_str)
		);
		if ($graph_status->error === 0) {
			$storage_status['running'] = $graph_status->output;
		}

		// terminated
		$query_str = '?name=' . rawurlencode($this->getStorageName()) . '&type=terminated';
		$graph_status = $this->getModule('api')->get(
			array('status', 'storage', $query_str)
		);
		if ($graph_status->error === 0) {
			$storage_status['terminated'] = $graph_status->output;
		}

		// devices
		$query_str = '?name=' . rawurlencode($this->getStorageName()) . '&type=devices';
		$graph_status = $this->getModule('api')->get(
			array('status', 'storage', $query_str)
		);
		if ($graph_status->error === 0) {
			$storage_status['devices'] = $graph_status->output;
		}

		// show
		$query_str = '?output=json';
		$show = $this->getModule('api')->get(
			array('storages', 'show', $query_str)
		);
		if ($show->error === 0) {
			$storage_status['show'] = $show->output;
		}

		$this->getCallbackClient()->callClientFunction('init_graphical_storage_status', [$storage_status]);
	}

	public function mount($sender, $param) {
		$drive = $this->getIsAutochanger() ? intval($this->Drive->Text) : 0;
		$slot = $this->getIsAutochanger() ? intval($this->Slot->Text) : 0;
		$params = [
			'drive' => $drive,
			'slot' => $slot
		];
		$query = '?' . http_build_query($params);
		$mount = $this->getModule('api')->get(
			array('storages', $this->getStorageId(), 'mount', $query)
		);
		if ($mount->error === 0) {
			$this->StorageActionLog->Text = implode(PHP_EOL, $mount->output);
		} else {
			$this->StorageActionLog->Text = $mount->output;
		}
	}

	public function umount($sender, $param) {
		$drive = $this->getIsAutochanger() ? intval($this->Drive->Text) : 0;
		$params = [
			'drive' => $drive
		];
		$query = '?' . http_build_query($params);
		$umount = $this->getModule('api')->get(
			array('storages', $this->getStorageId(), 'umount', $query)
		);
		if ($umount->error === 0) {
			$this->StorageActionLog->Text = implode(PHP_EOL, $umount->output);
		} else {
			$this->StorageActionLog->Text = $umount->output;
		}

	}

	public function release($sender, $param) {
		$drive = $this->getIsAutochanger() ? intval($this->Drive->Text) : 0;
		$params = [
			'drive' => $drive
		];
		$query = '?' . http_build_query($params);
		$release = $this->getModule('api')->get(
			array('storages', $this->getStorageId(), 'release', $query)
		);
		if ($release->error === 0) {
			$this->StorageActionLog->Text = implode(PHP_EOL, $release->output);
		} else {
			$this->StorageActionLog->Text = $release->output;
		}
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
