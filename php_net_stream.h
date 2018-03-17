/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
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

#ifndef PHP_NET_STREAM_H
#define PHP_NET_STREAM_H

extern zend_module_entry net_stream_module_entry;
#define phpext_net_stream_ptr &net_stream_module_entry

#ifdef PHP_WIN32
#define PHP_NET_STREAM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_NET_STREAM_API __attribute__ ((visibility("default")))
#else
#define PHP_NET_STREAM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif
#include "net_stream.h"

PHP_MINIT_FUNCTION(net_stream);
PHP_MSHUTDOWN_FUNCTION(net_stream);
PHP_MINFO_FUNCTION(net_stream);

PHP_FUNCTION(net_stream_get);
PHP_FUNCTION(net_stream_set);
PHP_FUNCTION(net_stream_pack);
PHP_FUNCTION(net_stream_unpack);
PHP_FUNCTION(net_stream_encode);
PHP_FUNCTION(net_stream_decode);

#define NET_STREAM_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(net_stream, v)

#if defined(ZTS) && defined(COMPILE_DL_NET_STREAM)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif  /* PHP_NET_STREAM_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
