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

Prado::using('Application.Common.Class.Params');
Prado::using('Application.Web.Portlets.DirectiveListTemplate');
Prado::using('Application.Web.Portlets.DirectiveCheckBox');
Prado::using('Application.Web.Portlets.DirectiveComboBox');
Prado::using('Application.Web.Portlets.DirectiveTextBox');
Prado::using('Application.Web.Portlets.DirectiveTimePeriod');

/**
 * Schedule directive control.
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 * @category Control
 * @package Baculum Web
 */
class DirectiveSchedule extends DirectiveListTemplate {

	private $directive_types = array(
		'DirectiveCheckBox',
		'DirectiveTextBox',
		'DirectiveComboBox',
		'DirectiveTimePeriod'
	);

	private $overwrite_directives = array(
		'Pool',
		'FullPool',
		'IncrementalPool',
		'DifferentialPool',
		'Level',
		'Storage',
		'Messages',
		'Priority',
		'SpoolData',
		'MaxRunSchedTime',
		'Accurate',
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

	public function loadConfig() {
		$load_values = $this->getLoadValues();
		$directives = $this->getData();
		$host = $this->getHost();
		$component_type = $this->getComponentType();
		$component_name = $this->getComponentName();
		$resource_type = $this->getResourceType();
		$resource_name = $this->getResourceName();
		$data_desc = $this->Application->getModule('data_desc');
		$resource_desc = $data_desc->getDescription($this->getComponentType(), 'Job');
		$overwrite_directives = $time_directives = $directive_values = array();
		foreach ($directives as $index => $directive) {
			for ($i = 0; $i < count($this->overwrite_directives); $i++) {
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
					$in_config = property_exists($directive, $this->overwrite_directives[$i]);
				}

				$directive_value = $in_config ? $directive->{$this->overwrite_directives[$i]} : null;
				$overwrite_directives[$this->overwrite_directives[$i]] = array(
					'host' => $host,
					'component_type' => $component_type,
					'component_name' => $component_name,
					'resource_type' => $resource_type,
					'resource_name' => $resource_name,
					'directive_name' => $this->overwrite_directives[$i],
					'directive_value' => $directive_value,
					'default_value' => $default_value,
					'required' => $required,
					'data' => $data,
					'resource' => $resource,
					'label' => $this->overwrite_directives[$i],
					'in_config' => $in_config,
					'show' => $in_config || !$load_values || $this->SourceTemplateControl->getShowAllDirectives(),
					'resource_names' => $this->getResourceNames(),
					'parent_name' => __CLASS__,
					'group_name' => $index
				);
			}

			for ($i = 0; $i < count($this->time_directives); $i++) {
				$time_directives[$this->time_directives[$i]] = array(
					'host' => $host,
					'component_type' => $component_type,
					'component_name' => $component_name,
					'resource_type' => $resource_type,
					'resource_name' => $resource_name,
					'directive_name' => $this->time_directives[$i],
					'directive_values' => $directive,
					'default_value' => 0,
					'parent_name' => __CLASS__,
					'group_name' => $index
				);
			}
			$directive_values[] = array(
				'overwrite_directives' => $overwrite_directives,
				'time_directives' => $time_directives
			);
		}
		$this->RepeaterScheduleRuns->DataSource = $directive_values;
		$this->RepeaterScheduleRuns->dataBind();
	}

