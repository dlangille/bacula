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

Prado::using('Application.API.Class.APIModule');
Prado::using('Application.API.Class.JobRecord');
Prado::using('Application.API.Class.Database');

class JobManager extends APIModule {

	public function getJobs($limit, $params = array()) {
		$criteria = new TActiveRecordCriteria;
		$order = 'JobId';
		$db_params = $this->getModule('api_config')->getConfig('db');
		if ($db_params['type'] === Database::PGSQL_TYPE) {
		    $order = strtolower($order);
		}
		$criteria->OrdersBy[$order] = 'desc';
		if(is_int($limit) && $limit > 0) {
			$criteria->Limit = $limit;
		}

		if (count($params) > 0) {
			$condition = array();
			foreach ($params as $key => $value) {
				$cond = array();
				$vals = array();
				if (is_array($value['vals'])) {
					for ($i = 0; $i < count($value['vals']); $i++) {
						$cond[] = "{$key} = :{$key}{$i}";
						$vals[":{$key}{$i}"] = $value['vals'][$i];
					}
				} else {
					$cond[] = "$key = :$key";
					$vals[":$key"] = $value['vals'];
				}
				$condition[] = implode(' ' . $value['operator'] . ' ', $cond);
				foreach ($vals as $pkey => $pval) {
					$criteria->Parameters[$pkey] = $pval;
				}
			}
			$criteria->Condition = '(' . implode(') AND (' , $condition) . ')';
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

	/**
	 * Find all compojobs required to do full restore.
	 *
	 * @param array $jobs jobid to start searching for jobs
	 * @return array compositional jobs regarding given jobid
	 */
	private function findCompositionalJobs(array $jobs) {
		$jobids = [];
		$wait_on_full = false;
		foreach($jobs as $job) {
			if($job->level == 'F') {
				$jobids[] = $job->jobid;
				break;
			} elseif($job->level == 'D' && $wait_on_full === false) {
				$jobids[] = $job->jobid;
				$wait_on_full = true;
			} elseif($job->level == 'I' && $wait_on_full === false) {
				$jobids[] = $job->jobid;
			}
		}
		return $jobids;
	}

	/**
	 * Get latest recent compositional jobids to do restore.
	 *
	 * @param string $jobname job name
	 * @param integer $clientid client identifier
	 * @param integer $filesetid fileset identifier
	 * @param boolean $inc_copy_job determine if include copy jobs to result
	 * @return array list of jobids required to do restore
	 */
	public function getRecentJobids($jobname, $clientid, $filesetid, $inc_copy_job = false) {
		$types = "('B')";
		if ($inc_copy_job) {
			$types = "('B', 'C')";
		}
		$sql = "name='$jobname' AND clientid='$clientid' AND filesetid='$filesetid' AND type IN $types AND jobstatus IN ('T', 'W') AND level IN ('F', 'I', 'D')";
		$finder = JobRecord::finder();
		$criteria = new TActiveRecordCriteria;
		$order1 = 'RealEndTime';
		$order2 = 'JobId';
		$db_params = $this->getModule('api_config')->getConfig('db');
		if ($db_params['type'] === Database::PGSQL_TYPE) {
		    $order1 = strtolower($order1);
		    $order2 = strtolower($order2);
		}
		$criteria->OrdersBy[$order1] = 'desc';
		$criteria->OrdersBy[$order2] = 'desc';
		$criteria->Condition = $sql;
		$jobs = $finder->findAll($criteria);

		$jobids = array();
		if(is_array($jobs)) {
			$jobids = $this->findCompositionalJobs($jobs);
		}
		return $jobids;
	}

	/**
	 * Get compositional jobids to do restore starting from given job (full/incremental/differential).
	 *
	 * @param integer $jobid job identifier of last job to do restore
	 * @return array list of jobids required to do restore
	 */
	public function getJobidsToRestore($jobid) {
		$jobids = [];
		$bjob = JobRecord::finder()->findBySql(
			"SELECT * FROM job WHERE jobid = '$jobid' AND jobstatus IN ('T', 'W') AND type IN ('B', 'C') AND level IN ('F', 'I', 'D')"
		);
		if (is_object($bjob)) {
			if ($bjob->level != 'F') {
				$sql = "clientid=:clientid AND filesetid=:filesetid AND type IN ('B', 'C')" .
					" AND jobstatus IN ('T', 'W') AND level IN ('F', 'I', 'D') " .
					" AND starttime <= :starttime and jobid <= :jobid";
				$finder = JobRecord::finder();
				$criteria = new TActiveRecordCriteria;
				$order1 = 'JobId';
				$db_params = $this->getModule('api_config')->getConfig('db');
				if ($db_params['type'] === Database::PGSQL_TYPE) {
					$order1 = strtolower($order1);
				}
				$criteria->OrdersBy[$order1] = 'desc';
				$criteria->Condition = $sql;
				$criteria->Parameters[':clientid'] = $bjob->clientid;
				$criteria->Parameters[':filesetid'] = $bjob->filesetid;
				$criteria->Parameters[':starttime'] = $bjob->endtime;
				$criteria->Parameters[':jobid'] = $bjob->jobid;
				$jobs = $finder->findAll($criteria);

				if(is_array($jobs)) {
					$jobids = $this->findCompositionalJobs($jobs);
				}
			} else {
				$jobids[] = $bjob->jobid;
			}
		}
		return $jobids;
	}

	public function getJobTotals($allowed_jobs = array()) {
		$jobtotals = array('bytes' => 0, 'files' => 0);
		$connection = JobRecord::finder()->getDbConnection();
		$connection->setActive(true);

		$where = '';
		if (count($allowed_jobs) > 0) {
			$where = " WHERE name='" . implode("' OR name='", $allowed_jobs) . "'";
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

	/**
	 * Get jobs stored on given volume.
	 *
	 * @param string $mediaid volume identifier
	 * @param array $allowed_jobs jobs allowed to show
	 * @return array jobs stored on volume
	 */
	public function getJobsOnVolume($mediaid, $allowed_jobs = array()) {
		$jobs_criteria = '';
		if (count($allowed_jobs) > 0) {
			$jobs_sql = implode("', '", $allowed_jobs);
			$jobs_criteria = " AND Job.Name IN ('" . $jobs_sql . "')";
		}
		$sql = "SELECT DISTINCT Job.* FROM Job, JobMedia WHERE JobMedia.MediaId='$mediaid' AND JobMedia.JobId=Job.JobId $jobs_criteria";
		return JobRecord::finder()->findAllBySql($sql);
	}

	/**
	 * Get jobs for given client.
	 *
	 * @param string $clientid client identifier
	 * @param array $allowed_jobs jobs allowed to show
	 * @return array jobs for specific client
	 */
	public function getJobsForClient($clientid, $allowed_jobs = array()) {
		$jobs_criteria = '';
		if (count($allowed_jobs) > 0) {
			$jobs_sql = implode("', '", $allowed_jobs);
			$jobs_criteria = " AND Job.Name IN ('" . $jobs_sql . "')";
		}
		$sql = "SELECT DISTINCT Job.* FROM Client, Job WHERE Client.ClientId='$clientid' AND Client.ClientId=Job.ClientId $jobs_criteria";
		return JobRecord::finder()->findAllBySql($sql);
	}
}
?>
