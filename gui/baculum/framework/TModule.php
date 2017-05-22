<?php
/**
 * TModule class file.
 *
 * @author Qiang Xue <qiang.xue@gmail.com>
 * @link https://github.com/pradosoft/prado
 * @copyright Copyright &copy; 2005-2016 The PRADO Group
 * @license https://github.com/pradosoft/prado/blob/master/COPYRIGHT
 * @package System
 */

/**
 * TModule class.
 *
 * TModule implements the basic methods required by IModule and may be
 * used as the basic class for application modules.
 *
 * @author Qiang Xue <qiang.xue@gmail.com>
 * @package System
 * @since 3.0
 */
abstract class TModule extends TApplicationComponent implements IModule
{
	/**
	 * @var string module id
	 */
	private $_id;

	/**
	 * Initializes the module.
	 * This method is required by IModule and is invoked by application.
	 * @param TXmlElement module configuration
	 */
	public function init($config)
	{
	}

	/**
	 * @return string id of this module
	 */
	public function getID()
	{
		return $this->_id;
	}

	/**
	 * @param string id of this module
	 */
	public function setID($value)
	{
		$this->_id=$value;
	}
}

