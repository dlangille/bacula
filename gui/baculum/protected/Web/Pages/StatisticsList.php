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

/**
 * Statistics list page.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Page
 * @package Baculum Web
 */
class StatisticsList extends BaculumWebPage {

	const USE_CACHE = true;

	protected $admin = true;

	public $statistics = array();

	public function onInit($param) {
		parent::onInit($param);
		if ($this->IsPostBack || $this->IsCallBack) {
			return;
		}
		$misc = $this->getModule('misc');
		$components = array('dir', 'sd', 'fd');

		for ($i = 0; $i < count($components); $i++) {
			if (empty($_SESSION[$components[$i]])) {
				continue;
			}

			$result = $this->getModule('api')->get(
				array('config', $components[$i], 'statistics'),
				null,
				true,
				self::USE_CACHE
			);

			if ($result->error === 0) {
				for ($j = 0; $j < count($result->output); $j++) {
					$this->statistics[] = array(
						'statistics' => $result->output[$j]->Statistics->Name,
						'component_type' => $components[$i],
						'component_type_full' => $misc->getComponentFullName($components[$i]),
						'component_name' => $_SESSION[$components[$i]]
					);
				}
			}
		}
	}
}
?>
