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

Prado::using('Application.Common.Class.Params');
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveBoolean');
Prado::using('Application.Web.Portlets.DirectiveComboBox');
Prado::using('Application.Web.Portlets.DirectiveText');
Prado::using('Application.Web.Portlets.DirectiveTimePeriod');

class DirectiveSchedule extends DirectiveListTemplate {

	public $directives;

	private $directive_types = array(
		'DirectiveBoolean',
		'DirectiveText',
		'DirectiveComboBox',
		'DirectiveTimePeriod'
	);

	private $overwrite_directives = array(
		'Level',
		'Pool',
		'Storage',
		'Messages',
		'FullPool',
		'DifferentialPool',
		'IncrementalPool',
		'Accurate',
		'Priority',
		'SpoolData',
		'writepartafterjob',
		'MaxRunSchedTime',
		'NextPool'
	);

	private $time_values = array(
		'Minute',
		'Hour',
		'Day',
		'Month',
		'DayOfWeek',
		'WeekOfMonth',
		'WeekOfYear'
	);

	private $time_directives = array(
		'Month',
		'MonthRangeFrom',
		'MonthRangeTo',
		'Week',
		'WeekRangeFrom',
		'WeekRangeTo',
		'Day',
		'DayRangeFrom',
		'DayRangeTo',
		'Wday',
		'WdayRangeFrom',
		'WdayRangeTo',
		'TimeHourAt',
		'TimeMinAt',
		'TimeMinHourly'
	);

