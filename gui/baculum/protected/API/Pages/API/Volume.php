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
 
class Volume extends BaculumAPIServer {
	public function get() {
		$mediaid = intval($this->Request['id']);
		$volume = $this->getModule('volume')->getVolumeById($mediaid);
		if(!is_null($volume)) {
			$this->output = $volume;
			$this->error = VolumeError::ERROR_NO_ERRORS;
		} else {
			$this->output = VolumeError::MSG_ERROR_VOLUME_DOES_NOT_EXISTS;
			$this->error = VolumeError::ERROR_VOLUME_DOES_NOT_EXISTS;
		}
	}
	
	public function set($id, $params) {
		$volume = $this->getModule('volume')->getVolumeById($id);
		if (is_object($volume)) {
			$cmd = array('update', 'volume="' . $volume->volumename . '"');
			if(property_exists($params, 'volstatus')) {
				$cmd[] = 'volstatus="' . $params->volstatus . '"';
			}
			if(property_exists($params, 'poolid')) {
				$pool = $this->getModule('pool')->getPoolById($params->poolid);
				if (is_object($pool)) {
					$cmd[] = 'pool="' . $pool->name . '"';
				}
			}
			if(property_exists($params, 'volretention')) {
				$cmd[] = 'volretention="' . $params->volretention . '"';
			}
			if(property_exists($params, 'voluseduration')) {
				$cmd[] = 'voluseduration="' . $params->voluseduration . '"';
			}
			if(property_exists($params, 'maxvoljobs')) {
				$cmd[] = 'maxvoljobs="' . $params->maxvoljobs . '"';
			}
			if(property_exists($params, 'maxvolfiles')) {
				$cmd[] = 'maxvolfiles="' . $params->maxvolfiles . '"';
			}
			if(property_exists($params, 'maxvolbytes')) {
				$cmd[] = 'maxvolbytes="' . $params->maxvolbytes . '"';
			}
			if(property_exists($params, 'slot')) {
				$cmd[] = 'slot="' . $params->slot . '"';
			}
			if(property_exists($params, 'recycle')) {
				$cmd[] = 'recycle="' . ($params->recycle ? 'yes' : 'no') . '"';
			}
			if(property_exists($params, 'enabled')) {
				$cmd[] = 'enabled="' . ($params->enabled ? 'yes' : 'no') . '"';
			}
			if(property_exists($params, 'inchanger')) {
				$cmd[] = 'inchanger="' . ($params->inchanger ? 'yes' : 'no') . '"';
			}
			$result = $this->getModule('bconsole')->bconsoleCommand($this->director, $cmd);
			$this->output = $result->output;
			$this->error = $result->exitcode;
		} else {
			$this->output = VolumeError::MSG_ERROR_VOLUME_DOES_NOT_EXISTS;
			$this->error = VolumeError::ERROR_VOLUME_DOES_NOT_EXISTS;
		}
	}
}

?>
