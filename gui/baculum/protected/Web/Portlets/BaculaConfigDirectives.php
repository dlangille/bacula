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

Prado::using('System.Web.UI.ActiveControls.TActiveRepeater');
Prado::using('System.Web.UI.ActiveControls.TActiveLabel');
Prado::using('Application.Web.Portlets.BConditional');
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveBoolean');
Prado::using('Application.Web.Portlets.DirectiveComboBox');
Prado::using('Application.Web.Portlets.DirectiveInteger');
Prado::using('Application.Web.Portlets.DirectiveText');
Prado::using('Application.Web.Portlets.DirectiveTimePeriod');
Prado::using('Application.Web.Portlets.DirectiveRunscript');
Prado::using('Application.Web.Portlets.DirectiveMessages');
Prado::using('Application.IO.TTextWriter');

class BaculaConfigDirectives extends DirectiveListTemplate {

	const SHOW_ALL_DIRECTIVES = 'ShowAllDirectives';

	private $show_all_directives = false;

	public $resource_names = array();

	private $directive_types = array(
		'DirectiveBoolean',
		'DirectiveComboBox',
		'DirectiveInteger',
		'DirectiveText',
		'DirectiveTimePeriod'
	);

	private $directive_list_types = array(
		'DirectiveFileSet',
		'DirectiveSchedule',
		'DirectiveMessages',
		'DirectiveRunscript'
	);

	private function getConfigData($host, array $parameters) {
		$default_params = array('config');
		$params = array_merge($default_params, $parameters);
		$result = $this->Application->getModule('api')->get($params, $host, false);
		$config = array();
		if (is_object($result) && $result->error === 0 && (is_object($result->output) || is_array($result->output))) {
			$config = $result->output;
		}
		return $config;
	}

	public function loadConfig() {
		$load_values = $this->getLoadValues();

		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resource_type = $this->getResourceType();
		$resource_name = $this->getResourceName();
		$directives = array();
		$parent_directives = array();
		$config = new stdClass;
		if ($load_values === true) {
			$config = $this->getConfigData($host, array(
				$component_type,
				$resource_type,
				$resource_name
			));
			if ($resource_type === 'Job' && property_exists($config, 'JobDefs')) {
				$parent_directives = $this->getConfigData($host, array(
					$component_type,
					'JobDefs',
					$config->JobDefs
				));
			}
		}
		$data_desc = $this->Application->getModule('data_desc');
		$resource_desc = $data_desc->getDescription($component_type, $resource_type);
		foreach ($resource_desc as $directive_name => $directive_desc) {
			$in_config = false;
			if ($load_values === true) {
				$in_config = property_exists($config, $directive_name);
			}

			$directive_value = null;
			if ($in_config === true && $load_values === true) {
				$directive_value = $config->{$directive_name};
			}

			$default_value = null;
			$data = null;
			$field_type = 'TextBox';
			$resource = null;
			$required = false;
			// @TODO: Add support for all directive properties defined in description file
			if (is_object($directive_desc)) {
				if (property_exists($directive_desc, 'Required')) {
					$required = $directive_desc->Required;
					if ($load_values === true && array_key_exists($directive_name, $parent_directives)) {
						// values can be taken from JobDefs
						$required = false;
					}
				}
				if (property_exists($directive_desc, 'DefaultValue')) {
					$default_value = $directive_desc->DefaultValue;
				}
				if (property_exists($directive_desc, 'Data')) {
					$data = $directive_desc->Data;
				}
				if (property_exists($directive_desc, 'FieldType')) {
					$field_type = $directive_desc->FieldType;
				}
				if (property_exists($directive_desc, 'Resource')) {
					$resource = $directive_desc->Resource;
				}
			}

			if (!is_array($directive_value) && !is_object($directive_value)) {
				$directive_value = array($directive_value);
			}
			if (is_object($directive_value)) {
				$directive_value = (array)$directive_value;
			}
			foreach ($directive_value as $key => $value) {
				$directive = array(
					'host' => $host,
					'component_type' => $component_type,
					'component_name' => $component_name,
					'resource_type' => $resource_type,
					'resource_name' => $resource_name,
					'directive_name' => $directive_name,
					'directive_value' => $value,
					'default_value' => $default_value,
					'required' => $required,
					'data' => $data,
					'resource' => $resource,
					'field_type' => $field_type,
					'label' => $directive_name,
					'in_config' => $in_config,
					'show' => (($in_config || !$load_values) || $this->getShowAllDirectives())
				);
				array_push($directives, $directive);
			}
		}
		$config = $this->getConfigData($host, array($component_type));
		for ($i = 0; $i < count($config); $i++) {
			$resource_type = $this->getConfigResourceType($config[$i]);
			$resource_name = property_exists($config[$i]->{$resource_type}, 'Name') ? $config[$i]->{$resource_type}->Name : '';
			if (!array_key_exists($resource_type, $this->resource_names)) {
				$this->resource_names[$resource_type] = array();
			}
			array_push($this->resource_names[$resource_type], $resource_name);
		}
		$this->setResourceNames($this->resource_names);

		$this->RepeaterDirectives->DataSource = $directives;
		$this->RepeaterDirectives->dataBind();
	}

	public function loadDirectives($sender, $param) {
		$show_all_directives = !$this->getShowAllDirectives();
		$this->setShowAllDirectives($show_all_directives);
		$this->loadConfig();
	}

	public function unloadDirectives() {
		$this->RepeaterDirectives->DataSource = array();
		$this->RepeaterDirectives->dataBind();
	}