	public function createRunItem($sender, $param) {
		$load_values = $this->getLoadValues();
		for ($i = 0; $i < count($this->overwrite_directives); $i++) {
			$control = $param->Item->{$this->overwrite_directives[$i]};
			if (is_object($control)) {
				$data = $param->Item->Data['overwrite_directives'][$this->overwrite_directives[$i]];
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
				$control->setResourceNames($data['resource_names']);
				$control->setParentName($data['parent_name']);
				$control->onLoad(null);
				$control->createDirective();
			}
		}

		for ($i = 0; $i < count($this->time_directives); $i++) {
			$control = $param->Item->{$this->time_directives[$i]};
			if (is_object($control)) {
				$data = $param->Item->Data['time_directives'][$this->time_directives[$i]];
				$control->setHost($data['host']);
				$control->setComponentType($data['component_type']);
				$control->setComponentName($data['component_name']);
				$control->setResourceType($data['resource_type']);
				$control->setResourceName($data['resource_name']);
				$control->setDirectiveName($data['directive_name']);
				$control->setDefaultValue($data['default_value']);
				$control->setParentName($data['parent_name']);
			}
		}

		$directive = $param->Item->Data['time_directives']['Month']['directive_values'];

		$months = array_keys(Params::$months);

		$param->Item->Month->setData(Params::$months);
		$param->Item->MonthRangeFrom->setData(Params::$months);
		$param->Item->MonthRangeTo->setData(Params::$months);

		$month_single = null;
		$month_range_from = null;
		$month_range_to = null;
		$month_count = $load_values ? count($directive->Month) : 0;
		if ($month_count === 12) {
			$param->Item->MonthDisable->Checked = true;
		} elseif ($month_count == 1) {
			$month_single = $months[$directive->Month[0]];
			$param->Item->MonthSingle->Checked = true;
		} elseif ($month_count > 0 && $month_count < 12) {
			$month_start = $directive->Month[0];
			$month_end = $directive->Month[$month_count-1];
			$month_range_from = $months[$month_start];
			$month_range_to = $months[$month_end];
			$param->Item->MonthRange->Checked = true;
		}
		$param->Item->Month->setDirectiveValue($month_single);
		$param->Item->MonthRangeFrom->setDirectiveValue($month_range_from);
		$param->Item->MonthRangeTo->setDirectiveValue($month_range_to);

		$days = range(1, 31);
		$param->Item->Day->setData($days);
		$param->Item->DayRangeFrom->setData($days);
		$param->Item->DayRangeTo->setData($days);

		$day_single = null;
		$day_range_from = null;
		$day_range_to = null;
		$day_count = $load_values ? count($directive->Day) : 0;
		if ($day_count === 31) {
			$param->Item->DayDisable->Checked = true;
		} elseif ($day_count === 1) {
			$day_single = $days[$directive->Day[0]];
			$param->Item->DaySingle->Checked = true;
		} elseif ($day_count > 0 && $day_count < 31) {
			$day_start = $directive->Day[0];
			$day_end = $directive->Day[$day_count-1];
			$day_range_from = $days[$day_start];
			$day_range_to = $days[$day_end];
			$param->Item->DayRange->Checked = true;
		}
		$param->Item->Day->setDirectiveValue($day_single);
		$param->Item->DayRangeFrom->setDirectiveValue($day_range_from);
		$param->Item->DayRangeTo->setDirectiveValue($day_range_to);

		$weeks = array_keys(Params::$weeks);

		$param->Item->Week->setData(Params::$weeks);
		$param->Item->WeekRangeFrom->setData(Params::$weeks);
		$param->Item->WeekRangeTo->setData(Params::$weeks);
		$week_single = null;
		$week_range_from = null;
		$week_range_to = null;
		$week_count = $load_values ? count($directive->WeekOfMonth) : 0;
		if ($week_count == 6) {
			$param->Item->WeekDisable->Checked = true;
		} elseif ($week_count == 1) {
			$week_single = $weeks[$directive->WeekOfMonth[0]];
			$param->Item->WeekSingle->Checked = true;
		} elseif ($week_count > 0 && $week_count < 6) {
			$week_start = $directive->WeekOfMonth[0];
			$week_end = $directive->WeekOfMonth[$week_count-1];
			$week_range_from = $weeks[$week_start];
			$week_range_to = $weeks[$week_end];
			$param->Item->WeekRange->Checked = true;
		}
		$param->Item->Week->setDirectiveValue($week_single);
		$param->Item->WeekRangeFrom->setDirectiveValue($week_range_from);
		$param->Item->WeekRangeTo->setDirectiveValue($week_range_to);

		$wdays = array_keys(Params::$wdays);
		$param->Item->Wday->setData(Params::$wdays);
		$param->Item->WdayRangeFrom->setData(Params::$wdays);
		$param->Item->WdayRangeTo->setData(Params::$wdays);

		$wday_single = null;
		$wday_range_from = null;
		$wday_range_to = null;
		$wday_count = $load_values ? count($directive->DayOfWeek) : 0;
		if ($wday_count === 7) {
			$wday_single = '';
		} elseif ($wday_count === 1) {
			$wday_single = $wdays[$directive->DayOfWeek[0]];
			$param->Item->WdaySingle->Checked = true;
		} elseif ($wday_count > 0 && $wday_count < 7) {
			$wday_start = $directive->DayOfWeek[0];
			$wday_end = $directive->DayOfWeek[$wday_count-1];
			$wday_range_from = $wdays[$wday_start];
			$wday_range_to = $wdays[$wday_end];
			$param->Item->WdayRange->Checked = true;
		}
		$param->Item->Wday->setDirectiveValue($wday_single);
		$param->Item->WdayRangeFrom->setDirectiveValue($wday_range_from);
		$param->Item->WdayRangeTo->setDirectiveValue($wday_range_to);

		$hour = null;
		$minute = null;
		if ($load_values) {
			$hour = $directive->Hour[0]; // @TODO: Check for many hour values;
			/**
			 * Check if Minute property exists because of bug about missing Minute
			 * @see http://bugs.bacula.org/view.php?id=2318
			 */
			$minute = property_exists($directive, 'Minute') ? $directive->Minute : 0;
		}
		$param->Item->TimeHourAt->setDirectiveValue(0);
		$param->Item->TimeMinAt->setDirectiveValue(0);
		$param->Item->TimeMinHourly->setDirectiveValue(0);
		if ($load_values) {
			if (count($directive->Hour) == 24) {
				if ($minute === 0) {
					$param->Item->TimeDisable->Checked = true;
				} else {
					$param->Item->TimeHourly->Checked = true;
					$param->Item->TimeMinHourly->setDirectiveValue($minute);
				}
			} elseif (count($directive->Hour) == 1) {
				$param->Item->TimeAt->Checked = true;
				$param->Item->TimeHourAt->setDirectiveValue($hour);
				$param->Item->TimeMinAt->setDirectiveValue($minute);
			} else {
				$param->Item->TimeDisable->Checked = true;
			}
		} else {
			$param->Item->TimeDisable->Checked = true;
		}

		// @TODO: Fix controls to avoid forcing onLoad() and createDirective()
		for ($i = 0; $i < count($this->time_directives); $i++) {
			$control = $param->Item->{$this->time_directives[$i]};
			$control->onLoad(null);
			$control->createDirective();
		}
	}

