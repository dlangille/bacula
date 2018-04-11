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

Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('Application.Web.Class.BaculumWebPage'); 

class NewResource extends BaculumWebPage {

	const COMPONENT_TYPE = 'ComponentType';
	const COMPONENT_NAME = 'ComponentName';
	const RESOURCE_TYPE = 'ResourceType';

	public function onInit($param) {
		if ($this->IsCallBack || $this->IsPostBack) {
			return;
		}
		$component_type = null;
		$component_name = null;
		$resource_type = null;
		if ($this->Request->contains('component_type')) {
			$component_type = $this->Request['component_type'];
		}
		if ($this->Request->contains('component_name')) {
			$component_name = $this->Request['component_name'];
		}
		if ($this->Request->contains('resource_type')) {
			$resource_type = $this->Request['resource_type'];
		}
		if ($component_type && $component_name && $resource_type) {
			$this->setComponentType($component_type);
			$this->setComponentName($component_name);
			$this->setResourceType($resource_type);
			if (!$_SESSION['admin']) {
				// Non-admin can configure only host assigned to him
				$this->NewResource->setHost($_SESSION['api_host']);
			}
			$this->NewResource->setComponentType($component_type);
			$this->NewResource->setComponentName($component_name);
			$this->NewResource->setResourceType($resource_type);
			$this->NewResource->setLoadValues(false);
			$this->NewResource->raiseEvent('OnDirectiveListLoad', $this, null);
			$this->setHosts();
		}
	}

	public function setHosts() {
		$config = $this->getModule('host_config')->getConfig();
		$hosts = array('' => Prado::localize('Please select host'));
		foreach ($config as $host => $vals) {
			if (!$_SESSION['admin'] && $host !== $_SESSION['api_host']) {
				continue;
			}
			$item = "Host: $host, Address: {$vals['address']}, Port: {$vals['port']}";
			$hosts[$host] = $item;
		}
		$this->Host->DataSource = $hosts;
		$this->Host->dataBind();
	}

	public function setComponents($sender, $param) {
		$components = array('' => Prado::localize('Please select component'));
		$this->NewResourceLog->Display = 'None';
		if ($this->Host->SelectedIndex > 0) {
			$config = $this->getModule('api')->get(
				array('config'),
				$this->Host->SelectedValue,
				false
			);
			if ($config->error === 0) {
				for ($i = 0; $i < count($config->output); $i++) {
					$component = (array)$config->output[$i];
					if (key_exists('component_type', $component) && key_exists('component_name', $component)) {
						$label = $this->getModule('misc')->getComponentFullName($component['component_type']);
						$label .= ' - ' . $component['component_name'];
						$components[$component['component_type'] . ';' . $component['component_name']] = $label;

					}
				}
			} else {
				$this->NewResourceLog->Text = var_export($config, true);
				$this->NewResourceLog->Display = 'Dynamic';
			}
		} else {
			$this->Resource->DataSource = array();
			$this->Resource->dataBind();
		}
		$this->Component->DataSource = $components;
		$this->Component->dataBind();
	}

	public function setResource() {
		$resources = array();
		if ($this->Component->SelectedIndex > 0) {
			$this->NewResourceLog->Display = 'None';
			list($component_type, $component_name) = explode(';', $this->Component->SelectedValue);
			if ($component_type == 'dir') {
				$resources = array(
					"Director",
					"JobDefs",
					"Client",
					"Job",
					"Storage",
					"Catalog",
					"Schedule",
					"Fileset",
					"Pool",
					"Messages",
					"Console"
				);
			} elseif ($component_type == 'sd') {
				$resources = array(
					"Director",
					"Storage",
					"Device",
					"Autochanger",
					"Messages"
				);
			} elseif ($component_type == 'fd') {
				$resources = array(
					"Director",
					"FileDaemon",
					"Messages"
				);
			} elseif ($component_type == 'bcons') {
				$resources = array(
					"Director",
					"Console"
				);
			}
			$resources = array_combine($resources, $resources);
		}
		$this->Resource->DataSource = $resources;
		$this->Resource->dataBind();
	}

	public function getComponentType() {
		return $this->getViewState(self::COMPONENT_TYPE);
	}

	public function setComponentType($type) {
		$this->setViewState(self::COMPONENT_TYPE, $type);
	}

	public function getComponentName() {
		return $this->getViewState(self::COMPONENT_NAME);
	}

	public function setComponentName($name) {
		$this->setViewState(self::COMPONENT_NAME, $name);
	}

	public function getResourceType() {
		return $this->getViewState(self::RESOURCE_TYPE);
	}

	public function setResourceType($type) {
		$this->setViewState(self::RESOURCE_TYPE, $type);
	}

	public function createResource() {
		if ($this->Host->SelectedIndex > 0 && $this->Component->SelectedIndex > 0 && $this->Resource->SelectedValue) {
			$host = $this->Host->SelectedValue;
			list($component_type, $component_name) = explode(';', $this->Component->SelectedValue);
			$resource_type = $this->Resource->SelectedValue;
			$this->goToPage('NewResource', array(
				'host' => $host,
				'component_type' => $component_type,
				'component_name' => $component_name,
				'resource_type' => $resource_type
			));
		}
	}
}
?>