	public function loadConfig($sender, $param) {
		$load_values = $this->getLoadValues();
		$directives = $this->getData();
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resource_type = $this->getResourceType();
		$resource_name = $this->getResourceName();
		$data_desc = $this->Application->getModule('data_desc');
		$resource_desc = $data_desc->getDescription($this->getComponentType(), 'Job');
		for ($i = 0; $i < count($this->overwrite_directives); $i++) {
			$key = strtolower($this->overwrite_directives[$i]);
			$this->directives[$key] = array();
			$this->directives[$key]['directive_name'] = $this->overwrite_directives[$i];
			$default_value = null;
			$data = null;
			$resource = null;
			$directive_desc = null;
			$required = false;
			if (array_key_exists($this->overwrite_directives[$i], $resource_desc)) {
				$directive_desc = $resource_desc[$this->overwrite_directives[$i]];
			}
			if (is_object($directive_desc)) {
				if (property_exists($directive_desc, 'DefaultValue')) {
					$default_value = $directive_desc->DefaultValue;
				}
				if (property_exists($directive_desc, 'Data')) {
					$data = $directive_desc->Data;
				}
				if (property_exists($directive_desc, 'Resource')) {
					$resource = $directive_desc->Resource;
				}
			}
			if (preg_match('/^(Full|Incremental|Differential)Pool$/', $this->overwrite_directives[$i]) === 1) {
				$resource = 'Pool';
			}
			$in_config = false;
			if ($load_values === true) {
				$in_config = property_exists($directives, $this->overwrite_directives[$i]);
			}

			$directive_value = $in_config ? $directives->{$this->overwrite_directives[$i]} : null;
			$this->{$this->overwrite_directives[$i]}->setHost($host);
			$this->{$this->overwrite_directives[$i]}->setComponentType($component_type);
			$this->{$this->overwrite_directives[$i]}->setComponentName($component_name);
			$this->{$this->overwrite_directives[$i]}->setResourceType($resource_type);
			$this->{$this->overwrite_directives[$i]}->setResourceName($resource_name);
			$this->{$this->overwrite_directives[$i]}->setDirectiveName($this->overwrite_directives[$i]);
			$this->{$this->overwrite_directives[$i]}->setDirectiveValue($directive_value);
			$this->{$this->overwrite_directives[$i]}->setDefaultValue($default_value);
			$this->{$this->overwrite_directives[$i]}->setRequired($required);
			$this->{$this->overwrite_directives[$i]}->setData($data);
			$this->{$this->overwrite_directives[$i]}->setResource($resource);
			$this->{$this->overwrite_directives[$i]}->setLabel($this->overwrite_directives[$i]);
			$this->{$this->overwrite_directives[$i]}->setInConfig($in_config);
			$this->{$this->overwrite_directives[$i]}->setShow($in_config || $this->SourceTemplateControl->getShowAllDirectives());
			$this->{$this->overwrite_directives[$i]}->setResourceNames($this->getResourceNames());
			$this->{$this->overwrite_directives[$i]}->setParentName(__CLASS__);


		}

		for ($i = 0; $i < count($this->time_directives); $i++) {
			$this->{$this->time_directives[$i]}->setHost($host);
			$this->{$this->time_directives[$i]}->setComponentType($component_type);
			$this->{$this->time_directives[$i]}->setComponentName($component_name);
			$this->{$this->time_directives[$i]}->setResourceType($resource_type);
			$this->{$this->time_directives[$i]}->setResourceName($resource_name);
			$this->{$this->time_directives[$i]}->setDirectiveName($this->time_directives[$i]);
			$this->{$this->time_directives[$i]}->setDefaultValue(0);
			$this->{$this->time_directives[$i]}->setParentName(__CLASS__);
		}

		$months_long = array_values(Params::$months);

		$this->directives['month'] = array(
			'data' => Params::$months,
		);

		$single_months = Params::$months;
		$single_months['monthly'] = 'Monthly';
		$this->Month->setData($single_months);
		$this->MonthRangeFrom->setData(Params::$months);
		$this->MonthRangeTo->setData(Params::$months);

		$month_single = null;
		$month_range_from = null;
		$month_range_to = null;
		$month_count = $load_values ? count($directives->Month) : 0;
		if ($month_count === 12) {
			$month_single = 'Monthly';
		} elseif ($month_count == 1) {
			$month_single = $months_long[$directives->Month[0]];
			$this->MonthSingle->Checked = true;
		} elseif ($month_count > 0 && $month_count < 12) {
			$month_start = $directives->Month[0];
			$month_end = $directives->Month[$month_count-1];
			$month_range_from = $months_long[$month_start];
			$month_range_to = $months_long[$month_end];
			$this->MonthRange->Checked = true;
		}
		$this->Month->setDirectiveValue($month_single);
		$this->MonthRangeFrom->setDirectiveValue($month_range_from);
		$this->MonthRangeTo->setDirectiveValue($month_range_to);

		$days = range(1, 31);
		$single_days = $days;
		$single_days['daily'] = 'Daily';
		$this->Day->setData($single_days);
		$this->DayRangeFrom->setData($days);
		$this->DayRangeTo->setData($days);

		$day_single = null;
		$day_range_from = null;
		$day_range_to = null;
		$day_count = $load_values ? count($directives->Day) : 0;
		if ($day_count === 31) {
			$day_single = 'Daily';
		} elseif ($day_count === 1) {
			$day_single = $days[$directives->Day[0]];
			$this->DaySingle->Checked = true;
		} elseif ($day_count > 0 && $day_count < 31) {
			$day_start = $directives->Day[0];
			$day_end = $directives->Day[$day_count-1];
			$day_range_from = $day_start;
			$day_range_to = $day_end;
			$this->DayRange->Checked = true;
		}
		$this->Day->setDirectiveValue($day_single);
		$this->DayRangeFrom->setDirectiveValue($day_range_from);
		$this->DayRangeTo->setDirectiveValue($day_range_to);

		$weeks_long = array_values(Params::$weeks);

		$single_weeks = Params::$weeks;
		$single_weeks['weekly'] = 'Weekly';
		$this->Week->setData($single_weeks);
		$this->WeekRangeFrom->setData(Params::$weeks);
		$this->WeekRangeTo->setData(Params::$weeks);
		$week_single = null;
		$week_range_from = null;
		$week_range_to = null;
		$week_count = $load_values ? count($directives->WeekOfMonth) : 0;
		if ($week_count === 5) {
			$week_single = 'Weekly';
		} elseif ($week_count == 1) {
			$week_single = $weeks_long[$directives->WeekOfMonth[0]];
			$this->WeekSingle->Checked = true;
		} elseif ($week_count > 0 && $week_count < 5) {
			$week_start = $directives->WeekOfMonth[0];
			$week_end = $directives->WeekOfMonth[$week_count-1];
			$week_range_from = $weeks_long[$week_start];
			$week_range_to = $weeks_long[$week_end];
			$this->WeekRange->Checked = true;
		}
		$this->Week->setDirectiveValue($week_single);
		$this->WeekRangeFrom->setDirectiveValue($week_range_from);
		$this->WeekRangeTo->setDirectiveValue($week_range_to);

		$wdays_long = array_values(Params::$wdays);
		$this->Wday->setData(Params::$wdays);
		$this->WdayRangeFrom->setData(Params::$wdays);
		$this->WdayRangeTo->setData(Params::$wdays);

		$wday_single = null;
		$wday_range_from = null;
		$wday_range_to = null;
		$wday_count = $load_values ? count($directives->DayOfWeek) : 0;
		if ($wday_count === 7) {
			$wday_single = '';
		} elseif ($wday_count === 1) {
			$wday_single = $wdays_long[$directives->DayOfWeek[0]];
			$this->WdaySingle->Checked = true;
		} elseif ($wday_count > 0 && $wday_count < 7) {
			$wday_start = $directives->DayOfWeek[0];
			$wday_end = $directives->DayOfWeek[$wday_count-1];
			$wday_range_from = $wdays_long[$wday_start];
			$wday_range_to = $wdays_long[$wday_end];
			$this->WdayRange->Checked = true;
		}
		$this->Wday->setDirectiveValue($wday_single);
		$this->WdayRangeFrom->setDirectiveValue($wday_range_from);
		$this->WdayRangeTo->setDirectiveValue($wday_range_to);

		$hour = null;
		$minute = null;
		if ($load_values) {
			$hour = $directives->Hour[0]; // @TODO: Check for many hour values;
			/**
			 * Check if Minute property exists because of bug about missing Minute
			 * @see http://bugs.bacula.org/view.php?id=2318
			 */
			$minute = property_exists($directives, 'Minute') ? $directives->Minute : 0;
		}
		$this->directives['time'] = array(
			'hour' => $hour,
			'minute' => $minute
		);
		if ($load_values) {
			if (count($directives->Hour) == 24) {
				$this->TimeHourly->Checked = true;
				$this->TimeMinHourly->setDirectiveValue($minute);
			} elseif (count($directives->Hour) == 1) {
				$this->TimeAt->Checked = true;
				$this->TimeHourAt->setDirectiveValue($hour);
				$this->TimeMinAt->setDirectiveValue($minute);
			} else {
				$this->TimeDisable->Checked = true;
			}
		} else {
			$this->TimeDisable->Checked = true;
		}
	}

