#ifndef NET_STREAM_H
#define NET_STREAM_H

#define PHP_NET_STREAM_VERSION "0.1.1"
#define PHP_NET_STREAM_RELEASE_DATE "2018-10-24"

enum
{
  NET_STREAM_FORMAT_NONE = 0,
  NET_STREAM_FORMAT_INT_8,
  NET_STREAM_FORMAT_INT_16,
  NET_STREAM_FORMAT_INT_32,
  NET_STREAM_FORMAT_INT_64,
  NET_STREAM_FORMAT_UINT_8,
  NET_STREAM_FORMAT_UINT_16,
  NET_STREAM_FORMAT_UINT_32,
  NET_STREAM_FORMAT_FLOAT,
  NET_STREAM_FORMAT_DOUBLE,
  NET_STREAM_FORMAT_STRING,
  NET_STREAM_FORMAT_ARRAY,
  NET_STREAM_FORMAT_BYTE,
  NET_STREAM_FORMAT_MAX_TAG
};

#define NET_STREAM_FORMAT_CHAR_INT_8      'c'
#define NET_STREAM_FORMAT_CHAR_INT_16     'r'
#define NET_STREAM_FORMAT_CHAR_INT_32     'i'
#define NET_STREAM_FORMAT_CHAR_INT_64     'q'
#define NET_STREAM_FORMAT_CHAR_UINT_8     'n'
#define NET_STREAM_FORMAT_CHAR_UINT_16    'm'
#define NET_STREAM_FORMAT_CHAR_UINT_32    'u'
#define NET_STREAM_FORMAT_CHAR_FLOAT      'f'
#define NET_STREAM_FORMAT_CHAR_DOUBLE     'd'
#define NET_STREAM_FORMAT_CHAR_STRING     's'
#define NET_STREAM_FORMAT_CHAR_ARRAY      'a'
#define NET_STREAM_FORMAT_CHAR_KEY_ARRAY  'k'

// pre-allocated memory, used to compress or uncompress
#define NET_STREAM_OUTBUF_SIZE  0x200

#define NET_STREAM_COMPRESSED   1

// compress
#define NET_STREAM_Z_DEINFLATE  0
// uncompress
#define NET_STREAM_Z_INFLATE    1

#define NET_STREAM_LOG_1 "net_stream_pack: Unknown error."
#define NET_STREAM_LOG_2 "net_stream_pack: Unknown format."
#define NET_STREAM_LOG_3 "net_stream_pack: Invalid key name \"%s\"."
#define NET_STREAM_LOG_4 "net_stream_pack: Invalid array index \"%u\"."
#define NET_STREAM_LOG_5 "net_stream_unpack: Unknown format."

typedef struct
{
  char *key_head, *key_tail, *key_end;
  char *data_key, *data_format, *data;
  size_t index, data_len, key_len, format_len, format_index;
} net_stream_packet_t;

#endif  /* NET_STREAM_H */
