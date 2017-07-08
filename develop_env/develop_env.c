/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_develop_env.h"
/*#include "Zend/zend_stream.h"*/

/* If you declare any globals in php_develop_env.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(develop_env)
*/

/* True global resources - no need for thread safety here */
static int le_develop_env;
ZEND_API zend_op_array *(*old_zend_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);
ZEND_API zend_op_array * develop_env_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC){
	zval **dev_map;
	if(zend_hash_find(&EG(symbol_table),"zend_develop_env_map",21,(void **)&dev_map)==SUCCESS){
		const char * fn=file_handle->filename;
		zval **zstr;
		if(zend_hash_find(Z_ARRVAL_PP(dev_map),fn,strlen(fn)+1,(void **)&zstr)==SUCCESS){
			
			zend_file_handle file_handle_map;
			zend_op_array *retval;
			char *opened_path = NULL;
			file_handle_map.filename = Z_STRVAL_PP(zstr);
			file_handle_map.free_filename = 0;
			file_handle_map.type = ZEND_HANDLE_FILENAME;
			file_handle_map.opened_path = NULL;
			file_handle_map.handle.fp = NULL;
			retval = old_zend_compile_file(&file_handle_map, type TSRMLS_CC);
			if (retval && file_handle_map.handle.stream.handle) {
				int dummy = 1;

				if (!file_handle_map.opened_path) {
					file_handle_map.opened_path = opened_path = estrndup((*zstr)->value.str.val, (*zstr)->value.str.len);
				}

				zend_hash_add(&EG(included_files), file_handle_map.opened_path, strlen(file_handle_map.opened_path)+1, (void *)&dummy, sizeof(int), NULL);

				if (opened_path) {
					efree(opened_path);
				}
			}
			zend_destroy_file_handle(&file_handle_map TSRMLS_CC);
			return retval;

		}
	}
	return old_zend_compile_file(file_handle,type TSRMLS_CC);
}
/* {{{ develop_env_functions[]
 *
 * Every user visible function must have an entry in develop_env_functions[].
 */
const zend_function_entry develop_env_functions[] = {
	PHP_FE(confirm_develop_env_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in develop_env_functions[] */
};
/* }}} */

/* {{{ develop_env_module_entry
 */
zend_module_entry develop_env_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"develop_env",
	develop_env_functions,
	PHP_MINIT(develop_env),
	PHP_MSHUTDOWN(develop_env),
	PHP_RINIT(develop_env),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(develop_env),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(develop_env),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_DEVELOP_ENV_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_DEVELOP_ENV
ZEND_GET_MODULE(develop_env)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("develop_env.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_develop_env_globals, develop_env_globals)
    STD_PHP_INI_ENTRY("develop_env.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_develop_env_globals, develop_env_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_develop_env_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_develop_env_init_globals(zend_develop_env_globals *develop_env_globals)
{
	develop_env_globals->global_value = 0;
	develop_env_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(develop_env)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(develop_env)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(develop_env)
{
	old_zend_compile_file=zend_compile_file;
	zend_compile_file=develop_env_compile_file;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(develop_env)
{
	zend_compile_file=old_zend_compile_file;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(develop_env)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "develop_env support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_develop_env_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_develop_env_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "develop_env", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
