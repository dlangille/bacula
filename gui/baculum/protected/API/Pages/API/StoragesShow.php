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
 
Prado::using('Application.API.Class.ConsoleOutputPage');

/**
 * Show storages command endpoint.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category API
 * @package Baculum API
 */
class StoragesShow extends ConsoleOutputPage {

	public function get() {
		$out_format = $this->Request->contains('output') && $this->isOutputFormatValid($this->Request['output']) ? $this->Request['output'] : parent::OUTPUT_FORMAT_RAW;
		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			array('.storage'),
			null,
			true
		);
		$storage = null;
		if ($result->exitcode === 0) {
			if ($this->Request->contains('name')) {
				if (in_array($this->Request['name'], $result->output)) {
					$storage = $this->Request['name'];
				} else {
					$this->output = StorageError::MSG_ERROR_STORAGE_DOES_NOT_EXISTS;
					$this->error = StorageError::ERROR_STORAGE_DOES_NOT_EXISTS;
					return;
				}
			}
		} else {
			$this->output = $result->output;
			$this->error = $result->exitcode;
			return;
		}

		$out = (object)['output' => [], 'exitcode' => 0];
		if ($out_format === parent::OUTPUT_FORMAT_RAW) {
			$out = $this->getRawOutput(['storage' => $storage]);
		} elseif($out_format === parent::OUTPUT_FORMAT_JSON) {
			$out = $this->getJSONOutput(['storage' => $storage]);
		}

		$this->output = $out->output;
		$this->error = $out->exitcode;
	}

	/**
	 * Get show storage output from console in raw format.
	 *
	 * @param array $params command  parameters
	 * @return StdClass object with output and exitcode
	 */
	protected function getRawOutput($params = []) {
		$cmd = array('show');
		if (is_string($params['storage'])) {
			$cmd[] = 'storage="' . $params['storage'] . '"';
		} else {
			$cmd[] = 'storages';
		}
		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			$cmd,
			true
		);
		return $result;
	}

	/**
	 * Get show storage output in JSON format.
	 *
	 * @param array $params command parameters
	 * @return StdClass object with output and exitcode
	 */
	protected function getJSONOutput($params = []) {
		$result = (object)['output' => [], 'exitcode' => 0];
		$output = $this->getRawOutput($params);
		if ($output->exitcode === 0) {
			$part = [];
			for ($i = 0; $i < count($output->output); $i++) {
				if (preg_match_all('/(?<=\s)\w+=.+?(?=\s+\w+=.+|$)/i', $output->output[$i], $matches) > 0) {
					for ($j = 0; $j < count($matches[0]); $j++) {
						list($key, $value) = explode('=', $matches[0][$j], 2);
						if (key_exists($key, $result->output)) {
							/*
							 * The most important options are in first lines.
							 * If keys are double skip the second ones
							 */
							continue;
						}
						if ($key == 'name' && count($part) > 0) {
							$result->output[] = $part;
							$part = [];
						}
						$part[strtolower($key)] = $value;
					}
				}
			}
			if (count($part) > 0) {
				$result->output[] = $part;
				$part = [];
			}
		}
		$result->exitcode = $output->exitcode;
		return $result;
	}
}
?>