	public function createDirectiveElement($sender, $param) {
		$load_values = $this->getLoadValues();
		for ($i = 0; $i < count($this->directive_types); $i++) {
			$control = $this->getChildControl($param->Item, $this->directive_types[$i]);
			if (is_object($control)) {
				$control->setHost($param->Item->DataItem['host']);
				$control->setComponentType($param->Item->DataItem['component_type']);
				$control->setComponentName($param->Item->DataItem['component_name']);
				$control->setResourceType($param->Item->DataItem['resource_type']);
				$control->setResourceName($param->Item->DataItem['resource_name']);
				$control->setDirectiveName($param->Item->DataItem['directive_name']);
				$control->setDirectiveValue($param->Item->DataItem['directive_value']);
				$control->setDefaultValue($param->Item->DataItem['default_value']);
				$control->setRequired($param->Item->DataItem['required']);
				$control->setData($param->Item->DataItem['data']);
				$control->setResource($param->Item->DataItem['resource']);
				$control->setLabel($param->Item->DataItem['label']);
				$control->setInConfig($param->Item->DataItem['in_config']);
				$show_all_directives = ($param->Item->DataItem['in_config'] || !$load_values || $this->getShowAllDirectives());
				$control->setShow($show_all_directives);
				$control->setResourceNames($this->resource_names);
				break;
			}
		}
		for ($i = 0; $i < count($this->directive_list_types); $i++) {
			$control = $this->getChildControl($param->Item, $this->directive_list_types[$i]);
			if (is_object($control)) {
				$control->setHost($param->Item->DataItem['host']);
				$control->setComponentType($param->Item->DataItem['component_type']);
				$control->setComponentName($param->Item->DataItem['component_name']);
				$control->setResourceType($param->Item->DataItem['resource_type']);
				$control->setResourceName($param->Item->DataItem['resource_name']);
				$control->setDirectiveName($param->Item->DataItem['directive_name']);
				$control->setData($param->Item->DataItem['directive_value']);
				$control->setLoadValues($this->getLoadValues());
				$control->setResourceNames($this->resource_names);
				$control->raiseEvent('OnDirectiveListLoad', $this, null);
			}
		}
	}


	public function saveResource($sender, $param) {
		$directives = array();
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$resource_type = $this->getResourceType();
		$resource_desc = $this->Application->getModule('data_desc')->getDescription($component_type, $resource_type);
		for ($i = 0; $i < count($this->directive_types); $i++) {
			$controls = $this->RepeaterDirectives->findControlsByType($this->directive_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$parent_name = $controls[$j]->getParentName();
				if (!is_null($parent_name)) {
					continue;
				}
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				$default_value = $resource_desc[$directive_name]->DefaultValue;
				$in_config = $controls[$j]->getInConfig();
				if (is_null($directive_value)) {
					// skip not changed values that don't exist in config
					continue;
				}
				if ($this->directive_types[$i] === 'DirectiveBoolean') {
					settype($default_value, 'bool');
				} elseif ($this->directive_types[$i] === 'DirectiveInteger') {
					settype($directive_value, 'int');
				}
				if ($directive_value === $default_value && $in_config === false) {
					// value the same as default value, skip it
					continue;
				}
				$directives[$directive_name] = $directive_value;
			}
		}
		for ($i = 0; $i < count($this->directive_list_types); $i++) {
			$controls = $this->RepeaterDirectives->findControlsByType($this->directive_list_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				if (is_null($directive_value)) {
					continue;
				}
				if (!array_key_exists($directive_name, $directives)) {
					$directives[$directive_name] = array();
				}
				if (is_array($directive_value)) {
					if ($this->directive_list_types[$i] === 'DirectiveMessages') {
						$directives = array_merge($directives, $directive_value);
					} elseif ($this->directive_list_types[$i] === 'DirectiveRunscript') {
						if (!isset($directives[$directive_name])) {
							$directives[$directive_name] = array();
						}
						$directives[$directive_name] = array_merge($directives[$directive_name], $directive_value[$directive_name]);
					} elseif (array_key_exists($directive_name, $directive_value)) {
						$directives[$directive_name][] = $directive_value[$directive_name];
					} elseif (count($directive_value) > 0) {
						$directives[$directive_name][] = $directive_value;
					}
				} else {
					$directives[$directive_name] = $directive_value;
				}
			}
		}
		$load_values = $this->getLoadValues();
		if ($load_values === true) {
			$resource_name = $this->getResourceName();
		} else {
			$resource_name = array_key_exists('Name', $directives) ? $directives['Name'] : '';
		}

		$params = array(
			'config',
			$component_type,
			$resource_type,
			$resource_name
		);
		$result = $this->Application->getModule('api')->set($params, array('config' => json_encode($directives)), $host, false);
		if ($result->error === 0) {
			$this->SaveDirectiveOk->Display = 'Dynamic';
			$this->SaveDirectiveError->Display = 'None';
			$this->SaveDirectiveErrMsg->Text = '';
		} else {
			$this->SaveDirectiveOk->Display = 'None';
			$this->SaveDirectiveError->Display = 'Dynamic';
			$this->SaveDirectiveErrMsg->Display = 'Dynamic';
			$this->SaveDirectiveErrMsg->Text = "Error {$result->error}: {$result->output}";
		}
	}

	public function setShowAllDirectives($show_all_directives) {
		$this->setViewState(self::SHOW_ALL_DIRECTIVES, $show_all_directives);
	}

	public function getShowAllDirectives() {
		return $this->getViewState(self::SHOW_ALL_DIRECTIVES, false);
	}
}
?>
