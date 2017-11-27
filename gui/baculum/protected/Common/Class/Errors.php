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
 
class GenericError {
	const ERROR_NO_ERRORS = 0;
	const ERROR_INVALID_COMMAND = 1;
	const ERROR_INTERNAL_ERROR = 1000;

	const MSG_ERROR_NO_ERRORS = '';
	const MSG_ERROR_INVALID_COMMAND = 'Invalid command.';
	const MSG_ERROR_INTERNAL_ERROR = 'Internal error.';
}

class DatabaseError extends GenericError {
	const ERROR_DB_CONNECTION_PROBLEM = 2;
	const ERROR_DATABASE_ACCESS_NOT_SUPPORTED = 3;
	
	const MSG_ERROR_DB_CONNECTION_PROBLEM = 'Problem with connection to database.';
	const MSG_ERROR_DATABASE_ACCESS_NOT_SUPPORTED = 'Database access is not supported by this API instance.';
}

class BconsoleError extends GenericError {

	const ERROR_BCONSOLE_CONNECTION_PROBLEM = 4;
	const ERROR_INVALID_DIRECTOR = 5;

	const MSG_ERROR_BCONSOLE_CONNECTION_PROBLEM = 'Problem with connection to bconsole.';
	const MSG_ERROR_INVALID_DIRECTOR = 'Invalid director.';
}

class AuthorizationError extends GenericError {

	const ERROR_AUTHORIZATION_TO_API_PROBLEM = 6;
	const ERROR_ACCESS_ATTEMPT_TO_NOT_ALLOWED_RESOURCE = 7;

	const MSG_ERROR_AUTHORIZATION_TO_API_PROBLEM = 'Problem with authorization to Baculum API.';
	const MSG_ERROR_ACCESS_ATTEMPT_TO_NOT_ALLOWED_RESOURCE = 'Access attempt to not allowed resource. Permission denied.';
}

class ClientError extends GenericError {
	const ERROR_CLIENT_DOES_NOT_EXISTS = 10;

	const MSG_ERROR_CLIENT_DOES_NOT_EXISTS = 'Client with inputted clientid does not exist.';
}

class StorageError extends GenericError {
	const ERROR_STORAGE_DOES_NOT_EXISTS = 20;

	const MSG_ERROR_STORAGE_DOES_NOT_EXISTS = 'Storage with inputted storageid does not exist.';
}

class VolumeError extends GenericError {
	const ERROR_VOLUME_DOES_NOT_EXISTS = 30;

	const MSG_ERROR_VOLUME_DOES_NOT_EXISTS = 'Volume with inputted mediaid does not exist.';
}

class PoolError extends GenericError {
	const ERROR_POOL_DOES_NOT_EXISTS = 40;
	const ERROR_NO_VOLUMES_IN_POOL_TO_UPDATE = 41;

	const MSG_ERROR_POOL_DOES_NOT_EXISTS = 'Pool with inputted poolid does not exist.';
	const MSG_ERROR_NO_VOLUMES_IN_POOL_TO_UPDATE= 'Pool with inputted poolid does not contain any volume to update.';
}

class JobError extends GenericError {
	const ERROR_JOB_DOES_NOT_EXISTS = 50;
	const ERROR_INVALID_JOBLEVEL = 51;
	const ERROR_FILESET_DOES_NOT_EXISTS = 52;
	const ERROR_CLIENT_DOES_NOT_EXISTS = 53;
	const ERROR_STORAGE_DOES_NOT_EXISTS = 54;
	const ERROR_POOL_DOES_NOT_EXISTS = 55;
	const ERROR_INVALID_RPATH = 56;
	const ERROR_INVALID_WHERE_OPTION = 57;
	const ERROR_INVALID_REPLACE_OPTION = 58;