	public function getDirectiveValue() {
		$directive_values = array();
		$component_type = $this->getComponentType();
		$resource_type = $this->getResourceType();

		for ($i = 0; $i < count($this->directive_types); $i++) {
			$controls = $this->DirectiveContainer->findControlsByType($this->directive_types[$i]);
			for ($j = 0; $j < count($controls); $j++) {
				$directive_name = $controls[$j]->getDirectiveName();
				$directive_value = $controls[$j]->getDirectiveValue();
				$default_value = $controls[$j]->getDefaultValue();
				if (is_null($directive_value)) {
					continue;
				}
				if ($this->directive_types[$i] === 'DirectiveBoolean') {
					settype($default_value, 'bool');
				}

				if ($directive_value === $default_value) {
					// value the same as default value, skip it
					continue;
				}
				$directive_values[] = "{$directive_name}={$directive_value}";
			}
		}

		if ($this->MonthSingle->Checked === true) {
			$directive_values[] = $this->Month->getDirectiveValue();
		} elseif ($this->MonthRange->Checked === true) {
			$from = $this->MonthRangeFrom->getDirectiveValue();
			$to = $this->MonthRangeTo->getDirectiveValue();
			$directive_values[] = "{$from}-{$to}";
		}

		if ($this->WeekSingle->Checked === true) {
			$directive_values[] = $this->Week->getDirectiveValue();
		} elseif ($this->WeekRange->Checked === true) {
			$from = $this->WeekRangeFrom->getDirectiveValue();
			$to = $this->WeekRangeTo->getDirectiveValue();
			$directive_values[] = "{$from}-{$to}";
		}

		if ($this->DaySingle->Checked === true) {
			$directive_values[] = $this->Day->getDirectiveValue();
		} elseif ($this->DayRange->Checked === true) {
			$from = $this->DayRangeFrom->getDirectiveValue();
			$to = $this->DayRangeTo->getDirectiveValue();
			$directive_values[] = "{$from}-{$to}";
		}

		if ($this->WdaySingle->Checked === true) {
			$directive_values[] = $this->Wday->getDirectiveValue();
		} elseif ($this->WdayRange->Checked === true) {
			$from = $this->WdayRangeFrom->getDirectiveValue();
			$to = $this->WdayRangeTo->getDirectiveValue();
			$directive_values[] = "{$from}-{$to}";
		}

		if ($this->TimeAt->Checked === true) {
			$hour = $this->TimeHourAt->getDirectiveValue();
			$minute = sprintf('%02d', $this->TimeMinAt->getDirectiveValue());
			$directive_values[] = "at {$hour}:{$minute}";
		} elseif ($this->TimeHourly->Checked === true) {
			$hour = '00';
			$minute = sprintf('%02d', $this->TimeMinHourly->getDirectiveValue());
			$directive_values[] = "hourly at {$hour}:{$minute}";
		}

		$directive_value = array('Run' => implode(' ', $directive_values));
		return $directive_value;
	}
}
?>
