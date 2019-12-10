<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2019 Kern Sibbald
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
Prado::using('Application.Web.Pages.Monitor');

/**
 * Application settings class.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class ApplicationSettings extends BaculumWebPage {

	protected $admin = true;

	public $web_config;

	public function onInit($param) {
		parent::onInit($param);
		$this->web_config = $this->getModule('web_config')->getConfig();
		$this->DecimalBytes->Checked = true;
		if(count($this->web_config) > 0) {
			$this->Debug->Checked = ($this->web_config['baculum']['debug'] == 1);
			$this->MaxJobs->Text = (key_exists('max_jobs', $this->web_config['baculum']) ? intval($this->web_config['baculum']['max_jobs']) : Monitor::DEFAULT_MAX_JOBS);
			if (key_exists('size_values_unit', $this->web_config['baculum'])) {
				$this->DecimalBytes->Checked = ($this->web_config['baculum']['size_values_unit'] === 'decimal');
				$this->BinaryBytes->Checked = ($this->web_config['baculum']['size_values_unit'] === 'binary');
			}
		}
	}


	public function save() {
		if (count($this->web_config) > 0) {
			$this->web_config['baculum']['debug'] = ($this->Debug->Checked === true) ? 1 : 0;
			$max_jobs = intval($this->MaxJobs->Text);
			$this->web_config['baculum']['max_jobs'] = $max_jobs;
			$this->web_config['baculum']['size_values_unit'] = $this->BinaryBytes->Checked ? 'binary' : 'decimal';
			$this->getModule('web_config')->setConfig($this->web_config);
		}
	}
}
?>
