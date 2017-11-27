<?php

/*
 * Small sorting callback function to sort files and directories by name.
 * Function keeps '.' and '..' names always in the beginning of array.
 * Used to sort files and directories from Bvfs.
 */
function sortFilesListByName($a, $b) {
	$firstLeft = substr($a['name'], 0, 1);
	$firstRight = substr($b['name'], 0, 1);
	if ($firstLeft == '.' && $firstRight != '.') {
		return -1;
	} else if ($firstRight == '.' && $firstLeft != '.') {
		return 1;
	}
	return strcasecmp($a['name'], $b['name']);
}

class Miscellaneous extends TModule {

	const LICENCE_FILE = 'LICENSE';

	const RPATH_PATTERN = '/^b2\d+$/';

	public $job_types = array(
		'B' => 'Backup',
		'M' => 'Migrated',
		'V' => 'Verify',
		'R' => 'Restore',
		'I' => 'Internal',
		'D' => 'Admin',
		'A' => 'Archive',
		'C' => 'Copy',
		'c' => 'Copy Job',
		'g' => 'Migration'
	);

	private $jobLevels = array(
		'F' => 'Full',
		'I' => 'Incremental',
		'D' => 'Differential',
		'B' => 'Base',
		'f' => 'VirtualFull',
		'V' => 'InitCatalog',
		'C' => 'Catalog',
		'O' => 'VolumeToCatalog',
		'd' => 'DiskToCatalog'
	);

	public $jobStates =  array(
		'C' => array('value' => 'Created', 'description' =>'Created but not yet running'),
		'R' => array('value' => 'Running', 'description' => 'Running'),
		'B' => array('value' => 'Blocked', 'description' => 'Blocked'),
		'T' => array('value' => 'Terminated', 'description' =>'Terminated normally'),
		'W' => array('value' => 'Terminated', 'description' =>'Terminated normally with warnings'),
		'E' => array('value' => 'Error', 'description' =>'Terminated in Error'),
		'e' => array('value' => 'Non-fatal error', 'description' =>'Non-fatal error'),
		'f' => array('value' => 'Fatal error', 'description' =>'Fatal error'),
		'D' => array('value' => 'Verify Diff.', 'description' =>'Verify Differences'),
		'A' => array('value' => 'Canceled', 'description' =>'Canceled by the user'),
		'I' => array('value' => 'Incomplete', 'description' =>'Incomplete Job'),
		'F' => array('value' => 'Waiting on FD', 'description' =>'Waiting on the File daemon'),
		'S' => array('value' => 'Waiting on SD', 'description' =>'Waiting on the Storage daemon'),
		'm' => array('value' => 'Waiting for new vol.', 'description' =>'Waiting for a new Volume to be mounted'),
		'M' => array('value' => 'Waiting for mount', 'description' =>'Waiting for a Mount'),
		's' => array('value' => 'Waiting for storage', 'description' =>'Waiting for Storage resource'),
		'j' => array('value' => 'Waiting for job', 'description' =>'Waiting for Job resource'),
		'c' => array('value' => 'Waiting for client', 'description' =>'Waiting for Client resource'),
		'd' => array('value' => 'Waiting for Max. jobs', 'description' =>'Wating for Maximum jobs'),
		't' => array('value' => 'Waiting for start', 'description' =>'Waiting for Start Time'),
		'p' => array('value' => 'Waiting for higher priority', 'description' =>'Waiting for higher priority job to finish'),
		'i' => array('value' => 'Batch insert', 'description' =>'Doing batch insert file records'),
		'a' => array('value' => 'Despooling attributes', 'description' =>'SD despooling attributes'),
		'l' => array('value' => 'Data despooling', 'description' =>'Doing data despooling'),
		'L' => array('value' => 'Commiting data', 'description' =>'Committing data (last despool)')
	);

	private $jobStatesOK = array('T', 'D');
	private $jobStatesWarning = array('W');
	private $jobStatesError = array('E', 'e', 'f', 'I');
	private $jobStatesCancel = array('A');
	private $jobStatesRunning = array('C', 'R', 'B', 'F', 'S', 'm', 'M', 's', 'j', 'c', 'd','t', 'p', 'i', 'a', 'l', 'L');

	private $runningJobStates = array('C', 'R');

	private $components = array(
		'dir' => array('full_name' => 'Director', 'main_resource' => 'Director'),
		'sd' => array('full_name' => 'Storage Daemon', 'main_resource' => 'Storage'),
		'fd' => array('full_name' => 'File Daemon', 'main_resource' => 'FileDaemon'),
		'bcons' => array('full_name' => 'Console', 'main_resource' => 'Director')
	);

	private $replace_opts = array(
		'always',
		'ifnewer',
		'ifolder',
		'never'
	);


	/**
	 * Getting the licence from file.
	 * 
	 * @access public
	 * @return string licence text
	 */
	public function getLicence() {
		return nl2br(htmlspecialchars(file_get_contents(self::LICENCE_FILE)));
	}

	public function getJobLevels() {
		return $this->jobLevels;
	}

