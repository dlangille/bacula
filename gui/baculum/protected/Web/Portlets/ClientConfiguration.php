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

Prado::using('System.Web.UI.ActiveControls.TActiveCustomValidator');
Prado::using('System.Web.UI.ActiveControls.TActiveDataGrid');
Prado::using('Application.Web.Portlets.Portlets');

class ClientConfiguration extends Portlets {

	public function onInit($param) {
		parent::onInit($param);
		$this->Status->setActionClass($this);
	}

	public function configure($clientId) {
		$clientdata = $this->Application->getModule('api')->get(array('clients', 'show', $clientId))->output;
		$this->ShowClient->Text = implode(PHP_EOL, $clientdata);
		$client = $this->Application->getModule('api')->get(array('clients', $clientId))->output;
		$this->ClientName->Text = $client->name;
		$this->ClientIdentifier->Text = $client->clientid;
		$this->ClientDescription->Text = $client->uname;

		$jobs_for_client = $this->Application->getModule('api')->get(array('clients', 'jobs', $client->clientid))->output;
		$this->JobsForClient->DataSource = $this->Application->getModule('misc')->objectToArray($jobs_for_client);
		$this->JobsForClient->dataBind();
	}

	public function status($sender, $param) {
		$status = $this->Application->getModule('api')->get(array('clients', 'status', $this->ClientIdentifier->Text))->output;
		$this->ShowClient->Text = implode(PHP_EOL, $status);
	}

	public function openJob($sender, $param) {
		$jobid = $param->CallbackParameter;
		$this->getPage()->JobConfiguration->configure($jobid);
	}
}
?>
