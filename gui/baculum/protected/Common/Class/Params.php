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

Prado::using('Application.Common.Class.CommonModule');
Prado::using('Application.Common.Class.Params');

class Params extends CommonModule {

	public static $months = array(
		'jan' => 'January',
		'feb' => 'February',
		'mar' => 'March',
		'apr' => 'April',
		'may' => 'May',
		'jun' => 'June',
		'jul' => 'July',
		'aug' => 'August',
		'sep' => 'September',
		'oct' => 'October',
		'nov' => 'November',
		'dec' => 'December'
	);

	public static $weeks = array(
		'1st' => 'first',
		'2nd' => 'second',
		'3rd' => 'third',
		'4th' => 'fourth',
		'5th' => 'fifth'
	);
	public static $wdays = array(
		'sun' => 'Sunday',
		'mon' => 'Monday',
		'tue' => 'Tuesday',
		'wed' => 'Wednesday',
		'thu' => 'Thursday',
		'fri' => 'Friday',
		'sat' => 'Saturday'
	);

	public function getMonthsConfig(array $months_cfg) {
		$month = '';
		$month_count = count($months_cfg);
		$months = array_keys(Params::$months);
		if ($month_count < 12) {
			if ($month_count > 1) {
				$month_start = $months_cfg[0];
				$month_end = $months_cfg[$month_count-1];
				$month .= $months[$month_start] . '-' . $months[$month_end];
			} else {
				$month .= $months[$months_cfg[0]];
			}
		}
		return $month;
	}

	public function getWeeksConfig(array $weeks_cfg) {
		$week = '';
		$week_count = count($weeks_cfg);
		$weeks = array_keys(Params::$weeks);
		if ($week_count < 5) {
			if ($week_count > 1) {
				$week_start = $weeks_cfg[0];
				$week_end = $weeks_cfg[$week_count-1];
				$week .= $weeks[$week_start] . '-' . $weeks[$week_end];
			} else {
				$week .= $weeks[$weeks_cfg[0]];
			}
		}
		return $week;
	}

	public function getWdaysConfig(array $wdays_cfg) {
		$wday = '';
		$wday_count = count($wdays_cfg);
		$wdays = array_keys(Params::$wdays);
		if ($wday_count < 7) {
			if ($wday_count > 1) {
				$wday_start = $wdays_cfg[0];
				$wday_end = $wdays_cfg[$wday_count-1];
				$wday .= $wdays[$wday_start] . '-' . $wdays[$wday_end];
			} else {
				$wday .= $wdays[$wdays_cfg[0]];
			}
		}
		return $wday;
	}

	/**
	 * Get day value in config form.
	 *
	 * @param array $days_cfg days array (ex. array(0,1,2,3,4))
	 * @return string days config value
	 */
	public function getDaysConfig(array $days_cfg) {
		$days = '';
		if (count($days_cfg) < 31) {
			$days_map = array_map(array('Params', 'getDayByNo') , $days_cfg);
			$days = 'on ' . implode(',', $days_map);
		}
		return $days;
	}

	/**
	 * Simple method to prepare day config value by day number.
	 *
	 * @static
	 * @return integer single day config value
	 */
	private static function getDayByNo($day_no) {
		return ++$day_no;
	}

	/**
	 * Get Bacula config boolean value.
	 *
	 * @param boolean $value value
	 * @return string bacula config boolean value
	 */
	public static function getBoolValue($value) {
		return ($value ? 'yes' : 'no');
	}
}
?>