	public function getJobState($jobStateLetter = null) {
		$state;
		if(is_null($jobStateLetter)) {
			$state = $this->jobStates;
		} else {
			$state = array_key_exists($jobStateLetter, $this->jobStates) ? $this->jobStates[$jobStateLetter] : null;
		}
		return $state;
	}

	public function getRunningJobStates() {
		return $this->runningJobStates;
	}

	public function getComponents() {
		$components = array_keys($this->components);
	}

	public function getMainComponentResource($type) {
		$resource = null;
		if (array_key_exists($type, $this->components)) {
			$resource = $this->components[$type]['main_resource'];
		}
		return $resource;
	}

	public function getComponentFullName($type) {
		$name = '';
		if (array_key_exists($type, $this->components)) {
			$name = $this->components[$type]['full_name'];
		}
		return $name;
	}

	public function getJobStatesByType($type) {
		$statesByType = array();
		$states = array();
		switch($type) {
			case 'ok':
				$states = $this->jobStatesOK;
				break;
			case 'warning':
				$states = $this->jobStatesWarning;
				break;
			case 'error':
				$states = $this->jobStatesError;
				break;
			case 'cancel':
				$states = $this->jobStatesCancel;
				break;
			case 'running':
				$states = $this->jobStatesRunning;
				break;
		}

		for ($i = 0; $i < count($states); $i++) {
			$statesByType[$states[$i]] = $this->getJobState($states[$i]);
		}

		return $statesByType;
	}

	/*
	 * @TODO: Move it to separate validation module.
	 */
	public function isValidJobLevel($jobLevel) {
		return array_key_exists($jobLevel, $this->getJobLevels());
	}

	public function isValidName($name) {
		return (preg_match('/^[\w:\.\-\s]{1,127}$/', $name) === 1);
	}

	public function isValidPath($path) {
		return (preg_match('/^[\p{L}\p{N}\p{Z}\[\]\(\)\-\+\/\\\:\.#~_,{}!]{1,1000}$/', $path) === 1);
	}

	public function isValidReplace($replace) {
		return in_array($replace, $this->replace_opts);
	}

	public function isValidIdsList($list) {
		return (preg_match('/^[\d,]+$/', $list) === 1);
	}

	public function isValidBvfsPath($path) {
		return (preg_match('/^b2\d+$/', $path) === 1);
	}

	/**
	 * Writing INI-style configuration file.
	 * 
	 * Functions has been got from StackOverflow.com service (http://stackoverflow.com/questions/4082626/save-ini-file-with-comments).
	 * 
	 * @access public
	 * @param string $file file localization
	 * @param array $options structure of config file params
	 * @return mixed if success then returns the number of bytes that were written to the file as the integer type, if failure then returns false
	 */
	public function writeINIFile($file, array $options){
		$tmp = '';
		foreach($options as $section => $values){
			$tmp .= "[$section]\n";
				foreach($values as $key => $val){
					if(is_array($val)){
						foreach($val as $k => $v) {
							$v = $this->escapeINIVal($v);
							$tmp .= "{$key}[$k] = \"$v\"\n";
						}
					} else {
						$val = $this->escapeINIVal($val);
						$tmp .= "$key = \"$val\"\n";
					}
				}
			$tmp .= "\n";
		}
		$old_umask = umask(0);
		umask(0077);
		$result = file_put_contents($file, $tmp);
		umask($old_umask);
		return $result;
	}

	/**
	 * Escape text written to INI-style file.
	 *
	 * @access private
	 * @param string $value text to escape
	 * @return string escaped text
	 */
	private function escapeINIVal($value) {
		$esc_value = str_replace('"', '\"', $value);
		return $esc_value;
	}

	/**
	 * Parse INI-style configuration file.
	 * 
	 * @access public
	 * @param string $file file localization
	 * @return array data of configuration file
	 */
	public static function parseINIFile($file) {
		$content = array();
		if (file_exists($file)) {
			$content = parse_ini_file($file, true);
			if (!is_array($content)) {
				$content = array();
			}
		}
		return $content;
	}


	/**
	 * This method is copied from http://stackoverflow.com/questions/4345554/convert-php-object-to-associative-array
	 */
	public function objectToArray($data) {
		if (is_array($data) || is_object($data)) {
			$result = array();
			foreach ($data as $key => $value) {
				$result[$key] = $this->objectToArray($value);
			}
			return $result;
		}
		return $data;
	}

