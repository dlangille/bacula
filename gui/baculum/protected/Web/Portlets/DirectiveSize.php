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

class DirectiveSize extends DirectiveTemplate {

	const SIZE_FORMAT = 'SizeFormat';
	const DEFAULT_SIZE_FORMAT = 'byte';

	private $size_formats = array(
		array('format' => 'byte', 'value' => 1, 'label' => 'Bytes'),
		array('format' => 'kilobyte', 'value' => 1000, 'label' => 'Kilobytes'),
		array('format' => 'megabyte', 'value' => 1000, 'label' => 'Megabytes'),
		array('format' => 'gigabyte', 'value' => 1000, 'label' => 'Gigabytes'),
		array('format' => 'terabyte', 'value' => 1000, 'label' => 'Terabytes')
	);

	public function getValue() {
		$value = $this->Directive->Text;
		if (is_numeric($value)) {
			settype($value, 'integer');
			$size_format = $this->SizeFormat->SelectedValue;
			$value = $this->getValueBytes($value, $size_format);
		} else {
			$value = null;
		}
		return $value;
	}

	public function getSizeFormat() {
		return $this->getViewState(self::SIZE_FORMAT, self::DEFAULT_SIZE_FORMAT);
	}

	public function setSizeFormat($format) {
		$this->setViewState(self::SIZE_FORMAT, $format);
	}

	public function getSizeFormats() {
		$size_formats = array();
		for ($i = 0; $i < count($this->size_formats); $i++) {
			$format = array(
				'label' => Prado::localize($this->size_formats[$i]['label']),
				'format' => $this->size_formats[$i]['format']
			);
			array_push($size_formats, $format);
		}
		return $size_formats;
	}

	public function createDirective() {
		$size_format = $this->getSizeFormat();
		$directive_value = $this->getDirectiveValue();
		$default_value = $this->getDefaultValue();
		if ($this->getInConfig() === false && empty($directive_value)) {
			if ($default_value !== 0) {
				$directive_value = $default_value;
			} else {
				$directive_value = 0;
			}
		}
		$formatted_value = $this->formatSize($directive_value, $size_format);
		$this->Directive->Text = $formatted_value['value'];
		$this->SizeFormat->DataSource = $this->size_formats;
		$this->SizeFormat->SelectedValue = $formatted_value['format'];
		$this->SizeFormat->dataBind();
		$this->Label->Text = $this->getLabel();
		$validate = $this->getRequired();
		$this->DirectiveValidator->setVisible($validate);
	}

	/**
	 * Convert original size in bytes into given size format.
	 *
	 * Note, if there is not possible to convert size value into given format
	 * then there will be returned value converted by using as close format as possible.
	 * Example:
	 *  size_value: 121000
	 *  given format: byte
	 *  returned value: 121
	 *  returned format: kilobyte
	 */
	private function formatSize($size_bytes, $format) {
		$value = $size_bytes;
		for ($i = 0; $i < count($this->size_formats); $i++) {
			if ($this->size_formats[$i]['format'] != $format) {
				$remainder = $value % $this->size_formats[$i]['value'];
				if ($remainder === 0) {
					$value /= $this->size_formats[$i]['value'];
					$format = $this->size_formats[$i]['format'];
					continue;
				}
				break;
			}
			break;
		}
		$result = array('value' => $value, 'format' => $format);
		return $result;
	}

	private function getValueBytes($value, $size_format) {
		for ($i = 0; $i < count($this->size_formats); $i++) {
			$value *= $this->size_formats[$i]['value'];
			if ($this->size_formats[$i]['format'] === $size_format) {
				break;
			}
		}
		return $value;
	}
}
?>
