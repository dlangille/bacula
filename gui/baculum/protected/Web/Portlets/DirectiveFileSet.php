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
Prado::using('System.Web.UI.ActiveControls.TActiveLinkButton');
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveBoolean');
Prado::using('Application.Web.Portlets.DirectiveText');
Prado::using('Application.Web.Portlets.DirectiveComboBox');
Prado::using('Application.Web.Portlets.DirectiveInteger');

class DirectiveFileSet extends DirectiveListTemplate {

	const MENU_CONTROL = 'NewFileSetMenu';

	private $directive_types = array(
		'DirectiveBoolean',
		'DirectiveText',
		'DirectiveComboBox',
		'DirectiveInteger'
	);

	private $directive_inc_exc_types = array(
		'DirectiveText'
	);

	public function loadConfig($sender, $param) {
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentType();
		$resource_type = $this->getResourceType();
		$directive_name = $this->getDirectiveName();
		$directives = $this->getData();
		$data_source = array();
		$include = array();
		$exclude = array();
		$options = array();
		if (is_object($directives)) { // Include with options
			foreach($directives as $name => $values) {
				switch($name) {
					case 'File': {
						$this->setFile($include, $name, $values);
						break;
					}
					case 'Options': {
						$this->setOption($options, $name, $values);
						break;
					}
					case 'Exclude': {
						$this->setFile($exclude, $name, $values);
						break;
					}
				}
			}
		}

		$this->RepeaterFileSetOptions->DataSource = $options;
		$this->RepeaterFileSetOptions->dataBind();
		$this->RepeaterFileSetInclude->DataSource = $include;
		$this->RepeaterFileSetInclude->dataBind();
		$this->RepeaterFileSetExclude->DataSource = $exclude;
		$this->RepeaterFileSetExclude->dataBind();
		$this->FileSetMenu->setComponentType($component_type);
		$this->FileSetMenu->setComponentName($component_name);
		$this->FileSetMenu->setResourceType($resource_type);
		$this->FileSetMenu->setDirectiveName($directive_name);
	}

	private function setFile(&$files, $name, $config) {
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resource_type = $this->getResourceType();
		$resource_name = $this->getResourceName();

		for ($i = 0; $i < count($config); $i++) {
			$files[] = array(
				'host' => $host,
				'component_type' => $component_type,
				'component_name' => $component_name,
				'resource_type' => $resource_type,
				'resource_name' => $resource_name,
				'directive_name' => $name,
				'directive_value' => $config[$i],
				'parent_name' => $name
			);
		}
	}

