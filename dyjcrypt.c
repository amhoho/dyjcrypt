/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:duyunjiang                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_dyjcrypt.h"
#include "dyjcrypt\tool.h"

/* If you declare any globals in php_dyjcrypt.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(dyjcrypt)
*/

/* True global resources - no need for thread safety here */
static int le_dyjcrypt;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("dyjcrypt.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_dyjcrypt_globals, dyjcrypt_globals)
    STD_PHP_INI_ENTRY("dyjcrypt.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_dyjcrypt_globals, dyjcrypt_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_dyjcrypt_compiled(string arg)
   Return a string to confirm that the module is compiled in */

/**

加密函数

**/
PHP_FUNCTION(dyjencode)
{
	/*char *arg = NULL;
	size_t arg_len, len;*/
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &strg) == FAILURE) {
		return;
	}

	if (!memcmp(ZSTR_VAL(strg), dyj_header, sizeof(dyj_header))) { //相等返回0 已经加密就不再加密了
		RETURN_FALSE;
	}

	if (_dyj_checkSn() == FALSE) { //秘钥验证失败,无法加密工作
		php_printf("secret key error");
		RETURN_FALSE;
	}

	char *t = emalloc(sizeof(dyj_header) + ZSTR_LEN(strg)); //混淆头和数据流进来
	memcpy(t, dyj_header, sizeof(dyj_header));
	if (ZSTR_LEN(strg) > 0) {
		_dyj_encode(ZSTR_VAL(strg), ZSTR_LEN(strg));
		memcpy(t + sizeof(dyj_header), ZSTR_VAL(strg), ZSTR_LEN(strg)); //跳指针
	}
	

	RETURN_STR(zend_string_init(t, sizeof(dyj_header) + ZSTR_LEN(strg), 0));
}



/**

解密函数

**/
/*PHP_FUNCTION(dyjdecode)
{
zend_string *strg;

if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &strg) == FAILURE) {
return;
}

if (_dyj_checkSn() == FALSE) { //秘钥验证失败,无法解密工作
	php_printf("secret key error");
	RETURN_FALSE;
}

if (memcmp(ZSTR_VAL(strg), dyj_header, sizeof(dyj_header))) { //不相等返回1 没有加密就返回false
RETURN_FALSE;
}

size_t len = ZSTR_LEN(strg) - sizeof(dyj_header);
if (ZSTR_LEN(strg) > 0) {
_dyj_decode(ZSTR_VAL(strg) + sizeof(dyj_header), len); //跳指针
}

RETURN_STR(zend_string_init(ZSTR_VAL(strg) + sizeof(dyj_header), len, 0));
}*/

/*
获取特征码
*/
PHP_FUNCTION(dyjGetSnCode)
{
	char cpuCode[64];
	char *strg;
	//zend_string *strg;
	int len = 0;
    _dyj_getcpucode(cpuCode);
	//_dyj_checkSn(cpuCode);

	len = spprintf(&strg, 0, "%s", cpuCode);
	RETURN_STRINGL(strg, len, 0);

}


/*
获取特征码
*/
PHP_FUNCTION(testdebug_function)
{

	if (_dyj_checkSn() == TRUE) {
		php_printf("sn check Successful");
	}
	else {
		php_printf("sn check fail");
	}




}

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_dyjcrypt_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_dyjcrypt_init_globals(zend_dyjcrypt_globals *dyjcrypt_globals)
{
	dyjcrypt_globals->global_value = 0;
	dyjcrypt_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(dyjcrypt)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	old_compile_file = zend_compile_file;  //赋值给老文件编译指针
	zend_compile_file = new_compile_file; //最关键的一个代码，把编译文件动手脚了。
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(dyjcrypt)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	zend_compile_file = old_compile_file;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(dyjcrypt)
{
#if defined(COMPILE_DL_DYJCRYPT) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(dyjcrypt)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(dyjcrypt)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "dyjcrypt support", "enabled");
	php_info_print_table_row(2, "Version", "1.0 beta");
	php_info_print_table_row(2, "Author", "duyunjiang");
	php_info_print_table_row(2, "QQ", "1666263536");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ dyjcrypt_functions[]
 *
 * Every user visible function must have an entry in dyjcrypt_functions[].
 */
const zend_function_entry dyjcrypt_functions[] = {
	PHP_FE(dyjencode,	NULL)		/* For testing, remove later. */
	//PHP_FE(dyjdecode,	NULL)		/* For testing, remove later. */
	PHP_FE(dyjGetSnCode,NULL)
	PHP_FE(testdebug_function,NULL)
	PHP_FE_END	/* Must be the last line in dyjcrypt_functions[] */
};
/* }}} */

/* {{{ dyjcrypt_module_entry
 */
zend_module_entry dyjcrypt_module_entry = {
	STANDARD_MODULE_HEADER,
	"dyjcrypt",
	dyjcrypt_functions,
	PHP_MINIT(dyjcrypt),
	PHP_MSHUTDOWN(dyjcrypt),
	PHP_RINIT(dyjcrypt),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(dyjcrypt),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(dyjcrypt),
	PHP_DYJCRYPT_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_DYJCRYPT
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(dyjcrypt)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
