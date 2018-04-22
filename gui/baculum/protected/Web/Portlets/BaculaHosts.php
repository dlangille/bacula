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

Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('System.Web.UI.ActiveControls.TActiveRepeater');
Prado::using('Application.Web.Class.HostConfig');
Prado::using('Application.Web.Portlets.HostListTemplate');

class BaculaHosts extends HostListTemplate {

	const CHILD_CONTROL = 'BaculaConfigComponents';

	public $config;

	public function loadConfig($sender, $param) {
		if(!$_SESSION['admin']) {
			return;
		}
		$this->config = $this->getModule('host_config')->getConfig();
		$hosts = array_keys($this->config);
		$this->RepeaterHosts->DataSource = $hosts;
		$this->RepeaterHosts->dataBind();
	}

	public function createHostListElement($sender, $param) {
		$control = $this->getChildControl($param->Item, self::CHILD_CONTROL);
		if (is_object($control)) {
			$control->setHost($param->Item->Data);
		}
		$param->Item->RemoveHost->setCommandParameter($param->Item->Data);
	}

	public function getComponents($sender, $param) {
		$control = $this->getChildControl($sender->getParent(), self::CHILD_CONTROL);
		if (is_object($control)) {
			$control->raiseEvent('OnComponentListLoad', $this, null);
		}
	}

	public function bubbleEvent($sender, $param) {
		if ($param instanceof TCommandEventParameter) {
			$this->loadConfig(null, null);
			return true;
		} else {
			return false;
		}
	}

	public function removeHost($sender, $param) {
		if(!$_SESSION['admin']) {
			return;
		}
		$host = $param->getCommandParameter();
		if (!empty($host)) {
			$host_config = $this->getModule('host_config');
			$config = $host_config->getConfig();
			unset($config[$host]);
			$host_config->setConfig($config);
			$this->loadConfig(null, null);
			if ($host === HostConfig::MAIN_CATALOG_HOST) {
				$url = $this->Service->constructUrl('WebConfigWizard', array(), false);
				$this->getResponse()->redirect($url);
			}
		}
	}
}
?>
