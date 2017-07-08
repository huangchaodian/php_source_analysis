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
#include "php_funtrace.h"
#define OPLINE EX(opline)
#define DCL_OPLINE
#define USE_OPLINE zend_op *opline = EX(opline);
#define LOAD_OPLINE()
#define SAVE_OPLINE()
#define LOAD_REGS()
#define OPLINE EX(opline)
/* If you declare any globals in php_funtrace.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(funtrace)

/* True global resources - no need for thread safety here */
static int le_funtrace;
static zval *fun_name;
static int enable;
static void (*old_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);
ZEND_API void funtrace_execute_ex (zend_execute_data *execute_data TSRMLS_DC)
{
	if(enable){
		zend_bool original_in_execution;
		original_in_execution = EG(in_execution);
		EG(in_execution) = 1;

		const char * function_name = get_active_function_name(TSRMLS_C);
		if(function_name){
			const char * file_name= zend_get_executed_filename(TSRMLS_C);
			const char * class_name= get_active_class_name(0 TSRMLS_C);

			int len = strlen(file_name) + strlen(class_name) + strlen(function_name);
			char * trace_str= (char*)emalloc(len+3);
			snprintf(trace_str, len+3, "%s#%s#%s", file_name, class_name, function_name);
	
			zval **l;
			if(zend_hash_find(Z_ARRVAL_P(fun_name),trace_str,len+3,(void **)&l)==SUCCESS){
				add_assoc_long(fun_name,trace_str,Z_LVAL_PP(l)+1);
			}else{
				add_assoc_long(fun_name,trace_str,1);
			}
			efree(trace_str);
		}

		EG(in_execution) = original_in_execution;
	}
	old_zend_execute_ex (execute_data TSRMLS_DC);
}
/* {{{ funtrace_functions[]
 *
 * Every user visible function must have an entry in funtrace_functions[].
 */
const zend_function_entry funtrace_functions[] = {
	PHP_FE(confirm_funtrace_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(funtrace_enable,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in funtrace_functions[] */
};
/* }}} */

/* {{{ funtrace_module_entry
*/
zend_module_entry funtrace_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"funtrace",
	funtrace_functions,
	PHP_MINIT(funtrace),
	PHP_MSHUTDOWN(funtrace),
	PHP_RINIT(funtrace),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(funtrace),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(funtrace),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_FUNTRACE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FUNTRACE
ZEND_GET_MODULE(funtrace)
#endif

	/* {{{ PHP_INI
	*/
	/* Remove comments and fill if you need to have entries in php.ini
	*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("funtrace.request_count",      "0", PHP_INI_ALL, OnUpdateLong, request_count, zend_funtrace_globals, funtrace_globals)
	STD_PHP_INI_ENTRY("funtrace.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_funtrace_globals, funtrace_globals)
PHP_INI_END()
	/* }}} */

	/* {{{ php_funtrace_init_globals
	*/
	/* Uncomment this function if you have INI entries
	*/
static void php_funtrace_init_globals(zend_funtrace_globals *funtrace_globals)
{
	funtrace_globals->request_count= 0;
	funtrace_globals->global_string = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(funtrace)
{
	/* If you have INI entries, uncomment these lines 
	*/
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(funtrace)
{
	/* uncomment this line if you have INI entries
	*/
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(funtrace)
{
	FUNTRACE_G(request_count)=FUNTRACE_G(request_count)+1;
	/*enable=1;*/
	enable=0;
	old_zend_execute_ex = zend_execute_ex;
	zend_execute_ex = funtrace_execute_ex;

	fun_name=(zval *)emalloc(sizeof(zval));
	array_init(fun_name);
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(funtrace)
{
	zend_execute_ex = old_zend_execute_ex ;
	efree(fun_name);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(funtrace)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "funtrace support", "enabled");
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
/* {{{ proto string confirm_funtrace_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_funtrace_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}
	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP. %d %d", "funtrace", arg,FUNTRACE_G(request_count),enable);
	RETURN_STRINGL(strg, len, 0);
}
PHP_FUNCTION(funtrace_enable){
	enable=1;
	RETURN_ZVAL(fun_name,0,0);
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
