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

class BConditional extends TTemplateControl implements IActiveControl {

	const BCONDITION = 'BCondition';
	const TYPE_TPL_FALSE = 0;
	const TYPE_TPL_TRUE = 1;

	private $item_true_template;
	private $item_false_template;

	public function __construct() {
		parent::__construct();
		$this->setAdapter(new TActiveControlAdapter($this));
	}

	public function getActiveControl() {
		return $this->getAdapter()->getBaseActiveControl();
	}

	public function onLoad($param) {
		$this->prepareControlContent();
		parent::onLoad($param);
	}

	public function bubbleEvent($sender, $param) {
		if ($param instanceof Prado\Web\UI\TCommandEventParameter) {
			$this->raiseBubbleEvent($this, $param);
			return true;
		} else {
			return false;
		}
	}

	private function createItemInternal($item_type) {
		$item = $this->createItem($item_type);
		if (!is_null($item)) {
			$this->getControls()->add($item);
		}
		return $item;
	}

	protected function createItem($item_type) {
		$template = null;
		$item = null;
		switch ($item_type) {
			case self::TYPE_TPL_TRUE: {
				$template = $this->getTrueTemplate();
				break;
			}
			case self::TYPE_TPL_FALSE: {
				$template = $this->getFalseTemplate();
				break;
			}
		}
		if (!is_null($template)) {
			$item = new BConditionalItem;
			$item->setItemType($item_type);
			$item->setTemplate($template);
			$item->setTemplateControl($this);
			$item->setData($this->getTemplateControl());
		}
		return $item;
	}

	public function getTrueTemplate() {
		return $this->item_true_template;
	}

	public function setTrueTemplate($template) {
		if ($template instanceof Prado\Web\UI\ITemplate) {
			$this->item_true_template = $template;
		}
	}

	public function getFalseTemplate() {
		return $this->item_false_template;
	}

	public function setFalseTemplate($template) {
		if ($template instanceof Prado\Web\UI\ITemplate) {
			$this->item_false_template = $template;
		}
	}

	public function setBCondition($value) {
		settype($value, 'bool');
		$this->setViewState(self::BCONDITION, $value);
	}

	public function getBCondition() {
		return $this->getViewState(self::BCONDITION);
	}

	public function dataBind() {
		$this->dataBindProperties();
		$this->prepareControlContent();
	}

	public function prepareControlContent() {
		if ($this->getBCondition() === true) {
			$this->createItemInternal(self::TYPE_TPL_TRUE);
		} else {
			$this->createItemInternal(self::TYPE_TPL_FALSE);
		}
	}
}

class BConditionalItem extends TTemplateControl implements IDataRenderer, INamingContainer {
	private $item_type;
	private $data;

	public function getItemType() {
		return $this->item_type;
	}

	public function setItemType($type) {
		$this->item_type = $type;
	}

	public function getData() {
		return $this->data;
	}

	public function setData($data) {
		$this->data = $data;
	}

	public function bubbleEvent($sender,$param) {
		if ($param instanceof Prado\Web\UI\TCommandEventParameter) {
			$this->raiseBubbleEvent($this, $param);
			return true;
		} else {
			return false;
		}
	}
}
?>
