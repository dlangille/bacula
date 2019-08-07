<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2019 Kern Sibbald
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
Prado::using('System.Web.UI.ActiveControls.TActivePanel');
Prado::using('System.Web.UI.ActiveControls.TActiveRepeater');
Prado::using('System.Web.UI.WebControls.TItemDataRenderer');
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveCheckBox');
Prado::using('Application.Web.Portlets.DirectiveComboBox');
Prado::using('Application.Web.Portlets.DirectiveInteger');
Prado::using('Application.Web.Portlets.DirectiveListBox');
Prado::using('Application.Web.Portlets.DirectivePassword');
Prado::using('Application.Web.Portlets.DirectiveSize');
Prado::using('Application.Web.Portlets.DirectiveSpeed');
Prado::using('Application.Web.Portlets.DirectiveTextBox');
Prado::using('Application.Web.Portlets.DirectiveMultiComboBox');
Prado::using('Application.Web.Portlets.DirectiveMultiTextBox');
Prado::using('Application.Web.Portlets.DirectiveTimePeriod');
Prado::using('Application.Web.Portlets.DirectiveRunscript');
Prado::using('Application.Web.Portlets.DirectiveMessages');

class DirectiveRenderer extends TItemDataRenderer {

	const DATA = 'Data';

	private $directive_types = array(
		'DirectiveCheckBox',
		'DirectiveComboBox',
		'DirectiveInteger',
		'DirectiveListBox',
		'DirectivePassword',
		'DirectiveTextBox',
		'DirectiveSize',
		'DirectiveSpeed',
		'DirectiveTimePeriod'
	);

	private $directive_list_types = array(
		'DirectiveFileSet',
		'DirectiveSchedule',
		'DirectiveMessages',
		'DirectiveRunscript',
		'DirectiveMultiComboBox',
		'DirectiveMultiTextBox'
	);

	public function loadState() {
		parent::loadState();
		$this->createItemInternal();
	}

	public function createItemInternal() {
		$data = $this->getData();
		$item = $this->createItem($data);
		$this->addParsedObject($item);
	}

	public function createItem($data) {
		$load_values = $this->SourceTemplateControl->getLoadValues();
		$field = $this->getField($data['field_type']);
		$control = Prado::createComponent($field);
		$type = 'Directive' . $data['field_type'];
		if (in_array($type, $this->directive_types)) {
			$control->setHost($data['host']);
			$control->setComponentType($data['component_type']);
			$control->setComponentName($data['component_name']);
			$control->setResourceType($data['resource_type']);
			$control->setResourceName($data['resource_name']);
			$control->setDirectiveName($data['directive_name']);
			$control->setDirectiveValue($data['directive_value']);
			$control->setDefaultValue($data['default_value']);
			$control->setRequired($data['required']);
			$control->setData($data['data']);
			$control->setResource($data['resource']);
			$control->setLabel($data['label']);
			$control->setInConfig($data['in_config']);
			$control->setShow($data['show']);
			$control->setGroupName($data['group_name']);
			$control->setParentName($data['parent_name']);
			$control->setResourceNames($this->SourceTemplateControl->getResourceNames());
		} elseif (in_array($type, $this->directive_list_types)) {
			$control->setHost($data['host']);
			$control->setComponentType($data['component_type']);
			$control->setComponentName($data['component_name']);
			$control->setResourceType($data['resource_type']);
			$control->setResourceName($data['resource_name']);
			$control->setDirectiveName($data['directive_name']);
			$control->setData($data['directive_value']);
			$control->setParentName($data['parent_name']);
			$control->setLoadValues($this->SourceTemplateControl->getLoadValues());
			$control->setResourceNames($this->SourceTemplateControl->getResourceNames());
			$control->setShow($data['show']);
			$control->setGroupName($data['group_name']);
			$control->setResource($data['resource']);
		}
		return $control;
	}

	public function getData() {
		return $this->getViewState(self::DATA);
	}

	public function setData($data) {
		$this->setViewState(self::DATA, $data);
	}

	private function getField($field_type) {
		return 'Application.Web.Portlets.Directive' . $field_type;
	}
}
?>
