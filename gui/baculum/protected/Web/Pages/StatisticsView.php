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

Prado::using('Application.Web.Class.BaculumWebPage');

class StatisticsView extends BaculumWebPage {

	const COMPONENT_TYPE = 'ComponentType';
	const STATISTICS_NAME = 'StatisticsName';

	protected $admin = true;

	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsPostBack || $this->IsCallBack) {
			return;
		}

		$components = $this->getModule('misc')->getComponents();
		if (in_array($this->Request['component_type'], $components)) {
			$this->setComponentType($this->Request['component_type']);
		} else {
			$wmsg = sprintf(
				'Invalid component type "%s" on %s',
				$this->Request['component_type'],
				__CLASS__
			);
			$this->Application->getModule('logging')->log(
				__FUNCTION__,
				$wmsg,
				Logging::CATEGORY_SECURITY,
				__FILE__,
				__LINE__
			);
			$this->StatisticsConfig->setVisible(false);
		}
		$this->setStatisticsName($this->Request['statistics']);
	}

	public function onPreRender($param) {
		parent::onPreRender($param);
		if ($this->IsCallBack || $this->IsPostBack) {
			return;
		}
		$component_type = $this->getComponentType();
		if (key_exists($component_type, $_SESSION) && !empty($_SESSION[$component_type])) {
			$this->StatisticsConfig->setComponentType($component_type);
			$this->StatisticsConfig->setComponentName($_SESSION[$component_type]);
			$this->StatisticsConfig->setResourceName($this->getStatisticsName());
			$this->StatisticsConfig->setLoadValues(true);
			$this->StatisticsConfig->raiseEvent('OnDirectiveListLoad', $this, null);
		}
	}

	public function getComponentType() {
		return $this->getViewState(self::COMPONENT_TYPE, '');
	}

	public function setComponentType($component_type) {
		$this->setViewState(self::COMPONENT_TYPE, $component_type);
	}

	public function getStatisticsName() {
		return $this->getViewState(self::STATISTICS_NAME, '');
	}

	public function setStatisticsName($statistics_name) {
		$this->setViewState(self::STATISTICS_NAME, $statistics_name);
	}
}
?>
