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


Prado::using('System.Web.Ui.ActiveControls.TActiveRepeater');
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveBoolean');

class MessageTypes extends DirectiveListTemplate {

	public function loadConfig() {
		$this->RepeaterMessageTypes->dataSource = $this->getData();
		$this->RepeaterMessageTypes->dataBind();
	}

	public function getDirectiveValues() {
		$type_controls = $this->RepeaterMessageTypes->findControlsByType('DirectiveBoolean');
		$is_all = false;
		$types = array();
		for ($i = 0; $i < count($type_controls); $i++) {
			$directive_name = $type_controls[$i]->getDirectiveName();
			$directive_value = $type_controls[$i]->getDirectiveValue();
			if (is_null($directive_value) || $directive_value === false) {
				continue;
			}
			if ($directive_name === 'All' && $directive_value === true) {
				$is_all = true;
			}
			$neg = $directive_name != 'All' && $directive_value === true && $is_all === true ? '!' : '';
			array_push($types, "{$neg}{$directive_name}");
		}
		return $types;
	}

	public function createTypeListElement($sender, $param) {
		$control = $this->getChildControl($param->Item, 'DirectiveBoolean');
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
			$control->setData($param->Item->DataItem['directive_value']);
			$control->setInConfig($param->Item->DataItem['in_config']);
			$control->setShow($param->Item->DataItem['show']);
			$control->setParentName($param->Item->DataItem['parent_name']);
		}
	}
}