	const MSG_ERROR_JOB_DOES_NOT_EXISTS = 'Job with inputted jobid does not exist.';
	const MSG_ERROR_INVALID_JOBLEVEL = 'Inputted job level is invalid.';
	const MSG_ERROR_FILESET_DOES_NOT_EXISTS = 'FileSet resource does not exist.';
	const MSG_ERROR_CLIENT_DOES_NOT_EXISTS = 'Client does not exist.';
	const MSG_ERROR_STORAGE_DOES_NOT_EXISTS = 'Storage does not exist.';
	const MSG_ERROR_POOL_DOES_NOT_EXISTS = 'Pool does not exist.';
	const MSG_ERROR_INVALID_RPATH = 'Inputted rpath for restore is invalid. Proper format is b2[0-9]+.';
	const MSG_ERROR_INVALID_WHERE_OPTION = 'Inputted "where" option is invalid.';
	const MSG_ERROR_INVALID_REPLACE_OPTION = 'Inputted "replace" option is invalid.';
}

class FileSetError extends GenericError {
	const ERROR_FILESET_DOES_NOT_EXISTS = 60;

	const MSG_ERROR_FILESET_DOES_NOT_EXISTS = 'FileSet with inputted filesetid does not exist.';
}

class BVFSError extends GenericError {
	const ERROR_INVALID_JOBID_LIST = 70;
	const ERROR_INVALID_RPATH = 71;
	const ERROR_INVALID_RESTORE_PATH = 72;
	const ERROR_INVALID_FILEID_LIST = 73;
	const ERROR_INVALID_FILEINDEX_LIST = 74;
	const ERROR_INVALID_DIRID_LIST = 75;

	const MSG_ERROR_INVALID_JOBID_LIST = 'Invalid jobid list.';
	const MSG_ERROR_INVALID_RPATH = 'Inputted path for restore is invalid. Proper format is b2[0-9]+.';
	const MSG_ERROR_INVALID_RESTORE_PATH = 'Inputted BVFS path param is invalid.';
	const MSG_ERROR_INVALID_FILEID_LIST = 'Invalid fileid list.';
	const MSG_ERROR_INVALID_FILEINDEX_LIST = 'Invalid file index list.';
	const MSG_ERROR_INVALID_DIRID_LIST = 'Invalid dirid list.';
}

class JSONToolsError extends GenericError {

	const ERROR_JSON_TOOLS_DISABLED = 80;
	const ERROR_JSON_TOOLS_CONNECTION_PROBLEM = 81;
	const ERROR_JSON_TOOLS_WRONG_EXITCODE = 82;
	const ERROR_JSON_TOOLS_UNABLE_TO_PARSE_OUTPUT = 83;
	const ERROR_JSON_TOOL_NOT_CONFIGURED = 84;


	const MSG_ERROR_JSON_TOOLS_DISABLED = 'JSON tools support is disabled.';
	const MSG_ERROR_JSON_TOOLS_CONNECTION_PROBLEM = 'Problem with connection to a JSON tool.';
	const MSG_ERROR_JSON_TOOLS_WRONG_EXITCODE = 'JSON tool returned wrong exitcode.';
	const MSG_ERROR_JSON_TOOLS_UNABLE_TO_PARSE_OUTPUT = 'JSON tool output was unable to parse.';
	const MSG_ERROR_JSON_TOOL_NOT_CONFIGURED = 'JSON tool not configured.';
}

class BaculaConfigError extends GenericError {

	const ERROR_CONFIG_DIR_NOT_WRITABLE = 90;
	const ERROR_UNEXPECTED_BACULA_CONFIG_VALUE = 91;
	const ERROR_CONFIG_NO_JSONTOOL_READY = 92;
	const ERROR_WRITE_TO_CONFIG_ERROR = 93;
	const ERROR_CONFIG_VALIDATION_ERROR = 94;

	const MSG_ERROR_CONFIG_DIR_NOT_WRITABLE = 'Config directory is not writable.';
	const MSG_ERROR_UNEXPECTED_BACULA_CONFIG_VALUE = 'Unexpected Bacula config value.';
	const MSG_ERROR_CONFIG_NO_JSONTOOL_READY = 'No JSON tool ready.';
	const MSG_ERROR_WRITE_TO_CONFIG_ERROR = 'Write to config file error.';
	const MSG_ERROR_CONFIG_VALIDATION_ERROR = 'Config validation error.';
}

class ConnectionError extends GenericError {

	const ERROR_CONNECTION_TO_HOST_PROBLEM = 100;

	const MSG_ERROR_CONNECTION_TO_HOST_PROBLEM = 'Problem with connection to remote host.';
}
?>
