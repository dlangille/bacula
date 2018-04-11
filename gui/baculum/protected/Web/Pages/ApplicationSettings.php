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

Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('Application.Web.Class.BaculumWebPage'); 

class ApplicationSettings extends BaculumWebPage {

	protected $admin = true;

	public $web_config;

	public function onInit($param) {
		parent::onInit($param);
		$this->web_config = $this->getModule('web_config')->getConfig();
		if(count($this->web_config) > 0) {
			$this->Debug->Checked = ($this->web_config['baculum']['debug'] == 1);
		}
	}


	public function save() {
		$this->enableDebug($this->Debug->Checked);
	}

	public function enableDebug($enable) {
		$result = false;
		if(count($this->web_config) > 0) {
			$this->web_config['baculum']['debug'] = ($enable === true) ? 1 : 0;
			$result = $this->getModule('web_config')->setConfig($this->web_config);
		}
		return $result;
	}
}
?>
