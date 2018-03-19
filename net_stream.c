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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_net_stream.h"
#include <stdint.h>
#include <string.h>
#include <zlib.h>

static int le_net_stream;

PHP_MINFO_FUNCTION(net_stream)
{
  php_info_print_table_start();
  php_info_print_table_header(2, "net_stream support", "enabled");
  php_info_print_table_row(2, "version", PHP_NET_STREAM_VERSION);
  php_info_print_table_row(2, "release date", PHP_NET_STREAM_RELEASE_DATE);
  php_info_print_table_end();
}

PHP_MINIT_FUNCTION(net_stream)
{
  REGISTER_LONG_CONSTANT("NET_STREAM_VALUE", 0, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_CURSOR", 1, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_COMPRESSED", NET_STREAM_COMPRESSED, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_OUTBUF_SIZE", NET_STREAM_OUTBUF_SIZE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_INT_8", NET_STREAM_FORMAT_INT_8, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_INT_16", NET_STREAM_FORMAT_INT_16, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_INT_32", NET_STREAM_FORMAT_INT_32, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_INT_64", NET_STREAM_FORMAT_INT_64, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_UINT_8", NET_STREAM_FORMAT_UINT_8, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_UINT_16", NET_STREAM_FORMAT_UINT_16, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_UINT_32", NET_STREAM_FORMAT_UINT_32, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_FLOAT", NET_STREAM_FORMAT_FLOAT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_DOUBLE", NET_STREAM_FORMAT_DOUBLE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_STRING", NET_STREAM_FORMAT_STRING, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("NET_STREAM_ARRAY", NET_STREAM_FORMAT_ARRAY, CONST_CS | CONST_PERSISTENT);
  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(net_stream)
{
  return SUCCESS;
}

const zend_function_entry net_stream_functions[] = {
  PHP_FE(net_stream_get, NULL)
  PHP_FE(net_stream_set, NULL)
  PHP_FE(net_stream_pack, NULL)
  PHP_FE(net_stream_unpack, NULL)
  PHP_FE(net_stream_encode, NULL)
  PHP_FE(net_stream_decode, NULL)
#ifdef PHP_FE_END
  PHP_FE_END
#else
  {NULL,NULL,NULL}
};

zend_module_entry net_stream_module_entry = {
  STANDARD_MODULE_HEADER,
  "net_stream",
  net_stream_functions,
  PHP_MINIT(net_stream),
  PHP_MSHUTDOWN(net_stream),
  NULL,
  NULL,
  PHP_MINFO(net_stream),
  PHP_NET_STREAM_VERSION,
  STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_NET_STREAM
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(net_stream)
#endif

char* php_net_stream_zcodec(size_t* data_len, char* inbuf, size_t inbuf_len, char* outbuf, size_t outbuf_size, size_t mode)
{
  z_stream z;
  size_t count, status, total_count = 0;
  char* data = (char*)emalloc(outbuf_size);

  z.zalloc = Z_NULL;
  z.zfree = Z_NULL;
  z.opaque = Z_NULL;
  z.next_in = Z_NULL;
  z.avail_in = 0;
  if (mode)
    inflateInit(&z);
  else
    deflateInit(&z, 1);
  z.next_out = (Bytef*)outbuf;
  z.avail_out = outbuf_size;
  z.next_in = (Bytef*)inbuf;
  z.avail_in = inbuf_len;

  while (1)
  {
    status = mode ? inflate(&z, Z_NO_FLUSH) : deflate(&z, Z_FINISH);
    if (Z_STREAM_END == status)
      break;
    if (Z_OK != status)
    {
      if (mode)
        inflateEnd(&z);
      else
        deflateEnd(&z);
      *data_len = 0;
      return data;
    }
    if (!z.avail_out)
    {
      data = (char*)erealloc(data, total_count + outbuf_size);
      memcpy(data + total_count, outbuf, outbuf_size);
      total_count += outbuf_size;
      z.next_out = (Bytef*)outbuf;
      z.avail_out = outbuf_size;
    }
  }
  if (count = outbuf_size - z.avail_out)
  {
    data = (char*)erealloc(data, total_count + outbuf_size);
    memcpy(data + total_count, outbuf, count);
    total_count += count;
  }
  if (mode)
    inflateEnd(&z);
  else
    deflateEnd(&z);
  *data_len = total_count;
  return data;
}

void php_net_stream_extract(INTERNAL_FUNCTION_PARAMETERS, int8_t mode)
{
  HashTable* arr;
  zval* obfuscator = NULL;
#if defined(ZEND_ENGINE_3)
  zval* entry;
#elif defined(ZEND_ENGINE_2)
  zval** entry;
#endif
  char *data, *raw, *outbuf, *key;
  size_t i, data_len, raw_len, key_len = 0;
  size_t outbuf_size = NET_STREAM_OUTBUF_SIZE;
  zend_bool flag = 0;

  if (2 > ZEND_NUM_ARGS()) WRONG_PARAM_COUNT;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa!|bl", &raw, &raw_len, &obfuscator, &flag, &outbuf_size) == FAILURE)
    RETURN_NULL();

  if (obfuscator)
  {
    arr = Z_ARRVAL_P(obfuscator);
    key = (int8_t*)emalloc(sizeof(int8_t)*zend_hash_num_elements(arr));
#if defined(ZEND_ENGINE_3)
    ZEND_HASH_FOREACH_VAL(arr, entry) {
      if (Z_TYPE_P(entry) == IS_LONG)
        key[key_len++] = (int8_t)Z_LVAL_P(entry);
    } ZEND_HASH_FOREACH_END();
#elif defined(ZEND_ENGINE_2)
    for (i = zend_hash_num_elements(arr); 0 < i; --i)
    {
      zend_hash_get_current_data(arr, (void**)&entry);
      if (Z_TYPE_PP(entry) == IS_LONG)
        key[key_len++] = (int8_t)Z_LVAL_PP(entry);
    }
#endif
  }

  if (NET_STREAM_COMPRESSED != flag)
  {
    data_len = raw_len;
    data = (char*)emalloc(data_len);
    if (obfuscator)
    {
      for (i = 0; i < data_len; ++i)
        data[i] = key[(data_len - i) % key_len] ^ (~(raw[i]));
    }
    else
    {
      memcpy(data, raw, raw_len);
    }
  }
  else if (mode)
  {
    if (obfuscator)
    {
      for (i = 0; i < raw_len; ++i)
        raw[i] = key[(raw_len - i) % key_len] ^ (~(raw[i]));
    }
    outbuf = (char*)emalloc(outbuf_size);
    data = php_net_stream_zcodec(&data_len, raw, raw_len, outbuf, outbuf_size, NET_STREAM_Z_INFLATE);
    efree(outbuf);
  }
  else
  {
    outbuf = (char*)emalloc(outbuf_size);
    data = php_net_stream_zcodec(&data_len, raw, raw_len, outbuf, outbuf_size, NET_STREAM_Z_DEINFLATE);
    efree(outbuf);
    if (obfuscator)
    {
      for (i = 0; i < data_len; ++i)
        data[i] = key[(data_len - i) % key_len] ^ (~(data[i]));
    }
  }
  if (obfuscator)
    efree(key);

#if defined(ZEND_ENGINE_3)
  ZVAL_STRINGL(return_value, data, data_len);
#elif defined(ZEND_ENGINE_2)
  ZVAL_STRINGL(return_value, data, data_len, 1);
#endif
  efree(data);
}

static void php_net_stream_set_int8(char* buf, size_t buf_len, size_t* index, zval* option)
{
  if (buf_len < *index + sizeof(int8_t))
  {
    *index = buf_len;
    return;
  }
  *(buf + (*index)++) = (int8_t)zval_get_long(option);
}

static void php_net_stream_get_int8(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  int8_t value;
  if (*index + sizeof(int8_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  value = (int8_t)data[(*index)++];
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
}

static void php_net_stream_get_uint8(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  uint8_t value;
  if (*index + sizeof(uint8_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  value = (uint8_t)data[(*index)++];
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
}

static void php_net_stream_set_int16(char* buf, size_t buf_len, size_t* index, zval* option)
{
  int16_t value;
  char* ptr = buf + *index;
#ifdef ZEND_ENGINE_2
  zval tmp = *option;
#endif
  if (buf_len < *index + sizeof(int16_t))
  {
    *index = buf_len;
    return;
  }
#if defined(ZEND_ENGINE_3)
  value = (int16_t)zval_get_long(option);
#elif defined(ZEND_ENGINE_2)
  zval_copy_ctor(&tmp);
  convert_to_long(&tmp);
  value = (int16_t)Z_LVAL(tmp);
  zval_dtor(&tmp);
#endif
  *ptr++ = (int8_t)((value >> 8) & 0xff);
  *ptr = (int8_t)(value & 0xff);
  *index += sizeof(int16_t);
}

static void php_net_stream_get_int16(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  int16_t value;
  const char* net_value = data + *index;
  if (*index + sizeof(int16_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(int16_t);
  value = (int16_t)((((uint16_t)net_value[0] & 0xff) << 8) + (uint8_t)net_value[1]);
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
}

static void php_net_stream_get_uint16(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  uint16_t value;
  const char* net_value = data + *index;
  if (*index + sizeof(uint16_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(uint16_t);
  value = (uint16_t)((((uint16_t)net_value[0] & 0xff) << 8) + (uint8_t)net_value[1]);
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
}

static void php_net_stream_set_int32(char* buf, size_t buf_len, size_t* index, zval* option)
{
  int32_t value;
  char* ptr = buf + *index;
#ifdef ZEND_ENGINE_2
  zval tmp = *option;
#endif
  if (buf_len < *index + sizeof(int32_t))
  {
    *index = buf_len;
    return;
  }
#if defined(ZEND_ENGINE_3)
  value = (int32_t)zval_get_long(option);
#elif defined(ZEND_ENGINE_2)
  zval_copy_ctor(&tmp);
  convert_to_long(&tmp);
  value = (int32_t)Z_LVAL(tmp);
  zval_dtor(&tmp);
#endif
  *ptr++ = (int8_t)((value >> 24) & 0xff);
  *ptr++ = (int8_t)((value >> 16) & 0xff);
  *ptr++ = (int8_t)((value >> 8) & 0xff);
  *ptr = (int8_t)(value & 0xff);
  *index += sizeof(int32_t);
}

static void php_net_stream_get_int32(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  int32_t value;
  const char* net_value = data + *index;
  if (*index + sizeof(int32_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(int32_t);
  value = (int32_t)((((uint32_t)net_value[0] & 0xff) << 24) + (((uint32_t)net_value[1] & 0xff) << 16) + (((uint16_t)net_value[2] & 0xff) << 8) + (uint8_t)net_value[3]);
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
}

static void php_net_stream_get_uint32(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  uint32_t value;
  const char* net_value = data + *index;
  if (*index + sizeof(uint32_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(uint32_t);
  value = (uint32_t)((((uint32_t)net_value[0] & 0xff) << 24) + (((uint32_t)net_value[1] & 0xff) << 16) + (((uint16_t)net_value[2] & 0xff) << 8) + (uint8_t)net_value[3]);
#if SIZEOF_ZEND_LONG == 4
  if (2147483647 < value)
  {
    if (NULL == name)
      add_next_index_double(z_value, (double)value);
    else
      add_assoc_double_ex(z_value, name, name_len, (double)value);
    return;
  }
#endif
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
}

static void php_net_stream_set_int64(char* buf, size_t buf_len, size_t* index, zval* option)
{
  int64_t value;
  char* ptr = buf + *index;
#ifdef ZEND_ENGINE_2
  zval tmp = *option;
#endif
  if (buf_len < *index + sizeof(int64_t))
  {
    *index = buf_len;
    return;
  }

#if SIZEOF_ZEND_LONG == 4
#if defined(ZEND_ENGINE_3)
  value = (int64_t)zval_get_double(option);
#elif defined(ZEND_ENGINE_2)
  zval_copy_ctor(&tmp);
  convert_to_double(&tmp);
  value = (int64_t)Z_DVAL(tmp);
  zval_dtor(&tmp);
#endif
#else
#if defined(ZEND_ENGINE_3)
  value = (int64_t)zval_get_long(option);
#elif defined(ZEND_ENGINE_2)
  zval_copy_ctor(&tmp);
  convert_to_long(&tmp);
  value = (int64_t)Z_LVAL(tmp);
  zval_dtor(&tmp);
#endif
#endif
  *ptr++ = (int8_t)((value >> 56) & 0xff);
  *ptr++ = (int8_t)((value >> 48) & 0xff);
  *ptr++ = (int8_t)((value >> 40) & 0xff);
  *ptr++ = (int8_t)((value >> 32) & 0xff);
  *ptr++ = (int8_t)((value >> 24) & 0xff);
  *ptr++ = (int8_t)((value >> 16) & 0xff);
  *ptr++ = (int8_t)((value >> 8) & 0xff);
  *ptr = (int8_t)(value & 0xff);
  *index += sizeof(int64_t);
}

static void php_net_stream_get_int64(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  int64_t value;
  const char* net_value = data + *index;
  if (*index + sizeof(int64_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(int64_t);
  value = (int64_t)((((uint64_t)net_value[0] & 0xff) << 56) + (((uint64_t)net_value[1] & 0xff) << 48) + (((uint64_t)net_value[2] & 0xff) << 40) + (((uint64_t)net_value[3] & 0xff) << 32) +
                    (((uint32_t)net_value[4] & 0xff) << 24) + (((uint32_t)net_value[5] & 0xff) << 16) + (((uint16_t)net_value[6] & 0xff) << 8) + (uint8_t)net_value[7]);
#if SIZEOF_ZEND_LONG == 4
  if (2147483647 < value || -2147483647 > value)
  {
    if (NULL == name)
      add_next_index_double(z_value, (double)value);
    else
      add_assoc_double_ex(z_value, name, name_len, (double)value);
    return;
  }
  if (NULL == name)
    add_next_index_long(z_value, (int32_t)value);
  else
    add_assoc_long_ex(z_value, name, name_len, (int32_t)value);
#else
  if (NULL == name)
    add_next_index_long(z_value, value);
  else
    add_assoc_long_ex(z_value, name, name_len, value);
#endif
}

static void php_net_stream_set_float(char* buf, size_t buf_len, size_t* index, zval* option)
{
  union {
    float f;
    int32_t i;
  } d;
  char* ptr = buf + *index;
#ifdef ZEND_ENGINE_2
  zval tmp = *option;
#endif
  if (buf_len < *index + sizeof(float))
  {
    *index = buf_len;
    return;
  }
#if defined(ZEND_ENGINE_3)
  d.f = (float)zval_get_double(option);
#elif defined(ZEND_ENGINE_2)
  zval_copy_ctor(&tmp);
  convert_to_double(&tmp);
  d.f = (float)Z_DVAL(tmp);
  zval_dtor(&tmp);
#endif
  *ptr++ = (int8_t)((d.i >> 24) & 0xff);
  *ptr++ = (int8_t)((d.i >> 16) & 0xff);
  *ptr++ = (int8_t)((d.i >> 8) & 0xff);
  *ptr = (int8_t)(d.i & 0xff);
  *index += sizeof(float);
}

static void php_net_stream_get_float(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  const char* net_value = data + *index;
  union {
    float f;
    int32_t i;
  } d;
  if (*index + sizeof(float) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(float);
  d.i = (int32_t)((((uint32_t)net_value[0] & 0xff) << 24) + (((uint32_t)net_value[1] & 0xff) << 16) + (((uint16_t)net_value[2] & 0xff) << 8) + (uint8_t)net_value[3]);
  if (NULL == name)
    add_next_index_double(z_value, d.f);
  else
    add_assoc_double_ex(z_value, name, name_len, d.f);
}

static void php_net_stream_set_double(char* buf, size_t buf_len, size_t* index, zval* option)
{
  union {
    double f;
    int64_t i;
  } d;
  char* ptr = buf + *index;
#ifdef ZEND_ENGINE_2
  zval tmp = *option;
#endif
  if (buf_len < *index + sizeof(double))
  {
    *index = buf_len;
    return;
  }
#if defined(ZEND_ENGINE_3)
  d.f = (double)zval_get_double(option);
#elif defined(ZEND_ENGINE_2)
  zval_copy_ctor(&tmp);
  convert_to_double(&tmp);
  d.f = (double)Z_DVAL(tmp);
  zval_dtor(&tmp);
#endif
  *ptr++ = (int8_t)((d.i >> 56) & 0xff);
  *ptr++ = (int8_t)((d.i >> 48) & 0xff);
  *ptr++ = (int8_t)((d.i >> 40) & 0xff);
  *ptr++ = (int8_t)((d.i >> 32) & 0xff);
  *ptr++ = (int8_t)((d.i >> 24) & 0xff);
  *ptr++ = (int8_t)((d.i >> 16) & 0xff);
  *ptr++ = (int8_t)((d.i >> 8) & 0xff);
  *ptr = (int8_t)(d.i & 0xff);
  *index += sizeof(double);
}

static void php_net_stream_get_double(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  const char* net_value = data + *index;
  union {
    double f;
    int64_t i;
  } d;
  if (*index + sizeof(double) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  *index += sizeof(double);
  d.i = (int64_t)((((uint64_t)net_value[0] & 0xff) << 56) + (((uint64_t)net_value[1] & 0xff) << 48) + (((uint64_t)net_value[2] & 0xff) << 40) + (((uint64_t)net_value[3] & 0xff) << 32) +
                  (((uint32_t)net_value[4] & 0xff) << 24) + (((uint32_t)net_value[5] & 0xff) << 16) + (((uint16_t)net_value[6] & 0xff) << 8) + (uint8_t)net_value[7]);
  if (NULL == name)
    add_next_index_double(z_value, d.f);
  else
    add_assoc_double_ex(z_value, name, name_len, d.f);
}

static void php_net_stream_set_string(char* buf, size_t buf_len, size_t* index, zval* option)
{
  size_t size;
  char* ptr = buf + *index;
#if defined(ZEND_ENGINE_3)
  zend_string* str;
  str = zval_get_string(option);
  size = ZSTR_LEN(str);
#elif defined(ZEND_ENGINE_2)
  zval tmp = *option;
  zval_copy_ctor(&tmp);
  convert_to_string(&tmp);
  size = Z_STRLEN(tmp);
#endif
  if (0x7f < size)
  {
    if (buf_len < *index + sizeof(int16_t) + size)
    {
      *index = buf_len;
#if defined(ZEND_ENGINE_3)
      zend_string_release(str);
#elif defined(ZEND_ENGINE_2)
      zval_dtor(&tmp);
#endif
      return;
    }
    *ptr++ = (int8_t)(((size >> 8) & 0x7f) | 0x80);
    *ptr = (int8_t)(size & 0xff);
    *index += sizeof(int16_t);
  }
  else
  {
    if (buf_len < *index + sizeof(int8_t) + size)
    {
      *index = buf_len;
#if defined(ZEND_ENGINE_3)
      zend_string_release(str);
#elif defined(ZEND_ENGINE_2)
      zval_dtor(&tmp);
#endif
      return;
    }
    *ptr = (uint8_t)size;
    ++(*index);
  }
#if defined(ZEND_ENGINE_3)
  if (size) memcpy(buf + *index, ZSTR_VAL(str), size);
  zend_string_release(str);
#elif defined(ZEND_ENGINE_2)
  if (size) memcpy(buf + *index, Z_STRVAL(tmp), size);
  zval_dtor(&tmp);
#endif
  *index += size;
}

static void php_net_stream_get_string(zval* z_value, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  size_t size;
  if (*index + sizeof(int8_t) > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
    return;
  }
  size = *((uint8_t*)(data + (*index)++));
  if (0x7f < size)
  {
    if (*index + sizeof(int8_t) > data_len)
    {
      if (NULL == name)
        add_next_index_null(z_value);
      else
        add_assoc_null_ex(z_value, name, name_len);
      *index = data_len;
      return;
    }
    size = (size & 0x7f) << 8;
    size += *((uint8_t*)(data + (*index)++));
  }
  if (!size || *index + size > data_len)
  {
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
    *index = data_len;
     return;
   }
#if defined(ZEND_ENGINE_3)
  if (NULL == name)
    add_next_index_stringl(z_value, (char*)data + *index, size);
  else
    add_assoc_stringl_ex(z_value, name, name_len, (char*)data + *index, size);
#elif defined(ZEND_ENGINE_2)
  if (NULL == name)
    add_next_index_stringl(z_value, (char*)data + *index, size, 1);
  else
    add_assoc_stringl_ex(z_value, name, name_len, (char*)data + *index, size, 1);
#endif
  *index += size;
}

static void php_net_stream_set_value(char* buf, size_t buf_len, size_t* index, zval* option, size_t format_type)
{
  switch (format_type)
  {
  case NET_STREAM_FORMAT_INT_8:
  case NET_STREAM_FORMAT_UINT_8:
    php_net_stream_set_int8(buf, buf_len, index, option);
    break;
  case NET_STREAM_FORMAT_INT_16:
  case NET_STREAM_FORMAT_UINT_16:
    php_net_stream_set_int16(buf, buf_len, index, option);
    break;
  case NET_STREAM_FORMAT_INT_32:
  case NET_STREAM_FORMAT_UINT_32:
    php_net_stream_set_int32(buf, buf_len, index, option);
    break;
  case NET_STREAM_FORMAT_INT_64:
    php_net_stream_set_int64(buf, buf_len, index, option);
    break;
  case NET_STREAM_FORMAT_FLOAT:
    php_net_stream_set_float(buf, buf_len, index, option);
    break;
  case NET_STREAM_FORMAT_DOUBLE:
    php_net_stream_set_double(buf, buf_len, index, option);
    break;
  case NET_STREAM_FORMAT_STRING:
    php_net_stream_set_string(buf, buf_len, index, option);
    break;
  default:
    zend_error(E_WARNING, NET_STREAM_LOG_2);
  }
}

static void php_net_stream_get_value(zval* z_value, size_t format_type, size_t* index, const char* data, size_t data_len, const char* name, size_t name_len)
{
  switch (format_type)
  {
  case NET_STREAM_FORMAT_INT_8:
    php_net_stream_get_int8(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_INT_16:
    php_net_stream_get_int16(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_INT_32:
    php_net_stream_get_int32(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_INT_64:
    php_net_stream_get_int64(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_UINT_8:
    php_net_stream_get_uint8(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_UINT_16:
    php_net_stream_get_uint16(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_UINT_32:
    php_net_stream_get_uint32(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_FLOAT:
    php_net_stream_get_float(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_DOUBLE:
    php_net_stream_get_double(z_value, index, data, data_len, name, name_len);
    break;
  case NET_STREAM_FORMAT_STRING:
    php_net_stream_get_string(z_value, index, data, data_len, name, name_len);
    break;
  default:
    if (NULL == name)
      add_next_index_null(z_value);
    else
      add_assoc_null_ex(z_value, name, name_len);
  }
}

static size_t php_net_stream_get_format_type(char ch)
{
  switch (ch)
  {
  case NET_STREAM_FORMAT_CHAR_INT_8:
    return NET_STREAM_FORMAT_INT_8;
  case NET_STREAM_FORMAT_CHAR_INT_16:
    return NET_STREAM_FORMAT_INT_16;
  case NET_STREAM_FORMAT_CHAR_INT_32:
    return NET_STREAM_FORMAT_INT_32;
  case NET_STREAM_FORMAT_CHAR_INT_64:
    return NET_STREAM_FORMAT_INT_64;
  case NET_STREAM_FORMAT_CHAR_UINT_8:
    return NET_STREAM_FORMAT_UINT_8;
  case NET_STREAM_FORMAT_CHAR_UINT_16:
    return NET_STREAM_FORMAT_UINT_16;
  case NET_STREAM_FORMAT_CHAR_UINT_32:
    return NET_STREAM_FORMAT_UINT_32;
  case NET_STREAM_FORMAT_CHAR_FLOAT:
    return NET_STREAM_FORMAT_FLOAT;
  case NET_STREAM_FORMAT_CHAR_DOUBLE:
    return NET_STREAM_FORMAT_DOUBLE;
  case NET_STREAM_FORMAT_CHAR_STRING:
    return NET_STREAM_FORMAT_STRING;
  default:
    return NET_STREAM_FORMAT_NONE;
  }
}

static void php_net_stream_invalid_key_name(const char* name, size_t name_len)
{
  char* str = (char*)emalloc(name_len + 1);
  memcpy(str, name, name_len);
  str[name_len] = '\0';
  zend_error(E_WARNING, NET_STREAM_LOG_3, str);
  efree(str);
}

static int8_t php_net_stream_set_array(net_stream_packet_t* pkt, zval* parameter, int8_t is_key_array TSRMLS_DC)
{
  zval* option;
#ifdef ZEND_ENGINE_2
  zval** tmpzval = NULL;
#endif
  HashTable *tuple, *arr = HASH_OF(parameter);
  char ch, *name, *ptr = pkt->data + pkt->index;
  size_t i, j, num, size, name_len, format_start, format_count = 0, format_type = NET_STREAM_FORMAT_NONE;
  int8_t is_0k = 0;

  ch = pkt->data_format[pkt->format_index++];

  if ('0' == ch)
  {
    if (!is_key_array)
      return -1;
    is_0k = 1;
    num = 1;
  }
  else
  {
    num = arr->nNumOfElements;
    if (0x7f < num)
    {
      if (pkt->data_len < pkt->index + sizeof(int16_t))
      {
        pkt->index = pkt->data_len;
        return -1;
      }
      *ptr++ = (int8_t)(((num >> 8) & 0x7f) | 0x80);
      *ptr = (int8_t)(num & 0xff);
      pkt->index += sizeof(int16_t);
    }
    else
    {
      if (pkt->data_len < pkt->index + sizeof(int8_t))
      {
        pkt->index = pkt->data_len;
        return -1;
      }
      *ptr = num;
      ++pkt->index;
    }
  }

  if (!is_0k && ('1' > ch || '9' < ch))
  {
    if (is_key_array)
      return -1;
    format_type = php_net_stream_get_format_type(ch);
    if (NET_STREAM_FORMAT_NONE == format_type)
      return -1;
    for (i = 0; i < num; ++i)
    {
#if defined(ZEND_ENGINE_3)
      if ((option = zend_hash_index_find(arr, i)) == NULL)
      {
        zend_error(E_WARNING, NET_STREAM_LOG_4, i);
        return -1;
      }
#elif defined(ZEND_ENGINE_2)
      if (zend_hash_index_find(arr, i, (void**)&tmpzval) != SUCCESS)
      {
        zend_error(E_WARNING, NET_STREAM_LOG_4, i);
        return -1;
      }
      option = *tmpzval;
      tmpzval = NULL;
#endif
      php_net_stream_set_value(pkt->data, pkt->data_len, &pkt->index, option, format_type);
    }
    return 0;
  }

  size = ch - '0';
  if (pkt->format_index < pkt->format_len && '0' <= pkt->data_format[pkt->format_index] && '9' >= pkt->data_format[pkt->format_index])
  {
    size *= 10;
    size += pkt->data_format[pkt->format_index++] - '0';
    if (pkt->format_index < pkt->format_len && '0' <= pkt->data_format[pkt->format_index] && '9' >= pkt->data_format[pkt->format_index])
    {
      size *= 10;
      size += pkt->data_format[pkt->format_index++] - '0';
    }
  }

  format_start = pkt->format_index;
  ptr = pkt->key_head;

  for (i = 0; i < num; ++i)
  {
    if (is_0k)
    {
      tuple = arr;
    }
    else
    {
#if defined(ZEND_ENGINE_3)
      if ((option = zend_hash_index_find(arr, i)) == NULL)
      {
        zend_error(E_WARNING, NET_STREAM_LOG_4, i);
        return -1;
      }
#elif defined(ZEND_ENGINE_2)
      if (zend_hash_index_find(arr, i, (void**)&tmpzval) != SUCCESS)
      {
        zend_error(E_WARNING, NET_STREAM_LOG_4, i);
        return -1;
      }
      option = *tmpzval;
      tmpzval = NULL;
#endif
      tuple = HASH_OF(option);
    }

    pkt->key_head = pkt->key_tail = ptr;
    pkt->format_index = format_start;
    j = 0;

    while (j < size)
    {
      if (is_key_array)
      {
        if (pkt->key_tail > pkt->key_end)
          break;
        while (pkt->key_tail != pkt->key_end && '/' != *pkt->key_tail)
        {
          if (++pkt->key_tail > pkt->key_end)
            break;
          continue;
        }
        name_len = pkt->key_tail - pkt->key_head;
        name = pkt->key_head;
        pkt->key_head = pkt->key_tail + 1;
        ++pkt->key_tail;
        if (!name_len) continue;

        if (pkt->data_len <= pkt->index || pkt->format_index >= pkt->format_len)
        {
          zend_error(E_WARNING, NET_STREAM_LOG_1);
          return -1;
        }

#if defined(ZEND_ENGINE_3)
        if ((option = zend_hash_str_find(tuple, name, name_len)) == NULL)
        {
          php_net_stream_invalid_key_name(name, name_len);
          return -1;
        }
#elif defined(ZEND_ENGINE_2)
        if (zend_hash_find(tuple, name, name_len, (void**)&tmpzval) != SUCCESS)
        {
          php_net_stream_invalid_key_name(name, name_len);
          return -1;
        }
        option = *tmpzval;
        tmpzval = NULL;
#endif
      }
      else
      {
        if (pkt->data_len <= pkt->index || pkt->format_index >= pkt->format_len)
        {
          zend_error(E_WARNING, NET_STREAM_LOG_1);
          return -1;
        }
#if defined(ZEND_ENGINE_3)
        if ((option = zend_hash_index_find(tuple, j)) == NULL)
        {
          zend_error(E_WARNING, NET_STREAM_LOG_4, j);
          return -1;
        }
#elif defined(ZEND_ENGINE_2)
        if (zend_hash_index_find(tuple, j, (void**)&tmpzval) != SUCCESS)
        {
          zend_error(E_WARNING, NET_STREAM_LOG_4, j);
          return -1;
        }
        option = *tmpzval;
        tmpzval = NULL;
#endif
      }

      if (format_count && NET_STREAM_FORMAT_NONE != format_type)
      {
        php_net_stream_set_value(pkt->data, pkt->data_len, &pkt->index, option, format_type);
        if (!--format_count) format_type = NET_STREAM_FORMAT_NONE;
        ++j;
        continue;
      }

      ch = pkt->data_format[pkt->format_index++];
      if ('1' <= ch && '9' >= ch)
      {
        if (NET_STREAM_FORMAT_NONE == format_type)
          continue;
        format_count = ch - '0';
        if (pkt->format_index < pkt->format_len && '0' <= pkt->data_format[pkt->format_index] && '9' >= pkt->data_format[pkt->format_index])
        {
          format_count *= 10;
          format_count += pkt->data_format[pkt->format_index++] - '0';
        }
        if (1 < format_count)
        {
          php_net_stream_set_value(pkt->data, pkt->data_len, &pkt->index, option, format_type);
          format_count -= 2;
          if (!format_count) format_type = NET_STREAM_FORMAT_NONE;
          ++j;
        }
        else
        {
          format_count = 0;
          format_type = NET_STREAM_FORMAT_NONE;
        }
        continue;
      }

      switch (ch)
      {
      case NET_STREAM_FORMAT_CHAR_ARRAY:
        if (php_net_stream_set_array(pkt, option, 0 TSRMLS_CC))
          return -1;
        format_count = 0;
        format_type = NET_STREAM_FORMAT_NONE;
        break;
      case NET_STREAM_FORMAT_CHAR_KEY_ARRAY:
        if (php_net_stream_set_array(pkt, option, 1 TSRMLS_CC))
          return -1;
        format_count = 0;
        format_type = NET_STREAM_FORMAT_NONE;
        break;
      default:
        format_type = php_net_stream_get_format_type(ch);
        if (NET_STREAM_FORMAT_NONE != format_type)
        {
          php_net_stream_set_value(pkt->data, pkt->data_len, &pkt->index, option, format_type);
          break;
        }
        zend_error(E_WARNING, NET_STREAM_LOG_2);
        return -1;
      }
      ++j;
    }
  }
  if (is_key_array) pkt->key_tail = pkt->key_head - 1;
  return 0;
}

static void php_net_stream_get_array(zval* z_root, net_stream_packet_t* pkt, const char* arr_name, size_t arr_name_len, int8_t is_key_array)
{
  zval array_value, z_value;
#ifdef ZEND_ENGINE_2
  zval** tmpzval = NULL;
#endif
  char ch, *name = NULL, *ptr;
  size_t i, j, num, size, name_len, format_start, format_count = 0, format_type = NET_STREAM_FORMAT_NONE;
  int8_t is_0k = 0;

  if (pkt->index + sizeof(int8_t) > pkt->data_len)
  {
    if (NULL == arr_name)
      add_next_index_null(z_root);
    else
      add_assoc_null_ex(z_root, arr_name, arr_name_len);
    pkt->index = pkt->data_len;
    return;
  }

  ch = pkt->data_format[pkt->format_index++];

  if ('0' == ch)
  {
    if (!is_key_array)
    {
      if (NULL == arr_name)
        add_next_index_null(z_root);
      else
        add_assoc_null_ex(z_root, arr_name, arr_name_len);
      pkt->index = pkt->data_len;
      return;
    }
    is_0k = 1;
    num = 1;
  }
  else
  {
    num = *((uint8_t*)(pkt->data + pkt->index++));
    if (0x7f < num)
    {
      if (pkt->index + sizeof(int8_t) > pkt->data_len)
      {
        if (NULL == arr_name)
          add_next_index_null(z_root);
        else
          add_assoc_null_ex(z_root, arr_name, arr_name_len);
        pkt->index = pkt->data_len;
        return;
      }
      num = (num & 0x7f) << 8;
      num += *((uint8_t*)(pkt->data + pkt->index++));
    }
    if (!num || pkt->index + num > pkt->data_len)
    {
      if (NULL == arr_name)
        add_next_index_null(z_root);
      else
        add_assoc_null_ex(z_root, arr_name, arr_name_len);
      pkt->index = pkt->data_len;
      return;
    }
  }

  if (!is_0k && ('1' > ch || '9' < ch))
  {
    if (is_key_array)
    {
      if (NULL == arr_name)
        add_next_index_null(z_root);
      else
        add_assoc_null_ex(z_root, arr_name, arr_name_len);
      pkt->index = pkt->data_len;
      return;
    }
    array_init(&array_value);
    format_type = php_net_stream_get_format_type(ch);
    for (i = 0; i < num; ++i)
      php_net_stream_get_value(&array_value, format_type, &pkt->index, pkt->data, pkt->data_len, NULL, 0);
    if (NULL == arr_name)
      add_next_index_zval(z_root, &array_value);
    else
      add_assoc_zval_ex(z_root, arr_name, arr_name_len, &array_value);
    if (is_key_array) pkt->key_tail = pkt->key_head - 1;
    return;
  }

  size = ch - '0';
  if (pkt->format_index < pkt->format_len && '0' <= pkt->data_format[pkt->format_index] && '9' >= pkt->data_format[pkt->format_index])
  {
    size *= 10;
    size += pkt->data_format[pkt->format_index++] - '0';
    if (pkt->format_index < pkt->format_len && '0' <= pkt->data_format[pkt->format_index] && '9' >= pkt->data_format[pkt->format_index])
    {
      size *= 10;
      size += pkt->data_format[pkt->format_index++] - '0';
    }
  }

  format_start = pkt->format_index;
  ptr = pkt->key_head;

  if (!is_0k)
    array_init(&array_value);

  for (i = 0; i < num; ++i)
  {
    array_init(&z_value);
    pkt->key_head = pkt->key_tail = ptr;
    pkt->format_index = format_start;

    for (j = 0; j < size; ++j)
    {
      if (is_key_array)
      {
        if (pkt->key_tail > pkt->key_end)
          break;
        while (pkt->key_tail != pkt->key_end && '/' != *pkt->key_tail)
        {
          if (++pkt->key_tail > pkt->key_end)
            break;
          continue;
        }
        name_len = pkt->key_tail - pkt->key_head;
        name = pkt->key_head;
        pkt->key_head = pkt->key_tail + 1;
        ++pkt->key_tail;
        if (!name_len) continue;
      }

      if (pkt->index >= pkt->data_len || pkt->format_index >= pkt->format_len)
      {
        if (NULL == name)
          add_next_index_null(&z_value);
        else
          add_assoc_null_ex(&z_value, name, name_len);
        continue;
      }
      if (format_count && NET_STREAM_FORMAT_NONE != format_type)
      {
        php_net_stream_get_value(&z_value, format_type, &pkt->index, pkt->data, pkt->data_len, name, name_len);
        if (!--format_count) format_type = NET_STREAM_FORMAT_NONE;
        continue;
      }

      ch = pkt->data_format[pkt->format_index++];
      if ('1' <= ch && '9' >= ch)
      {
        if (NET_STREAM_FORMAT_NONE == format_type)
        {
          if (NULL == name)
            add_next_index_null(&z_value);
          else
            add_assoc_null_ex(&z_value, name, name_len);
          continue;
        }
        format_count = ch - '0';
        if (pkt->format_index < pkt->format_len && '0' <= pkt->data_format[pkt->format_index] && '9' >= pkt->data_format[pkt->format_index])
        {
          format_count *= 10;
          format_count += pkt->data_format[pkt->format_index++] - '0';
        }
        if (1 < format_count)
        {
          php_net_stream_get_value(&z_value, format_type, &pkt->index, pkt->data, pkt->data_len, name, name_len);
          format_count -= 2;
          if (!format_count) format_type = NET_STREAM_FORMAT_NONE;
        }
        else
        {
          if (NULL == name)
            add_next_index_null(&z_value);
          else
            add_assoc_null_ex(&z_value, name, name_len);
          format_count = 0;
          format_type = NET_STREAM_FORMAT_NONE;
        }
        continue;
      }

      switch (ch)
      {
      case NET_STREAM_FORMAT_CHAR_ARRAY:
        php_net_stream_get_array(&z_value, pkt, name, name_len, 0);
        format_count = 0;
        format_type = NET_STREAM_FORMAT_NONE;
        break;
      case NET_STREAM_FORMAT_CHAR_KEY_ARRAY:
        php_net_stream_get_array(&z_value, pkt, name, name_len, 1);
        format_count = 0;
        format_type = NET_STREAM_FORMAT_NONE;
        break;
      default:
        format_type = php_net_stream_get_format_type(ch);
        if (NET_STREAM_FORMAT_NONE != format_type)
        {
          php_net_stream_get_value(&z_value, format_type, &pkt->index, pkt->data, pkt->data_len, name, name_len);
          break;
        }
        if (NULL == name)
          add_next_index_null(&z_value);
        else
          add_assoc_null_ex(&z_value, name, name_len);
        zend_error(E_WARNING, NET_STREAM_LOG_5);
      }
    }
    if (!is_0k)
      add_next_index_zval(&array_value, &z_value);
  }
  if (is_0k)
  {
    if (NULL == arr_name)
      add_next_index_zval(z_root, &z_value);
    else
      add_assoc_zval_ex(z_root, arr_name, arr_name_len, &z_value);
  }
  else
  {
    if (NULL == arr_name)
      add_next_index_zval(z_root, &array_value);
    else
      add_assoc_zval_ex(z_root, arr_name, arr_name_len, &array_value);
  }
  if (is_key_array) pkt->key_tail = pkt->key_head - 1;
}

PHP_FUNCTION(net_stream_get)
{
  size_t format_type;
  net_stream_packet_t pkt;
  pkt.data_format = pkt.data_key = NULL;
  pkt.format_len = pkt.key_len = 0;

  if (3 > ZEND_NUM_ARGS()) WRONG_PARAM_COUNT;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll|ss", &pkt.data, &pkt.data_len, &format_type, &pkt.index, &pkt.data_format, &pkt.format_len, &pkt.data_key, &pkt.key_len) == FAILURE)
    RETURN_NULL();
  if (!pkt.data_len || pkt.index >= pkt.data_len || NET_STREAM_FORMAT_NONE >= format_type || NET_STREAM_FORMAT_MAX_TAG <= format_type)
    RETURN_NULL();

  array_init(return_value);
  if (NET_STREAM_FORMAT_ARRAY == format_type)
  {
    pkt.key_tail = pkt.key_head = pkt.data_key;
    pkt.key_end = pkt.data_key + pkt.key_len;
    pkt.format_index = 0;
    php_net_stream_get_array(return_value, &pkt, NULL, 0, (NULL == pkt.data_key ? 0 : 1));
  }
  else
  {
    php_net_stream_get_value(return_value, format_type, &pkt.index, pkt.data, pkt.data_len, NULL, 0);
  }
  add_next_index_long(return_value, pkt.index);
}

PHP_FUNCTION(net_stream_set)
{
  zval* option;
  size_t format_type;
  net_stream_packet_t pkt;
  pkt.data_format = pkt.data_key = NULL;
  pkt.format_len = pkt.key_len = 0;

  if (3 > ZEND_NUM_ARGS()) WRONG_PARAM_COUNT;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zsll|ss", &option, &pkt.data, &pkt.data_len, &format_type, &pkt.index, &pkt.data_format, &pkt.format_len, &pkt.data_key, &pkt.key_len) == FAILURE)
    RETURN_FALSE;
  if (!pkt.data_len || pkt.index >= pkt.data_len || NET_STREAM_FORMAT_NONE >= format_type || NET_STREAM_FORMAT_MAX_TAG <= format_type)
    RETURN_FALSE;

  if (NET_STREAM_FORMAT_ARRAY == format_type)
  {
    pkt.key_tail = pkt.key_head = pkt.data_key;
    pkt.key_end = pkt.data_key + pkt.key_len;
    pkt.format_index = 0;
    if (php_net_stream_set_array(&pkt, option, (NULL == pkt.data_key ? 0 : 1) TSRMLS_CC))
      RETURN_FALSE;
  }
  else
  {
    php_net_stream_set_value(pkt.data, pkt.data_len, &pkt.index, option, format_type);
  }
  RETURN_LONG(pkt.index);
}

PHP_FUNCTION(net_stream_unpack)
{
  char ch, *name;
  size_t name_len, format_count, format_type;
  net_stream_packet_t pkt;
  pkt.index = 0;

  if (3 > ZEND_NUM_ARGS()) WRONG_PARAM_COUNT;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|l", &pkt.data, &pkt.data_len, &pkt.data_format, &pkt.format_len, &pkt.data_key, &pkt.key_len, &pkt.index) == FAILURE)
    RETURN_NULL();
  if (!pkt.data_len || !pkt.key_len || !pkt.format_len || pkt.index >= pkt.data_len)
    RETURN_NULL();

  array_init(return_value);
  pkt.key_tail = pkt.key_head = pkt.data_key;
  pkt.key_end = pkt.data_key + pkt.key_len;
  pkt.format_index = format_count = 0;
  format_type = NET_STREAM_FORMAT_NONE;

  for (; pkt.key_tail <= pkt.key_end; ++pkt.key_tail)
  {
    if (pkt.key_tail != pkt.key_end && '/' != *pkt.key_tail)
      continue;
    name_len = pkt.key_tail - pkt.key_head;
    name = pkt.key_head;
    pkt.key_head = pkt.key_tail + 1;
    if (!name_len) continue;

    if (pkt.index >= pkt.data_len || (pkt.format_index >= pkt.format_len && !format_count))
    {
      add_assoc_null_ex(return_value, name, name_len);
      continue;
    }
    if (format_count && NET_STREAM_FORMAT_NONE != format_type)
    {
      php_net_stream_get_value(return_value, format_type, &pkt.index, pkt.data, pkt.data_len, name, name_len);
      if (!--format_count) format_type = NET_STREAM_FORMAT_NONE;
      continue;
    }

    ch = pkt.data_format[pkt.format_index++];
    if ('1' <= ch && '9' >= ch)
    {
      if (NET_STREAM_FORMAT_NONE == format_type)
      {
        add_assoc_null_ex(return_value, name, name_len);
        continue;
      }
      format_count = ch - '0';
      if (pkt.format_index < pkt.format_len && '0' <= pkt.data_format[pkt.format_index] && '9' >= pkt.data_format[pkt.format_index])
      {
        format_count *= 10;
        format_count += pkt.data_format[pkt.format_index++] - '0';
      }
      if (1 < format_count)
      {
        php_net_stream_get_value(return_value, format_type, &pkt.index, pkt.data, pkt.data_len, name, name_len);
        format_count -= 2;
        if (!format_count) format_type = NET_STREAM_FORMAT_NONE;
      }
      else
      {
        add_assoc_null_ex(return_value, name, name_len);
        format_count = 0;
        format_type = NET_STREAM_FORMAT_NONE;
      }
      continue;
    }

    switch (ch)
    {
    case NET_STREAM_FORMAT_CHAR_ARRAY:
      php_net_stream_get_array(return_value, &pkt, name, name_len, 0);
      format_count = 0;
      format_type = NET_STREAM_FORMAT_NONE;
      break;
    case NET_STREAM_FORMAT_CHAR_KEY_ARRAY:
      php_net_stream_get_array(return_value, &pkt, name, name_len, 1);
      format_count = 0;
      format_type = NET_STREAM_FORMAT_NONE;
      break;
    default:
      format_type = php_net_stream_get_format_type(ch);
      if (NET_STREAM_FORMAT_NONE != format_type)
      {
        php_net_stream_get_value(return_value, format_type, &pkt.index, pkt.data, pkt.data_len, name, name_len);
        break;
      }
      add_assoc_null_ex(return_value, name, name_len);
      zend_error(E_WARNING, NET_STREAM_LOG_5);
    }
  }
}

PHP_FUNCTION(net_stream_pack)
{
  zval *parameter, *option;
#ifdef ZEND_ENGINE_2
  zval** tmpzval = NULL;
#endif
  HashTable* arr;
  char ch, *name;
  size_t name_len, format_count, format_type;
  net_stream_packet_t pkt;
  pkt.data_len = NET_STREAM_OUTBUF_SIZE;
  pkt.index = 0;

  if (3 > ZEND_NUM_ARGS()) WRONG_PARAM_COUNT;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z!s!s!|ll", &parameter, &pkt.data_format, &pkt.format_len, &pkt.data_key, &pkt.key_len, &pkt.index, &pkt.data_len) == FAILURE)
    RETURN_NULL();
  if (!pkt.data_len || pkt.index > pkt.data_len)
    RETURN_NULL();

  if (pkt.index == pkt.data_len)
  {
    pkt.data = (char*)emalloc(pkt.data_len);
    memset(pkt.data, 0, pkt.data_len);
#if defined(ZEND_ENGINE_3)
    RETVAL_STRINGL(pkt.data, pkt.index);
#elif defined(ZEND_ENGINE_2)
    RETVAL_STRINGL(pkt.data, pkt.index, 1);
#endif
    efree(pkt.data);
    return;
  }
  if (!parameter || !pkt.key_len || !pkt.format_len)
    RETURN_NULL();

  pkt.data = (char*)emalloc(pkt.data_len);
  arr = HASH_OF(parameter);
  pkt.key_tail = pkt.key_head = pkt.data_key;
  pkt.key_end = pkt.data_key + pkt.key_len;
  pkt.format_index = format_count = 0;
  format_type = NET_STREAM_FORMAT_NONE;

  for (; pkt.key_tail <= pkt.key_end; ++pkt.key_tail)
  {
    if (pkt.key_tail != pkt.key_end && '/' != *pkt.key_tail)
      continue;
    name_len = pkt.key_tail - pkt.key_head;
    name = pkt.key_head;
    pkt.key_head = pkt.key_tail + 1;
    if (!name_len) continue;

    if (pkt.data_len <= pkt.index || (pkt.format_index >= pkt.format_len && !format_count))
    {
      zend_error(E_WARNING, NET_STREAM_LOG_1);
      efree(pkt.data);
      RETURN_NULL();
    }
#if defined(ZEND_ENGINE_3)
    if ((option = zend_hash_str_find(arr, name, name_len)) == NULL)
    {
      php_net_stream_invalid_key_name(name, name_len);
      efree(pkt.data);
      RETURN_NULL();
    }
#elif defined(ZEND_ENGINE_2)
    if (zend_hash_find(arr, name, name_len, (void**)&tmpzval) != SUCCESS)
    {
      php_net_stream_invalid_key_name(name, name_len);
      efree(pkt.data);
      RETURN_NULL();
    }
    option = *tmpzval;
    tmpzval = NULL;
#endif

    if (format_count && NET_STREAM_FORMAT_NONE != format_type)
    {
      php_net_stream_set_value(pkt.data, pkt.data_len, &pkt.index, option, format_type);
      if (!--format_count) format_type = NET_STREAM_FORMAT_NONE;
      continue;
    }

    ch = pkt.data_format[pkt.format_index++];
    if ('1' <= ch && '9' >= ch)
    {
      if (NET_STREAM_FORMAT_NONE == format_type)
        continue;
      format_count = ch - '0';
      if (pkt.format_index < pkt.format_len && '0' <= pkt.data_format[pkt.format_index] && '9' >= pkt.data_format[pkt.format_index])
      {
        format_count *= 10;
        format_count += pkt.data_format[pkt.format_index++] - '0';
      }
      if (1 < format_count)
      {
        php_net_stream_set_value(pkt.data, pkt.data_len, &pkt.index, option, format_type);
        format_count -= 2;
        if (!format_count) format_type = NET_STREAM_FORMAT_NONE;
      }
      else
      {
        format_count = 0;
        format_type = NET_STREAM_FORMAT_NONE;
      }
      continue;
    }

    switch (ch)
    {
    case NET_STREAM_FORMAT_CHAR_ARRAY:
      if (php_net_stream_set_array(&pkt, option, 0 TSRMLS_CC))
      {
        efree(pkt.data);
        RETURN_NULL();
      }
      format_count = 0;
      format_type = NET_STREAM_FORMAT_NONE;
      break;
    case NET_STREAM_FORMAT_CHAR_KEY_ARRAY:
      if (php_net_stream_set_array(&pkt, option, 1 TSRMLS_CC))
      {
        efree(pkt.data);
        RETURN_NULL();
      }
      format_count = 0;
      format_type = NET_STREAM_FORMAT_NONE;
      break;
    default:
      format_type = php_net_stream_get_format_type(ch);
      if (NET_STREAM_FORMAT_NONE != format_type)
      {
        php_net_stream_set_value(pkt.data, pkt.data_len, &pkt.index, option, format_type);
        break;
      }
      zend_error(E_WARNING, NET_STREAM_LOG_2);
      efree(pkt.data);
      RETURN_NULL();
    }
  }

#if defined(ZEND_ENGINE_3)
  RETVAL_STRINGL(pkt.data, pkt.index);
#elif defined(ZEND_ENGINE_2)
  RETVAL_STRINGL(pkt.data, pkt.index, 1);
#endif
  efree(pkt.data);
}

PHP_FUNCTION(net_stream_encode)
{
  php_net_stream_extract(INTERNAL_FUNCTION_PARAM_PASSTHRU, NET_STREAM_Z_DEINFLATE);
}

PHP_FUNCTION(net_stream_decode)
{
  php_net_stream_extract(INTERNAL_FUNCTION_PARAM_PASSTHRU, NET_STREAM_Z_INFLATE);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
