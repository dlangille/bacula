<?php
/*
 * Bacula(R) - The Network Backup Solution
 * Baculum   - Bacula web interface
 *
 * Copyright (C) 2013-2016 Marcin Haba
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

/**
 * Base pages module.
 * The module contains methods that are common for all pages (wizards, main
 * page and error pages).
 *
 * @author Marcin Haba <marcin.haba@bacula.pl>
 */
class BaculumPage extends TPage {

	public function onPreInit($param) {
		parent::onPreInit($param);
		$this->Application->getGlobalization()->Culture = $this->getLanguage();
		$this->setURLPrefixForSubdir();
	}

	/**
	 * Get curently set language short name (for example: en, pl).
	 * If language short name is not set in session then the language value
	 * is taken from Baculum config file, saved in session and returned.
	 * If the language setting is set in session, then the value from
	 * session is returned.
	 *
	 * @access public
	 * @return string currently set language short name
	 */
	public function getLanguage() {
		if (isset($_SESSION['language']) && !empty($_SESSION['language'])) {
			$language =  $_SESSION['language'];
		} else {
			$language = $this->getModule('configuration')->getLanguage();
			$_SESSION['language'] = $language;
		}
		return $language;
	}

	/**
	 * Shortcut method for getting application modules instances by
	 * module name.
	 *
	 * @access public
	 * @param string $name application module name
	 * @return object module class instance
	 */
	public function getModule($name) {
		return $this->Application->getModule($name);
	}

	/**
	 * Redirection to a page.
	 * Page name is given in PRADO notation with "dot", for example: (Home.SomePage).
	 *
	 * @access public
	 * @param string $page_name page name to redirect
	 * @param array $params HTTP GET method parameters in associative array
         * @return none
	 */
	public function goToPage($page_name, $params = null) {
		$url = $this->Service->constructUrl($page_name, $params, false);
		$this->Response->redirect($url);
	}

	/**
	 * Redirection to default page defined in application config.
	 *
	 * @access public
	 * @param array $params HTTP GET method parameters in associative array
	 * @return none
	 */
	public function goToDefaultPage($params = null) {
		$this->goToPage($this->Service->DefaultPage, $params);
	}

	/**
	 * Set prefix when Baculum is running in document root subdirectory.
	 * For example:
	 *   web server document root: /var/www/
	 *   Baculum directory /var/www/baculum/
	 *   URL prefix: /baculum/
	 * In this case to base url is added '/baculum/' such as:
	 * http://localhost:9095/baculum/
	 *
	 * @access private
	 * @return none
	 */
	private function setURLPrefixForSubdir() {
		$full_document_root = preg_replace('#(\/)$#', '', $this->getFullDocumentRoot());
		$url_prefix = str_replace($full_document_root, '', APPLICATION_DIRECTORY);
		if (!empty($url_prefix)) {
			$this->Application->getModule('friendly-url')->setUrlPrefix($url_prefix);
		}
	}

	/**
	 * Get full document root directory path.
	 * Symbolic links in document root path are translated to full paths.
	 *
	 * @access private
	 * return string full document root directory path
	 */
	private function getFullDocumentRoot() {
		$root_dir = array();
		$dirs = explode('/', $_SERVER['DOCUMENT_ROOT']);
		for($i = 0; $i < count($dirs); $i++) {
			$document_root_part =  implode('/', $root_dir) . '/' . $dirs[$i];
			if (is_link($document_root_part)) {
				$root_dir = array(readlink($document_root_part));
			} else {
				$root_dir[] = $dirs[$i];
			}
		}

		$root_dir = implode('/', $root_dir);
		return $root_dir;
	}
}
?>