	public function removeSchedule($sender, $param) {
		if ($param instanceof Prado\Web\UI\TCommandEventParameter) {
			$idx = $param->getCommandName();
			$data = $this->getDirectiveValue(true);
			array_splice($data, $idx, 1);
			$this->setData($data);
			$this->loadConfig();
		}
	}

	public function getDirectiveValue($ret_obj = false) {
		$directive_values = array();
		$values = array('Run' => array());
		$component_type = $this->getComponentType();
		$resource_type = $this->getResourceType();
		$objs = array();

		$ctrls = $this->RepeaterScheduleRuns->getItems();
		foreach ($ctrls as $value) {
			$obj = new StdClass;
			for ($i = 0; $i < count($this->overwrite_directives); $i++) {
				$control = $value->{$this->overwrite_directives[$i]};
				$directive_name = $control->getDirectiveName();
				$directive_value = $control->getDirectiveValue();
				$default_value = $control->getDefaultValue();
				if (is_null($directive_value)) {
					continue;
				}
				if (get_class($control) === 'DirectiveCheckBox') {
					settype($default_value, 'bool');
				}

				if ($directive_value === $default_value) {
					// value the same as default value, skip it
					continue;
				}
				$obj->{$directive_name} = $directive_value;
				if (get_class($control) === 'DirectiveCheckBox') {
					$directive_value = Params::getBoolValue($directive_value);
				}
				$directive_values[] = "{$directive_name}=\"{$directive_value}\"";
			}

			$obj->Month = range(0, 11);
			$months = array_keys(Params::$months);
			if ($value->MonthSingle->Checked === true) {
				$month_val = $value->Month->getDirectiveValue();
				$directive_values[] = $month_val;
				$obj->Month = array(array_search($month_val, $months));
			} elseif ($value->MonthRange->Checked === true) {
				$from = $value->MonthRangeFrom->getDirectiveValue();
				$to = $value->MonthRangeTo->getDirectiveValue();
				$directive_values[] = "{$from}-{$to}";
				$f = array_search($from, $months);
				$t = array_search($to, $months);
				$obj->Month = range($f, $t);
			}

			$obj->WeekOfMonth = range(0, 5);
			$weeks = array_keys(Params::$weeks);
			if ($value->WeekSingle->Checked === true) {
				$week_val = $value->Week->getDirectiveValue();
				$directive_values[] = $week_val;
				$obj->WeekOfMonth = array(array_search($week_val, $weeks));
			} elseif ($value->WeekRange->Checked === true) {
				$from = $value->WeekRangeFrom->getDirectiveValue();
				$to = $value->WeekRangeTo->getDirectiveValue();
				$directive_values[] = "{$from}-{$to}";
				$f = array_search($from, $weeks);
				$t = array_search($to, $weeks);
				$obj->WeekOfMonth = range($f, $t);
			}

			$obj->Day = range(0, 30);
			if ($value->DaySingle->Checked === true) {
				$day = $value->Day->getDirectiveValue();
				$directive_values[] = 'on ' . $day;
				$obj->Day = array($day-1);
			} elseif ($value->DayRange->Checked === true) {
				$from = $value->DayRangeFrom->getDirectiveValue()-1;
				$to = $value->DayRangeTo->getDirectiveValue()-1;
				$day_range = range($from, $to);
				$directive_values[] = 'on ' . Params::getDaysConfig($day_range);
				$obj->Day = $day_range;
			}

			$obj->DayOfWeek = range(0, 6);
			$wdays = array_keys(Params::$wdays);
			if ($value->WdaySingle->Checked === true) {
				$directive_values[] = $value->Wday->getDirectiveValue();
				$obj->DayOfWeek = array(array_search($value->Wday->getDirectiveValue(), $wdays));
			} elseif ($value->WdayRange->Checked === true) {
				$from = $value->WdayRangeFrom->getDirectiveValue();
				$to = $value->WdayRangeTo->getDirectiveValue();
				$directive_values[] = "{$from}-{$to}";
				$f = array_search($from, $wdays);
				$t = array_search($to, $wdays);
				$obj->DayOfWeek = range($f, $t);
			}

			$obj->Hour = range(0, 23);
			$obj->Minute = 0;
			if ($value->TimeAt->Checked === true) {
				$hour = $value->TimeHourAt->getDirectiveValue();
				$minute = sprintf('%02d', $value->TimeMinAt->getDirectiveValue());
				$directive_values[] = "at {$hour}:{$minute}";
				$obj->Hour = array($hour);
				$obj->Minute = $minute;

			} elseif ($value->TimeHourly->Checked === true) {
				$hour = '00';
				$minute = sprintf('%02d', $value->TimeMinHourly->getDirectiveValue());
				$directive_values[] = "hourly at {$hour}:{$minute}";
				$obj->Hour = range(0, 23);
				$obj->Minute = $value->TimeMinHourly->getDirectiveValue();
			}
			$values['Run'][] = implode(' ', $directive_values);
			$objs[] = $obj;
			$directive_values = array();
			$obj = null;
		}
		return (($ret_obj) ? $objs : $values);
	}

	public function newScheduleDirective() {
		$data = $this->getDirectiveValue(true);
		$obj = new StdClass;
		$obj->Hour = range(0, 23);
		$obj->Minute = 0;
		$obj->Day = range(0, 30);
		$obj->Month = range(0, 11);
		$obj->DayOfWeek = range(0, 6);
		$obj->WeekOfMonth = range(0, 5);
		$obj->WeekOfYear = range(0, 53);

		if (is_array($data)) {
			$data[] = $obj;
		} else {
			$data = array($obj);
		}
		$this->setData($data);
		$this->SourceTemplateControl->setShowAllDirectives(true);
		$this->loadConfig();
	}
}
?>
