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
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveCheckBox');
Prado::using('Application.Web.Portlets.DirectiveComboBox');
Prado::using('Application.Web.Portlets.DirectiveInteger');
Prado::using('Application.Web.Portlets.DirectiveListBox');
Prado::using('Application.Web.Portlets.DirectiveSize');
Prado::using('Application.Web.Portlets.DirectiveTextBox');
Prado::using('Application.Web.Portlets.DirectiveMultiTextBox');
Prado::using('Application.Web.Portlets.DirectiveTimePeriod');
Prado::using('Application.Web.Portlets.DirectiveRunscript');
Prado::using('Application.Web.Portlets.DirectiveMessages');

class DirectiveRenderer extends DirectiveListTemplate implements IItemDataRenderer {

	const DATA = 'Data';
	const ITEM_INDEX = 'ItemIndex';

	private $directive_types = array(
		'DirectiveCheckBox',
		'DirectiveComboBox',
		'DirectiveInteger',
		'DirectiveListBox',
		'DirectiveTextBox',
		'DirectiveSize',
		'DirectiveTimePeriod'
	);

	private $directive_list_types = array(
		'DirectiveFileSet',
		'DirectiveSchedule',
		'DirectiveMessages',
		'DirectiveRunscript',
		'DirectiveMultiTextBox'
	);

	public $resource_names = array();

	public function onLoad($param) {
		parent::onLoad($param);
		$data = $this->getData();
		$this->createItem($data);
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
			$this->getControls()->add($control);
			$control->createDirective();
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
			$this->getControls()->add($control);
			if (!$this->getPage()->IsCallBack || $this->getPage()->getCallbackEventParameter()  === 'show_all_directives' || $this->getCmdParam() === 'show') {
				/*
				 * List types should be loaded only by load request, not by callback request.
				 * Otherwise OnLoad above is called during callback and overwrites data in controls.
				 */
				$control->raiseEvent('OnDirectiveListLoad', $this, null);
			}
		}
	}

	public function getItemIndex() {
		return $this->getViewState(self::ITEM_INDEX, 0);
	}

	public function setItemIndex($item_index) {
		$this->setViewState(self::ITEM_INDEX, $item_index);
	}

	public function getItemType() {
	}

	public function setItemType($item_type) {
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
