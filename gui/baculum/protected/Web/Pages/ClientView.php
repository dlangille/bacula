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
		}
		$this->setClientId($clientid);
		$clientshow = $this->getModule('api')->get(
			array('clients', 'show', $clientid),
			null,
			true
		);
		if ($clientshow->error === 0) {
			$this->ClientLog->Text = implode(PHP_EOL, $clientshow->output);
		}
		$client = $this->getModule('api')->get(
			array('clients', $clientid)
		);
		if ($client->error === 0) {
			$this->setClientName($client->output->name);
			if (!empty($_SESSION['dir'])) {
				$this->ClientConfig->setComponentName($_SESSION['dir']);
				$this->ClientConfig->setResourceName($this->getClientName());
				$this->ClientConfig->setLoadValues(true);
				$this->ClientConfig->raiseEvent('OnDirectiveListLoad', $this, null);
			}
		}
	}

	/**
	 * Set client clientid.
	 *
	 * @return none;
	 */
	public function setClientId($clientid) {
		$clientid = intval($clientid);
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
		$status = $this->getModule('api')->get(
			array('clients', 'status', $this->getClientId())
		)->output;
		$this->ClientLog->Text = implode(PHP_EOL, $status);
	}
}
?>