	public function decode_bacula_lstat($lstat) {
		$base64 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
		$lstat = trim($lstat);
		$lstat_fields = explode(' ', $lstat);
		$lstat_len = count($lstat_fields);
		if ($lstat_len < 16) {
			// not known or empty lstat value
			return;
		} elseif ($lstat_len > 16) {
			// cut off unknown fields
			array_splice($lstat_fields, 16);
		}

		list(
			$dev,
			$inode,
			$mode,
			$nlink,
			$uid,
			$gid,
			$rdev,
			$size,
			$blocksize,
			$blocks,
			$atime,
			$mtime,
			$ctime,
			$linkfi,
			$flags,
			$data
		) = $lstat_fields;
		$encoded_values = array(
			'dev' => $dev,
			'inode' => $inode,
			'mode' => $mode,
			'nlink' => $nlink,
			'uid' => $uid,
			'gid' => $gid,
			'rdev' => $rdev,
			'size' => $size,
			'blocksize' => $blocksize,
			'blocks' => $blocks,
			'atime' => $atime,
			'mtime' => $mtime,
			'ctime' => $ctime,
			'linkfi' => $linkfi,
			'flags' => $flags,
			'data' => $data
		);

		$ret = array();
		foreach($encoded_values as $key => $val) {
			$result = 0;
			$is_minus = false;
			$start = 0;

			if(substr($val, 0, 1) === '-') {
				$is_minus = true;
				$start++;
			}

			for($i = $start; $i < strlen($val); $i++) {
				$result = bcmul($result, bcpow(2,6));
				$result +=  strpos($base64, substr($val, $i , 1));
			}
			$ret[$key] = ($is_minus === true) ? -$result : $result;
		}
		return $ret;
	}

	public function parseBvfsList($list) {
		$elements = array();
		for($i = 0; $i < count($list); $i++) {
			if(preg_match('/^(?P<pathid>\d+)\t(?P<filenameid>\d+)\t(?P<fileid>\d+)\t(?P<jobid>\d+)\t(?P<lstat>[a-zA-z0-9\+\/\ ]+)\t(?P<name>.*)\/$/', $list[$i], $match) == 1 || preg_match('/^(?P<pathid>\d+)\t(?P<filenameid>\d+)\t(?P<fileid>\d+)\t(?P<jobid>\d+)\t(?P<lstat>[a-zA-z0-9\+\/\ ]+)\t(?P<name>\.{2})$/', $list[$i], $match) == 1) {
				if($match['name'] == '.') {
					continue;
				} elseif($match['name'] != '..') {
					$match['name'] .= '/';
				}
				$elements[] = array(
					'pathid' => $match['pathid'],
					'filenameid' => $match['filenameid'],
					'fileid' => $match['fileid'],
					'jobid' => $match['jobid'],
					'lstat' => $this->decode_bacula_lstat($match['lstat']),
					'name' => $match['name'],
					'type' => 'dir'
				);
			} elseif(preg_match('/^(?P<pathid>\d+)\t(?P<filenameid>\d+)\t(?P<fileid>\d+)\t(?P<jobid>\d+)\t(?P<lstat>[a-zA-z0-9\+\/\ ]+)\t(?P<name>[^\/]+)$/', $list[$i], $match) == 1) {
				if($match['name'] == '.') {
					continue;
				}
				$elements[] = array(
					'pathid' => $match['pathid'],
					'filenameid' => $match['filenameid'],
					'fileid' => $match['fileid'],
					'jobid' => $match['jobid'],
					'lstat' => $this->decode_bacula_lstat($match['lstat']),
					'name' => $match['name'],
					'type' => 'file'
				);
			}
		}
		usort($elements, 'sortFilesListByName');
		return $elements;
	}

	public function parseFileVersions($filename, $list) {
		$elements = array();
		for($i = 0; $i < count($list); $i++) {
			if(preg_match('/^(?P<pathid>\d+)\t(?P<filenameid>\d+)\t(?P<fileid>\d+)\t(?P<jobid>\d+)\t(?P<lstat>[a-zA-Z0-9\+\/\ ]+)\t(?P<md5>.+)\t(?P<volname>.+)\t(?P<inchanger>\d+)$/', $list[$i], $match) == 1) {
				$elements[$match['fileid']] = array(
					'name' => $filename,
					'pathid' => $match['pathid'],
					'filenameid' => $match['filenameid'],
					'fileid' => $match['fileid'],
					'jobid' => $match['jobid'],
					'lstat' => $this->decode_bacula_lstat($match['lstat']),
					'md5' => $match['md5'],
					'volname' => $match['volname'],
					'inchanger' => $match['inchanger'],
					'type' => 'file'
				);
			}
		}
		return $elements;
	}

	public function findJobIdStartedJob($output) {
		$jobid = null;
		$output = array_reverse($output); // jobid is ussually at the end of output
		for ($i = 0; $i < count($output); $i++) {
			if (preg_match('/^Job queued\.\sJobId=(?P<jobid>\d+)$/', $output[$i], $match) === 1) {
				$jobid = $match['jobid'];
				break;
			}
		}
		return $jobid;
	}

	/**
	 * Get (pseudo)random string.
	 *
	 * Useful for log out user from HTTP Basic auth by providing random password.
	 *
	 * @access public
	 * @return string random 62 characters string from range [a-zA-Z0-9]
	 */
	public function getRandomString($length = null) {
		$characters = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		$rand_string = str_shuffle($characters);
		if (is_int($length) && $length <= 62) {
			$rand_string = substr($rand_string, 0, $length);
		}
		return $rand_string;
	}

	/**
	 * Get encrypted password to use in HTTP Basic auth.
	 *
	 * @access public
	 * @param string $password plain text password
	 * @return string encrypted password
	 */
	public function getCryptedPassword($password) {
		$enc_pwd = crypt($password, base64_encode($password));
		return $enc_pwd;
	}
}
?>
