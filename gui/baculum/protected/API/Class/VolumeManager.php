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

Prado::using('Application.API.Class.APIModule');
Prado::using('Application.API.Class.VolumeRecord');
Prado::using('Application.API.Class.Database');

class VolumeManager extends APIModule {

	public function getVolumes($limit, $with_pools = false) {
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
		$this->setExtraVariables($volumes, $with_pools);
		return $volumes;
	}

	public function getVolumesByPoolId($poolid) {
		return VolumeRecord::finder()->findBypoolid($poolid);
	}

	public function getVolumeByName($volumeName) {
		return VolumeRecord::finder()->findByvolumename($volumeName);
	}

	public function getVolumeById($volumeId) {
		return VolumeRecord::finder()->findBymediaid($volumeId);
	}

	private function setExtraVariables(&$volumes, $with_pools) {
		$pools = $this->Application->getModule('pool')->getPools(false);
		foreach($volumes as $volume) {
			$volstatus = strtolower($volume->volstatus);
			$volume->whenexpire = ($volstatus == 'full' || $volstatus == 'used') ? date( 'Y-m-d H:i:s', (strtotime($volume->lastwritten) + $volume->volretention)) : 'no date';
			if ($with_pools === true) {
				foreach($pools as $pool) {
					if($volume->poolid == $pool->poolid) {
						$volume->pool = $pool;
					}
				}
			}
		}
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
		 WHERE JobId = %d GROUP BY (VolumeName, InChanger)
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
