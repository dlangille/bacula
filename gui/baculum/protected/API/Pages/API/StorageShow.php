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
 * Show storage command endpoint.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category API
 * @package Baculum API
 */
class StorageShow extends ConsoleOutputPage {
	public function get() {
		$storageid = $this->Request->contains('id') ? intval($this->Request['id']) : 0;
		$out_format = $this->Request->contains('output') && $this->isOutputFormatValid($this->Request['output']) ? $this->Request['output'] : parent::OUTPUT_FORMAT_RAW;
		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			array('.storage'),
			null,
			true
		);
		if ($result->exitcode === 0) {
			$storage = $this->getModule('storage')->getStorageById($storageid);
			if (is_object($storage) && in_array($storage->name, $result->output)) {
				$out = (object)['output' => [], 'exitcode' => 0];
				if ($out_format === parent::OUTPUT_FORMAT_RAW) {
					$out = $this->getRawOutput(['storage' => $storage->name]);
				} elseif($out_format === parent::OUTPUT_FORMAT_JSON) {
					$out = $this->getJSONOutput(['storage' => $storage->name]);
				}
				$this->output = $out->output;
				$this->error = $out->exitcode;
			} else {
				$this->output = StorageError::MSG_ERROR_STORAGE_DOES_NOT_EXISTS;
				$this->error = StorageError::ERROR_STORAGE_DOES_NOT_EXISTS;
			}
		} else {
			$this->output = $result->output;
			$this->error = $result->exitcode;
		}
	}

	/**
	 * Get show storage output from console in raw format.
	 *
	 * @param array $params command  parameters
	 * @return StdClass object with output and exitcode
	 */
	protected function getRawOutput($params = []) {
		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			array('show', 'storage="' . $params['storage'] . '"'),
			null,
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
						$result->output[strtolower($key)] = $value;
					}
				}
			}
		}
		$result->exitcode = $output->exitcode;
		return $result;
	}
}

?>
