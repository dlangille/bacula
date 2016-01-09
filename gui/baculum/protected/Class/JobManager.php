<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2015 Marcin Haba
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
 
class JobManager extends TModule {

	public function getJobs($limit, $allowedJobs = array()) {
		$criteria = new TActiveRecordCriteria;
		$order = 'JobId';
		$cfg = $this->Application->getModule('configuration');
		$appCfg = $cfg->getApplicationConfig();
		if($cfg->isPostgreSQLType($appCfg['db']['type'])) {
		    $order = strtolower($order);
		}
		$criteria->OrdersBy[$order] = 'desc';
		if(is_int($limit) && $limit > 0) {
			$criteria->Limit = $limit;
		}

		if (count($allowedJobs) > 0) {
			$where = array();
			$names = array();
			for ($i = 0; $i < count($allowedJobs); $i++) {
				$where[] = "name = :name$i";
				$names[":name$i"] = $allowedJobs[$i];
			}
			$criteria->Condition = implode(' OR ', $where);
			foreach($names as $name => $jobname) {
				$criteria->Parameters[$name] = $jobname;
			}
		}
		return JobRecord::finder()->findAll($criteria);
	}

	public function getJobById($id) {
		return JobRecord::finder()->findByjobid($id);
	}

	public function getJobByName($name) {
		return JobRecord::finder()->findByname($name);
	}

	public function deleteJobById($id) {
		return JobRecord::finder()->deleteByjobid($id);
	}

	public function getRecentJobids($jobname, $clientid) {
		$sql = "name='$jobname' AND clientid='$clientid' AND jobstatus IN ('T', 'W') AND level IN ('F', 'I', 'D')";
		$finder = JobRecord::finder();
		$criteria = new TActiveRecordCriteria;
		$order = 'endtime';
		$cfg = $this->Application->getModule('configuration');
		$appCfg = $cfg->getApplicationConfig();
		if($cfg->isPostgreSQLType($appCfg['db']['type'])) {
		    $order = strtolower($order);
		}
		$criteria->OrdersBy[$order] = 'desc';
		$criteria->Condition = $sql;
		$jobs = $finder->findAll($sql);

		$jobids = array();
		$waitForFull = false;
		if(!is_null($jobs)) {
			foreach($jobs as $job) {
				if($job->level == 'F') {
					$jobids[] = $job->jobid;
					break;
				} elseif($job->level == 'D' && $waitForFull === false) {
					$jobids[] = $job->jobid;
					$waitForFull = true;
				} elseif($job->level == 'I' && $waitForFull === false) {
					$jobids[] = $job->jobid;
				}
			}
		}
		return $jobids;
	}

	public function getJobTotals($allowedJobs = array()) {
		$jobtotals = array('bytes' => 0, 'files' => 0);
		$connection = JobRecord::finder()->getDbConnection();
		$connection->setActive(true);

		$where = '';
		if (count($allowedJobs) > 0) {
			$where = " WHERE name='" . implode("' OR name='", $allowedJobs) . "'";
		}

		$sql = "SELECT sum(JobFiles) AS files, sum(JobBytes) AS bytes FROM Job $where";
		$pdo = $connection->getPdoInstance();
		$result = $pdo->query($sql);
		$ret = $result->fetch();
		$jobtotals['bytes'] = $ret['bytes'];
		$jobtotals['files'] = $ret['files'];
		$pdo = null;
		return $jobtotals;
	}
}
?>
