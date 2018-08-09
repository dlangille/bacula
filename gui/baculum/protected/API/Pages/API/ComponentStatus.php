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

/**
 * Component status module.
 */
class ComponentStatus extends BaculumAPIServer {

	public function get() {
		$status = array();
		$misc = $this->getModule('misc');
		$component = $this->Request->contains('component') ? $this->Request['component'] : '';
		$type = $this->Request->contains('type') && $misc->isValidName($this->Request['type']) ? $this->Request['type'] : '';

		switch($component) {
			case 'director': {
				$result = $this->getModule('bconsole')->bconsoleCommand(
					$this->director,
					array('status', 'director'),
					true
				);
				if ($result->exitcode === 0) {
					$output = $this->getModule('status_dir')->getStatus($result->output);
					if ($misc->isValidName($type) && key_exists($type, $output)) {
						$this->output = $output[$type];
					} else {
						$this->output = $output;
					}
				} else {
					$this->output = $result->output;
				}
				$this->error = $result->exitcode;
				break;
			}
			case 'storage': {
				$storage = 'storage';
				if (empty($type)) {
					$type = 'header'; // default list terminated jobs
				}
				if ($this->Request->contains('name') && $misc->isValidName($this->Request['name'])) {
					$storage .= '="' . $this->Request['name'] . '"';
				}
				$result = $this->getModule('bconsole')->bconsoleCommand(
					$this->director,
					array('.status', $storage, $type),
					true
				);
				if ($result->exitcode === 0) {
					$this->output = $this->getModule('status_sd')->getStatus($result->output);
				} else {
					$this->output = $result->output;
				}
				$this->error = $result->exitcode;
				break;
			}
		}
	}
}
?>
