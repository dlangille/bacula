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

Prado::using('System.Web.UI.TTemplateControl');
Prado::using('System.Web.UI.ActiveControls.TActiveControlAdapter');
Prado::using('Application.Web.Portlets.IDirectiveField');

class DirectiveTemplate extends TTemplateControl implements IDirectiveField, IActiveControl {

	const HOST = 'Host';
	const COMPONENT_TYPE = 'ComponentType';
	const COMPONENT_NAME = 'ComponentName';
	const RESOURCE_TYPE = 'ResourceType';
	const RESOURCE_NAME = 'ResourceName';
	const DIRECTIVE_NAME = 'DirectiveName';
	const DIRECTIVE_VALUE = 'DirectiveValue';
	const DEFAULT_VALUE = 'DefaultValue';
	const REQUIRED = 'Required';
	const DATA = 'Data';
	const RESOURCE = 'Resource';
	const LABEL = 'Label';
	const IN_CONFIG = 'InConfig';
	const SHOW = 'Show';
	const RESOURCE_NAMES = 'ResourceNames';
	const PARENT_NAME = 'ParentName';
	const GROUP_NAME = 'GroupName';
	const IS_DIRECTIVE_CREATED = 'IsDirectiveCreated';

	public $display_directive;

	private $command_params = array('save', 'add');

	public function __construct() {
		parent::__construct();
		$this->setAdapter(new TActiveControlAdapter($this));
	}

	public function getActiveControl() {
		return $this->getAdapter()->getBaseActiveControl();
	}

	public function bubbleEvent($sender, $param) {
		if ($param instanceof TCommandEventParameter) {
			$this->raiseBubbleEvent($this, $param);
			return true;
		} else {
			return false;
		}
	}

	public function saveValue($sender, $param) {
		if (!$this->getPage()->IsPostBack) {
			return;
		}
		$command_param = $this->getCmdParam();
		if ($command_param === 'save' && method_exists($this, 'getValue')) {
			$new_value = $this->getValue();
			$this->setDirectiveValue($new_value);
		}
	}

	private function getCmdParam() {
		$command_param = null;
		if (method_exists($this->getPage()->CallBackEventTarget, 'getCommandParameter')) {
			$command_param = $this->getPage()->CallBackEventTarget->getCommandParameter();
		}
		return $command_param;
	}

	public function onPreRender($param) {
		parent::onPreRender($param);
		if (!$this->getIsDirectiveCreated()) {
			$this->createDirective();
			$this->setIsDirectiveCreated(true);
		}
		// show directives existing in config or all
		$this->display_directive = $this->getShow();
	}

	public function createDirective() {
		// so far nothing to do
	}

	public function loadValue($value_obj) {
		$value_obj->raisePostDataChangedEvent();
	}

	public function getHost() {
		return $this->getViewState(self::HOST);
	}

	public function setHost($host) {
		$this->setViewState(self::HOST, $host);
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

	public function getResourceName() {
		return $this->getViewState(self::RESOURCE_NAME);
	}

	public function setResourceName($name) {
		$this->setViewState(self::RESOURCE_NAME, $name);
	}

	public function getDirectiveName() {
		return $this->getViewState(self::DIRECTIVE_NAME);
	}

	public function setDirectiveName($name) {
		$this->setViewState(self::DIRECTIVE_NAME, $name);
	}

	public function getDirectiveValue() {
		return $this->getViewState(self::DIRECTIVE_VALUE);
	}

	public function setDirectiveValue($value) {
		$this->setViewState(self::DIRECTIVE_VALUE, $value);
	}

	public function getDefaultValue() {
		return $this->getViewState(self::DEFAULT_VALUE);
	}

	public function setDefaultValue($value) {
		$this->setViewState(self::DEFAULT_VALUE, $value);
	}

	public function getRequired() {
		return $this->getViewState(self::REQUIRED);
	}

	public function setRequired($value) {
		$value = TPropertyValue::ensureBoolean($value);
		$this->setViewState(self::REQUIRED, $value);
	}

	public function getLabel() {
		return $this->getViewState(self::LABEL);
	}

	public function setLabel($label) {
		$this->setViewState(self::LABEL, $label);
	}

	public function getInConfig() {
		return $this->getViewState(self::IN_CONFIG, false);
	}

	public function setInConfig($in_config) {
		$this->setViewState(self::IN_CONFIG, $in_config);
	}

	public function getShow() {
		return $this->getViewState(self::SHOW);
	}

	public function setShow($show) {
		$this->setViewState(self::SHOW, $show);
	}

	public function getResourceNames() {
		return $this->getViewState(self::RESOURCE_NAMES);
	}

	public function setResourceNames($resource_names) {
		$this->setViewState(self::RESOURCE_NAMES, $resource_names);
	}

	public function getData() {
		return $this->getViewState(self::DATA);
	}

	public function setData($data) {
		$this->setViewState(self::DATA, $data);
	}

	// Re-think name of the Resource property and use more general name if possible
	public function getResource() {
		return $this->getViewState(self::RESOURCE);
	}

	public function setResource($resource) {
		$this->setViewState(self::RESOURCE, $resource);
	}

	public function getParentName() {
		return $this->getViewState(self::PARENT_NAME);
	}

	public function setParentName($parent_name) {
		$this->setViewState(self::PARENT_NAME, $parent_name);
	}

	public function getGroupName() {
		return $this->getViewState(self::GROUP_NAME);
	}

	public function setGroupName($group_name) {
		$this->setViewState(self::GROUP_NAME, $group_name);
	}

	public function getIsDirectiveCreated() {
		return $this->getViewState(self::IS_DIRECTIVE_CREATED);
	}

	public function setIsDirectiveCreated($is_created) {
		$this->setViewState(self::IS_DIRECTIVE_CREATED, $is_created);
	}
}
?>
