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
Prado::using('Application.Web.Portlets.BConditional');
Prado::using('Application.Web.Portlets.DirectiveText');

class DirectiveMessages extends DirectiveListTemplate {

	private $directive_types = array(
		'DirectiveText'
	);

	public $destination_simple = array(
		'Console',
		'Stdout',
		'Stderr',
		'Syslog',
		'Catalog'
	);

	public $destination_address = array(
		'Director',
		'File',
		'Append',
		'Mail',
		'MailOnError',
		'MailOnSuccess',
		'Operator'
	);

	private $messages_types = array(
		'All',
		'Debug',
		'Info',
		'Warning',
		'Error',
		'Fatal',
		'Terminate',
		'Saved',
		'Skipped',
		'Mount',
		'Restored',
		'Security',
		'Alert',
		'Volmgmt'
	);

	public function loadConfig($sender, $param) {
		$load_values = $this->getLoadValues();
		$destinations = (array)$this->getData();
		if (array_key_exists('Type', $destinations)) {
			$destinations = array($destinations);
		}
		$directives = array();
		for ($i = 0; $i < count($destinations); $i++) {
			$is_address_type = in_array($destinations[$i]['Type'], $this->destination_address);
			$directive_value = null;
			if ($is_address_type && array_key_exists('Where', $destinations[$i])) {
				$directive_value = implode(',', $destinations[$i]['Where']);
			}
			$this->setDirectiveName($destinations[$i]['Type']);
			$directives[$i] = array(
				'host' => $this->getHost(),
				'component_type' => $this->getComponentType(),
				'component_name' => $this->getComponentName(),
				'resource_type' => $this->getResourceType(),
				'resource_name' => $this->getResourceName(),
				'directive_name' => $destinations[$i]['Type'],
				'directive_value' => $directive_value,
				'default_value' => false,
				'required' => false,
				'label' => $destinations[$i]['Type'],
				'field_type' => 'TextBox',
				'in_config' => true,
				'show' => true,
				'parent_name' => __CLASS__,
				'is_address_type' => $is_address_type,
				'messages_types' => array()
			);
			$value_all = $value_not = null;
			for ($j = 0; $j < count($this->messages_types); $j++) {
				$value_all = in_array('!' . $this->messages_types[$j], $destinations[$i]['MsgTypes']);
				$value_not = in_array($this->messages_types[$j], $destinations[$i]['MsgTypes']);
				$directives[$i]['messages_types'][] = array(
					'host' => $this->getHost(),
					'component_type' => $this->getComponentType(),
					'component_name' => $this->getComponentName(),
					'resource_type' => $this->getResourceType(),
					'resource_name' => $this->getResourceName(),
					'directive_name' => $this->messages_types[$j],
					'directive_value' => ($value_all || $value_not),
					'default_value' => false,
					'required' => false,
					'label' => $this->messages_types[$j],
					'field_type' => 'Messages',
					'data' => $destinations[$i]['Type'],
					'in_config' => true,
					'show' => true,
					'parent_name' => __CLASS__
				);
			}
		}
		$this->RepeaterMessages->dataSource = $directives;
		$this->RepeaterMessages->dataBind();
	}

	public function getDirectiveValue() {
		$values = array();
		$controls = $this->RepeaterMessages->getControls();
		for ($i = 0; $i < $controls->count(); $i++) {
			$directive_values = array();
			$where_control = $controls->itemAt($i)->findControlsByType('DirectiveText');
			if (count($where_control) === 1 && $where_control[0]->getShow() === true) {
				$directive_values = array($where_control[0]->getDirectiveValue());
			}
			$types_control = $controls->itemAt($i)->Types;
			$types = $types_control->getDirectiveValues();
			$directive_name = $types_control->getDirectiveName();
			if (count($types) > 0 && count($directive_values) > 0) {
				array_push($directive_values, '=');
			}
			array_push($directive_values, implode(', ', $types));
			$directive_values = array_filter($directive_values);
			if (count($directive_values) === 0) {
				continue;
			}
			$values[$directive_name] = implode(' ', $directive_values);
		}
		$ret = null;
		if (count($values) > 1) {
			// multiple messages values
			$ret = $values;
		} else {
			// single messages value
			$ret = implode('', array_values($values));
		}
		return $ret;
	}

	public function getDirectiveData() {
		$values = array();
		$controls = $this->RepeaterMessages->getControls();
		for ($i = 0; $i < $controls->count(); $i++) {
			$directive_values = array();
			$where_control = $controls->itemAt($i)->findControlsByType('DirectiveText');
			if (count($where_control) === 1 && $where_control[0]->getShow() === true) {
				$directive_values['Where'] = array($where_control[0]->getDirectiveValue());
			}
			$types_control = $controls->itemAt($i)->Types;
			$directive_values['MsgTypes'] = $types_control->getDirectiveValues();
			$directive_values['Type'] = $types_control->getDirectiveName();
			array_push($values, $directive_values);
		}
		return $values;
	}

	public function createDirectiveListElement($sender, $param) {
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
				$control->setLabel($param->Item->DataItem['label']);
				$control->setInConfig($param->Item->DataItem['in_config']);
				$control->setShow($param->Item->DataItem['is_address_type']);
				$control->setParentName($param->Item->DataItem['parent_name']);
				break;
			}
		}
		$param->Item->Types->setData($param->Item->DataItem['messages_types']);
		$param->Item->Types->setDirectiveName($param->Item->DataItem['directive_name']);
	}

	public function loadMessageTypes($sender, $param) {
		$param->Item->Types->loadConfig();
	}

	public function newMessagesDirective($sender, $param) {
		$data = $this->getDirectiveData();
		$msg_type = $sender->getID();
		array_push($data, array('Type' => $msg_type, 'MsgTypes' => array()));
		$data = (object)$data;
		$this->setData($data);
		$this->loadConfig(null, null);
	}
}
