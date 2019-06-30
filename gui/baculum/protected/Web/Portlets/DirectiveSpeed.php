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
Prado::using('System.Web.UI.ActiveControls.TActiveTextBox');
Prado::using('System.Web.UI.ActiveControls.TActiveDropDownList');
Prado::using('Application.Web.Portlets.DirectiveTemplate');

class DirectiveSpeed extends DirectiveTemplate {

	const SPEED_FORMAT = 'SpeedFormat';
	const DEFAULT_SPEED_FORMAT = '';

	private $speed_formats = array(
		array('format' => '', 'value' => 1, 'label' => 'B/s'),
		array('format' => 'kb/s', 'value' => 1000, 'label' => 'kB/s'),
		array('format' => 'k/s', 'value' => 1024, 'label' => 'KiB/s'),
		array('format' => 'mb/s', 'value' => 1000000, 'label' => 'MB/s'),
		array('format' => 'm/s', 'value' => 1048576, 'label' => 'MiB/s')
	);

	public function getValue() {
		$value = $this->Directive->Text;
		if (is_numeric($value)) {
			settype($value, 'integer');
			$speed_format = $this->SpeedFormat->SelectedValue;
			$value = $this->getValueBytes($value, $speed_format);
		} else {
			$value = null;
		}
		return $value;
	}

	public function getSpeedFormat() {
		return $this->getViewState(self::SPEED_FORMAT, self::DEFAULT_SPEED_FORMAT);
	}

	public function setSpeedFormat($format) {
		$this->setViewState(self::SPEED_FORMAT, $format);
	}

	public function getSpeedFormats() {
		$speed_formats = array();
		for ($i = 0; $i < count($this->speed_formats); $i++) {
			$speed_formats[$this->speed_formats[$i]['format']] = $this->speed_formats[$i]['label'];
		}
		return $speed_formats;
	}

	public function createDirective() {
		$speed_format = $this->getSpeedFormat();
		$directive_value = $this->getDirectiveValue();
		$default_value = $this->getDefaultValue();
		if ($this->getInConfig() === false && empty($directive_value)) {
			if ($default_value !== 0) {
				$directive_value = $default_value;
			} else {
				$directive_value = 0;
			}
		}
		$formatted_value = $this->formatSpeed($directive_value, $speed_format);
		$this->Directive->Text = $formatted_value['value'];
		$this->SpeedFormat->DataSource = $this->getSpeedFormats();
		$this->SpeedFormat->dataBind();
		$this->SpeedFormat->SelectedValue = $formatted_value['format'];
		$this->Label->Text = $this->getLabel();
		$validate = $this->getRequired();
		$this->DirectiveValidator->setVisible($validate);
	}

	/**
	 * Convert original speed in bytes into given speed format.
	 *
	 * Note, if there is not possible to convert speed value into given format
	 * then there will be returned value converted by using as close format as possible.
	 * Example:
	 *  speed value: 121000
	 *  given format: ''
	 *  returned value: 121
	 *  returned format: 'kb/s'
	 */
	private function formatSpeed($speed_bytes, $format) {
		$value = $speed_bytes;
		if ($value > 0) {
			for ($i = (count($this->speed_formats) - 1); $i >= 0; $i--) {
				if ($this->speed_formats[$i]['format'] != $format) {
					$remainder = $value % $this->speed_formats[$i]['value'];
					if ($remainder == 0) {
						$value /= $this->speed_formats[$i]['value'];
						$format = $this->speed_formats[$i]['format'];
						break;
					}
				}
			}
		}
		return array('value' => $value, 'format' => $format);
	}

	private function getValueBytes($value, $speed_format) {
		for ($i = 0; $i < count($this->speed_formats); $i++) {
			if ($this->speed_formats[$i]['format'] === $speed_format) {
				$value *= $this->speed_formats[$i]['value'];
				break;
			}
		}
		return $value;
	}
}
?>