	private function setOption(&$options, $name, $config) {
		$load_values = $this->getLoadValues();
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resource_type = $this->getResourceType();
		$resource_name = $this->getResourceName();

		$resource_desc = $this->Application->getModule('data_desc')->getDescription($component_type, $resource_type, 'Include');

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
						'field_type' => $field_type,
						'in_config' => $in_config,
						'label' => $directive_name,
						'show' => ($in_config || !$load_values || $this->SourceTemplateControl->getShowAllDirectives()),
						'parent_name' => $name,
						'group_name' => $i
					);
				}
			}
		}
	}

	public function getDirectiveValue() {
		$directive_values = array();
		$component_type = $this->getComponentType();
		$resource_type = $this->getResourceType();
		$resource_desc = $this->Application->getModule('data_desc')->getDescription($component_type, $resource_type);

		for ($i = 0; $i < count($this->directive_types); $i++) {
			$controls = $this->RepeaterFileSetOptions->findControlsByType($this->directive_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				$index = $controls[$j]->getGroupName();
				$default_value = $resource_desc['Include']->SubSections->{$directive_name}->DefaultValue;
				$in_config = $controls[$j]->getInConfig();
				if (is_null($directive_value)) {
					// option not set or removed
					continue;
				}
				if ($this->directive_types[$i] === 'DirectiveBoolean') {
					settype($default_value, 'bool');
				}
				if ($directive_value === $default_value) {
					// value the same as default value, skip it
					continue;
				}
				if (!array_key_exists('Include', $directive_values)) {
					$directive_values['Include'] = array('Options' => array());
				}
				if (!isset($directive_values['Include']['Options'][$index])) {
					$directive_values['Include']['Options'][$index] = array();
				}
				$directive_values['Include']['Options'][$index][$directive_name] = $directive_value;
			}
			$controls = $this->RepeaterFileSetInclude->findControlsByType($this->directive_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				if (is_null($directive_value)) {
					// Include file directive removed
					continue;
				}
				if (!array_key_exists('Include', $directive_values)) {
					$directive_values['Include'] = array();
				}
				if (!array_key_exists($directive_name, $directive_values['Include'])) {
					$directive_values['Include'][$directive_name] = array();
				}
				array_push($directive_values['Include'][$directive_name], $directive_value);
			}
			$controls = $this->RepeaterFileSetExclude->findControlsByType($this->directive_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				if (is_null($directive_value)) {
					// Exclude file directive removed
					continue;
				}
				if (!array_key_exists($directive_name, $directive_values)) {
					$directive_values[$directive_name] = array('File' => array());
				}
				array_push($directive_values[$directive_name]['File'], $directive_value);
			}
		}

		return $directive_values;
	}

	public function createFileSetOptions($sender, $param) {
		$load_values = $this->getLoadValues();
		$bconditionals = $this->RepeaterFileSetOptions->findControlsByType('BConditionalItem');
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
				}
			}
		}
	}

	public function createFileSetIncExcElement($sender, $param) {
		for ($i = 0; $i < count($this->directive_inc_exc_types); $i++) {
			$control = $this->getChildControl($param->Item, $this->directive_inc_exc_types[$i]);
			if (is_object($control)) {
				$control->setHost($param->Item->DataItem['host']);
				$control->setComponentType($param->Item->DataItem['component_type']);
				$control->setComponentName($param->Item->DataItem['component_name']);
				$control->setResourceType($param->Item->DataItem['resource_type']);
				$control->setResourceName($param->Item->DataItem['resource_name']);
				$control->setDirectiveName($param->Item->DataItem['directive_name']);
				$control->setDirectiveValue($param->Item->DataItem['directive_value']);
				$control->setLabel($param->Item->DataItem['directive_name']);
				$control->setData($param->Item->DataItem['directive_value']);
				$control->setInConfig(true);
				$control->setShow(true);
				$control->setParentName($param->Item->DataItem['parent_name']);
			}
		}
	}

	private function getDirectiveData() {
		$values = $this->getDirectiveValue();
		$data = array();
		if (array_key_exists('Include', $values) && array_key_exists('File', $values['Include'])) {
			$data['File'] = $values['Include']['File'];
			if (array_key_exists('Options', $values['Include']) && is_array($values['Include']['Options'])) {
				$data['Options'] = array();
				for ($i = 0; $i < count($values['Include']['Options']); $i++) {
					$data['Options'][$i] = (object)$values['Include']['Options'][$i];
				}
			}
		}
		if (array_key_exists('Exclude', $values) && array_key_exists('File', $values['Exclude'])) {
			$data['Exclude'] = $values['Exclude']['File'];
		}
		return $data;
	}

	public function newIncludeFile($sender, $param) {
		$data = $this->getDirectiveData();
		if (array_key_exists('File', $data) && is_array($data['File'])) {
			$data['File'][] = '';
		} else {
			$data['File'] = array('');
		}
		$data = (object)$data;
		$this->setData($data);
		$this->loadConfig(null, null);
	}

	public function newExcludeFile($sender, $param) {
		$data = $this->getDirectiveData();
		if (array_key_exists('Exclude', $data) && is_array($data['Exclude'])) {
			$data['Exclude'][] = '';
		} else {
			$data['Exclude'] = array('');
		}
		$data = (object)$data;
		$this->setData($data);
		$this->loadConfig(null, null);
	}

	public function newIncludeOptions($sender, $param) {
		$data = $this->getDirectiveData();
		if (array_key_exists('Options', $data) && is_array($data['Options'])) {
			$data['Options'][] = new stdClass;
		} else {
			$data['Options'] = array(new stdClass);
		}
		$data = (object)$data;
		$this->SourceTemplateControl->setShowAllDirectives(true);
		$this->setData($data);
		$this->loadConfig(null, null);
	}
}
?>
