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

/**
 * List files from 'list files jobid=xx' bconsole command.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category API
 * @package Baculum API
 */
class JobListFiles extends BaculumAPIServer {

	public function get() {
		$misc = $this->getModule('misc');
		$jobid = $this->Request->contains('id') ? intval($this->Request['id']) : 0;
		$type = $this->Request->contains('type') && $misc->isValidListFilesType($this->Request['type']) ? $this->Request['type'] : null;
		$offset = $this->Request->contains('offset') ? intval($this->Request['offset']) : 0;
		$limit = $this->Request->contains('limit') ? intval($this->Request['limit']) : 0;
		$search = $this->Request->contains('search') && $misc->isValidPath($this->Request['search']) ? $this->Request['search'] : null;

		$result = $this->getModule('bconsole')->bconsoleCommand(
			$this->director,
			array('.jobs')
		);
		if ($result->exitcode === 0) {
			array_shift($result->output);
			$job = $this->getModule('job')->getJobById($jobid);
			if (is_object($job) && in_array($job->name, $result->output)) {
				$cmd = array('list', 'files');
				if (is_string($type)) {
					/**
					 * NOTE: type param has to be used BEFORE jobid=xx, otherwise it doesn't work.
					 * This behavior is also described in Bacula source code (src/dird/ua_output.c).
					 */
					$cmd[] = 'type="' . $type . '"';
				}
				$cmd[] = 'jobid="' . $jobid . '"';
				$result = $this->getModule('bconsole')->bconsoleCommand(
					$this->director,
					$cmd
				);
				if ($result->exitcode === 0) {
					$file_list = $this->getModule('list')->parseListFilesOutput($result->output);
					if (is_string($search)) {
						// Find items
						$file_list = $this->getModule('list')->findFileListItems($file_list, $search);
					}
					$total_items = count($file_list);
					if ($offset > 0) {
						if ($limit > 0) {
							$file_list = array_slice($file_list, $offset, $limit);
						} else {
							$file_list = array_slice($file_list, $offset);
						}
					} elseif ($limit > 0) {
						$file_list = array_slice($file_list, 0, $limit);
					}
					$this->output = array('items' => $file_list, 'total' => $total_items);
					$this->error = GenericError::ERROR_NO_ERRORS;
				} else {
					$this->output = $result->output;
					$this->error = $result->exitcode;
				}
			} else {
				$this->output = JobError::MSG_ERROR_JOB_DOES_NOT_EXISTS;
				$this->error = JobError::ERROR_JOB_DOES_NOT_EXISTS;
			}
		} else {
			$this->output = $result->output;
			$this->error = $result->exitcode;
		}
	}
}
?>
