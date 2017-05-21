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
 
class FileSets extends BaculumAPIServer {
	public function get() {
		$directors = $this->getModule('bconsole')->getDirectors();
		if($directors->exitcode === 0) {
			$filesets = array();
			$error = false;
			$error_obj = null;
			for($i = 0; $i < count($directors->output); $i++) {
				$filesetsshow = $this->getModule('bconsole')->bconsoleCommand($directors->output[$i], array('show', 'fileset'), $this->user);
				if ($filesetsshow->exitcode != 0) {
					$error_obj = $filesetsshow;
					$error = true;
					break;
				}
				$filesets[$directors->output[$i]] = array();
				
				for($j = 0; $j < count($filesetsshow->output); $j++) {
					if(preg_match('/^FileSet:\ name=(.+?(?=\s\S+\=.+)|.+$)/i', $filesetsshow->output[$j], $match) === 1) {
						$filesets[$directors->output[$i]][] = $match[1];
					}
				}
			}

			if ($error === true) {
				$this->output = $error_obj->output;
				$this->error = $error_obj->exitcode;
			} else {
				$this->output = $filesets;
				$this->error =  BconsoleError::ERROR_NO_ERRORS;
			}
		} else {
			$this->output = $directors->output;
			$this->error = $directors->exitcode;
		}
	}
}
?>
