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

Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('Application.Web.Class.BaculumWebPage'); 

class ClientView extends BaculumWebPage {

	const CLIENTID = 'ClientId';
	const CLIENT_NAME = 'ClientName';

	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsCallBack || $this->IsPostBack) {
			return;
		}
		$clientid = 0;
		if ($this->Request->contains('clientid')) {
			$clientid = $this->Request['clientid'];
		} elseif ($this->Request->contains('client')) {
			$result = $this->getModule('api')->get(array('clients'));
			if ($result->error === 0) {
				for ($i = 0; $i < count($result->output); $i++) {
					if ($this->Request['client'] === $result->output[$i]->name) {
						$clientid = $result->output[$i]->clientid;
						break;
					}
				}
			}
		}
		$this->setClientId($clientid);
		$clientshow = $this->getModule('api')->get(
			array('clients', $clientid, 'show'),
			null,
			true
		);
		if ($clientshow->error === 0) {
			$this->ShowLog->Text = implode(PHP_EOL, $clientshow->output);
		}
		$client = $this->getModule('api')->get(
			array('clients', $clientid)
		);
		if ($client->error === 0) {
			$this->setClientName($client->output->name);
		}
	}

	public function onPreRender($param) {
		parent::onPreRender($param);
		if ($this->IsCallBack || $this->IsPostBack) {
			return;
		}
		if (!empty($_SESSION['dir'])) {
			$this->ClientConfig->setComponentName($_SESSION['dir']);
			$this->ClientConfig->setResourceName($this->getClientName());
			$this->ClientConfig->setLoadValues(true);
			$this->ClientConfig->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}

	/**
	 * Set client clientid.
	 *
	 * @return none;
	 */
	public function setClientId($clientid) {
		$clientid = intval($clientid);
		$this->BWLimit->setClientId($clientid);
		$this->setViewState(self::CLIENTID, $clientid, 0);
	}

	/**
	 * Get client clientid.
	 *
	 * @return integer clientid
	 */
	public function getClientId() {
		return $this->getViewState(self::CLIENTID, 0);
	}

	/**
	 * Set client name.
	 *
	 * @return none;
	 */
	public function setClientName($client_name) {
		$this->BWLimit->setClientName($client_name);
		$this->setViewState(self::CLIENT_NAME, $client_name);
	}

	/**
	 * Get client name.
	 *
	 * @return string client name
	 */
	public function getClientName() {
		return $this->getViewState(self::CLIENT_NAME);
	}

	public function status($sender, $param) {
		$raw_status = $this->getModule('api')->get(
			array('clients', $this->getClientId(), 'status')
		)->output;
		$this->ClientLog->Text = implode(PHP_EOL, $raw_status);

		$query_str = '?name=' . rawurlencode($this->getClientName()) . '&type=header';
		$graph_status = $this->getModule('api')->get(
			array('status', 'client', $query_str),
		);
		$client_status = array(
			'header' => array(),
			'running' => array(),
			'version' => $this->getClientVersion($raw_status)
		);
		if ($graph_status->error === 0) {
			$client_status['header'] = $graph_status->output;
			if (!$this->BWLimit->BandwidthLimit->getDirectiveValue()) {
				$this->BWLimit->setBandwidthLimit($client_status['header']->bwlimit);
				$this->getCallbackClient()->callClientFunction(
					'oClientBandwidthLimit.set_value',
					array($client_status['header']->bwlimit)
				);
			}
		}

		$query_str = '?name=' . rawurlencode($this->getClientName()) . '&type=running';
		$graph_status = $this->getModule('api')->get(
			array('status', 'client', $query_str),
		);
		if ($graph_status->error === 0) {
			$client_status['running'] = $graph_status->output;
		}

		$query_str = '?output=json';
		$show = $this->getModule('api')->get(
			array('clients', $this->getClientId(), 'show', $query_str),
		);
		if ($show->error === 0) {
			$client_status['show'] = $show->output;
		}

		$this->getCallbackClient()->callClientFunction('init_graphical_client_status', array($client_status));
	}

	private function getClientVersion($output) {
		$version = array('major' => 0, 'minor' => 0, 'release' => 0);
		for ($i = 0; $i < count($output); $i++) {
			if (preg_match('/^[\w\d\s:.\-]+Version:\s+(?P<major>\d+)\.(?P<minor>\d+)\.(?P<release>\d+)\s+\(/', $output[$i], $match) === 1) {
				$version['major'] = intval($match['major']);
				$version['minor'] = intval($match['minor']);
				$version['release'] = intval($match['release']);
				break;
			}
		}
		return $version;
	}
}
?>
