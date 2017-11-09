<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2017 Kern Sibbald
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

Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveBoolean');
Prado::using('Application.Web.Portlets.DirectiveText');
Prado::using('Application.Web.Portlets.DirectiveComboBox');

class DirectiveRunscript extends DirectiveListTemplate {

	private $directive_types = array(
		'DirectiveBoolean',
		'DirectiveText',
		'DirectiveComboBox'
	);

	public function loadConfig($sender, $param) {
		$load_values = $this->getLoadValues();
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resource_type = $this->getResourceType();
		$resource_name = $this->getResourceName();

		$config = $this->getData();
		if (is_null($config)) {
			return null;
		} elseif (is_object($config)) {
			$config = array($config);
		}
		$options = array();
		$resource_desc = $this->Application->getModule('data_desc')->getDescription($component_type, $resource_type, 'Runscript');
		for ($i = 0; $i < count($config); $i++) {
			foreach ($resource_desc->SubSections as $directive_name => $directive_desc) {
				$in_config = property_exists($config[$i], $directive_name);

				$directive_value = null;
				if ($in_config === true) {
					$directive_value = $config[$i]->{$directive_name};
				}

				$default_value = null;
				$data = null;
				$field_type = 'TextBox';
				$required = false;
				if (is_object($directive_desc)) {
					if (property_exists($directive_desc, 'Required')) {
						$required = $directive_desc->Required;
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
				}
				if (!is_array($directive_value)) {
					$directive_value = array($directive_value);
				}
				for ($j = 0; $j < count($directive_value); $j++) {
					$options[] = array(
						'host' => $host,
						'component_type' => $component_type,
						'component_name' => $component_name,
						'resource_type' => $resource_type,
						'resource_name' => $resource_name,
						'directive_name' => $directive_name,
						'directive_value' => $directive_value[$j],
						'default_value' => $default_value,
						'required' => $required,
						'data' => $data,
						'label' => $directive_name,
						'field_type' => $field_type,
						'in_config' => $in_config,
						'show' => ($in_config || !$load_values || $this->SourceTemplateControl->getShowAllDirectives()),
						'parent_name' => __CLASS__,
						'group_name' => $i
					);
				}
			}
		}
		$this->RepeaterRunscriptOptions->dataSource = $options;
		$this->RepeaterRunscriptOptions->dataBind();
	}

	public function createRunscriptOptions($sender, $param) {
		$load_values = $this->getLoadValues();
		$bconditionals = $this->RepeaterRunscriptOptions->findControlsByType('BConditionalItem');
		for ($i = 0; $i < count($bconditionals); $i++) {
			$item = $bconditionals[$i]->getData();
			for ($j = 0; $j < count($this->directive_types); $j++) {
				$control = $this->getChildControl($item, $this->directive_types[$j]);
				if (is_object($control)) {
					$control->setHost($item->DataItem['host']);
					$control->setComponentType($item->DataItem['component_type']);
					$control->setComponentName($item->DataItem['component_name']);
					$control->setResourceType($item->DataItem['resource_type']);
					$control->setResourceName($item->DataItem['resource_name']);
					$control->setDirectiveName($item->DataItem['directive_name']);
					$control->setDirectiveValue($item->DataItem['directive_value']);
					$control->setDefaultValue($item->DataItem['default_value']);
					$control->setRequired($item->DataItem['required']);
					$control->setData($item->DataItem['data']);
					$control->setLabel($item->DataItem['label']);
					$control->setInConfig($item->DataItem['in_config']);
					$show_all_directives = ($item->DataItem['in_config'] || !$load_values || $this->SourceTemplateControl->getShowAllDirectives());
					$control->setShow($show_all_directives);
					$control->setParentName($item->DataItem['parent_name']);
					$control->setGroupName($item->DataItem['group_name']);
					break;
				}
			}
		}
	}

	public function getDirectiveValue() {
		$directive_values = null;
		$component_type = $this->getComponentType();
		$resource_type = $this->getResourceType();
		$resource_desc = $this->Application->getModule('data_desc')->getDescription($component_type, $resource_type);

		for ($i = 0; $i < count($this->directive_types); $i++) {
			$controls = $this->RepeaterRunscriptOptions->findControlsByType($this->directive_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				$default_value = $resource_desc['Runscript']->SubSections->{$directive_name}->DefaultValue;
				$in_config = $controls[$j]->getInConfig();
				$index = $controls[$j]->getGroupName();
				if (is_null($directive_value)) {
					// skip not changed values that don't exist in config
					continue;
				}
				if ($this->directive_types[$i] === 'DirectiveBoolean') {
					settype($default_value, 'bool');
				}
				if ($directive_value === $default_value) {
					// value the same as default value, skip it
					continue;
				}
				if (!isset($directive_values['Runscript'])) {
					$directive_values['Runscript'] = array();
				}
				if (!isset($directive_values['Runscript'][$index])) {
					$directive_values['Runscript'][$index] = array();
				}
				$directive_values['Runscript'][$index][$directive_name] = $directive_value;
			}
		}
		return $directive_values;
	}

	public function getDirectiveData() {
		$data = array();
		$values = $this->getDirectiveValue();
		if (is_array($values) && array_key_exists('Runscript', $values)) {
			for ($i = 0; $i < count($values['Runscript']); $i++) {
				$data[$i] = (object)$values['Runscript'][$i];
			}
		}
		return $data;
	}

	public function newRunscriptDirective() {
		$data = $this->getDirectiveData();
		array_push($data, new stdClass);
		$this->setData($data);
		$this->SourceTemplateControl->setShowAllDirectives(true);
		$this->loadConfig(null, null);
	}
}
