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
Prado::using('Application.Web.Portlets.ResourceListTemplate');

class BaculaConfigResources extends ResourceListTemplate {

	const CHILD_CONTROL = 'BaculaConfigDirectives';

	public $resource_names = array();

	private function getConfigData($host, $component_type) {
		$params = array('config', $component_type);
		$result = $this->Application->getModule('api')->get($params, $host, false);
		$config = array();
		if (is_object($result) && $result->error === 0 && is_array($result->output)) {
			$config = $result->output;
		}
		return $config;
	}

	public function loadConfig() {
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resources = array();
		$config = $this->getConfigData($host, $component_type);
		for ($i = 0; $i < count($config); $i++) {
			$resource_type = $this->getConfigResourceType($config[$i]);
			$resource_name = property_exists($config[$i]->{$resource_type}, 'Name') ? $config[$i]->{$resource_type}->Name : '';
			$resource = array(
				'host' => $host,
				'component_type' => $component_type,
				'component_name' => $component_name,
				'resource_type' => $resource_type,
				'resource_name' => $resource_name
			);
			array_push($resources, $resource);
			if (!array_key_exists($resource_type, $this->resource_names)) {
				$this->resource_names[$resource_type] = array();
			}
			array_push($this->resource_names[$resource_type], $resource_name);
		}

		$this->RepeaterResources->DataSource = $resources;
		$this->RepeaterResources->dataBind();
	}

	public function createResourceListElement($sender, $param) {
		$control = $this->getChildControl($param->Item, self::CHILD_CONTROL);
		if (is_object($control)) {
			$control->setHost($param->Item->DataItem['host']);
			$control->setComponentType($param->Item->DataItem['component_type']);
			$control->setComponentName($param->Item->DataItem['component_name']);
			$control->setResourceType($param->Item->DataItem['resource_type']);
			$control->setResourceName($param->Item->DataItem['resource_name']);
			$control->setResourceNames($this->resource_names);
		}
	}

	public function getDirectives($sender, $param) {
		$control = $this->getChildControl($sender->getParent(), self::CHILD_CONTROL);
		if (is_object($control)) {
			$control->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}
}
?>
