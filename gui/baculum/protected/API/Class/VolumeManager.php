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

Prado::using('Application.API.Class.APIModule');
Prado::using('Application.API.Class.VolumeRecord');
Prado::using('Application.API.Class.Database');

class VolumeManager extends APIModule {

	public function getVolumes($limit) {
		$criteria = new TActiveRecordCriteria;
		$orderPool = 'PoolId';
		$orderVolume = 'VolumeName';
		$db_params = $this->getModule('api_config')->getConfig('db');
		if($db_params['type'] === Database::PGSQL_TYPE) {
		    $orderPool = strtolower($orderPool);
		    $orderVolume = strtolower($orderVolume);
		}
		$criteria->OrdersBy[$orderPool] = 'asc';
		$criteria->OrdersBy[$orderVolume] = 'asc';
		if(is_int($limit) && $limit > 0) {
			$criteria->Limit = $limit;
		}
		$volumes = VolumeRecord::finder()->findAll($criteria);
		$this->setExtraVariables($volumes);
		return $volumes;
	}

	public function getVolumesByPoolId($poolid) {
		$volumes = VolumeRecord::finder()->findAllBypoolid($poolid);
		$this->setExtraVariables($volumes);
		return $volumes;
	}

	public function getVolumeByPoolId($poolid) {
		$volume = VolumeRecord::finder()->findBypoolid($poolid);
		$this->setExtraVariables($volume);
		return $volume;
	}

	public function getVolumeByName($volumeName) {
		$volume = VolumeRecord::finder()->findByvolumename($volumeName);
		$this->setExtraVariables($volume);
		return $volume;
	}

	public function getVolumeById($volumeId) {
		$volume = VolumeRecord::finder()->findBymediaid($volumeId);
		$this->setExtraVariables($volume);
		return $volume;
	}

	private function setExtraVariables(&$volumes) {
		if (is_array($volumes)) {
			foreach($volumes as $volume) {
				$this->setWhenExpire($volume);
			}
		} else {
			$this->setWhenExpire($volumes);
		}
	}

	private function setWhenExpire(&$volume) {
		$volstatus = strtolower($volume->volstatus);
		if ($volstatus == 'full' || $volstatus == 'used') {
			$whenexpire = strtotime($volume->lastwritten) + $volume->volretention;
			$whenexpire = date( 'Y-m-d H:i:s', $whenexpire);
		} else{
			$whenexpire = 'no date';
		}
		$volume->whenexpire = $whenexpire;
	}

	/**
	 * Get volumes for specific jobid and fileid.
	 *
	 * @param integer $jobid job identifier
	 * @param integer $fileid file identifier
	 * @return array volumes list
	 */
	public function getVolumesForJob($jobid, $fileid) {
		$connection = VolumeRecord::finder()->getDbConnection();
		$connection->setActive(true);
		$sql = sprintf('SELECT first_index, last_index, VolumeName AS volname, InChanger AS inchanger FROM (
		 SELECT VolumeName, InChanger, MIN(FirstIndex) as first_index, MAX(LastIndex) as last_index
		 FROM JobMedia JOIN Media ON (JobMedia.MediaId = Media.MediaId)
		 WHERE JobId = %d GROUP BY VolumeName, InChanger
		) AS gv, File
		 WHERE FileIndex >= first_index
		 AND FileIndex <= last_index
		 AND File.FileId = %d', $jobid, $fileid);
		$pdo = $connection->getPdoInstance();
		$result = $pdo->query($sql);
		$ret = $result->fetchAll();
		$pdo = null;
		$volumes = array();
		if (is_array($ret)) {
			for ($i = 0; $i < count($ret); $i++) {
				$volumes[] = array(
					'first_index' => $ret[$i]['first_index'],
					'last_index' => $ret[$i]['last_index'],
					'volume' => $ret[$i]['volname'],
					'inchanger' => $ret[$i]['inchanger']
				);
			}
		}
		return $volumes;
	}
}
?>
