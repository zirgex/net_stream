<?php

$pack_data = net_stream_pack(null, null, null, 32, 32);

$value = 1001;
$cursor = net_stream_set($value, $pack_data, NET_STREAM_UINT_16, 0);

$key = 'name/passwd';
$type = '02s2';
$arr = array('name' => 'phper', 'passwd' => '123456');
$cursor = net_stream_set($arr, $pack_data, NET_STREAM_ARRAY, $cursor, $type, $key);

$cursor = net_stream_set('ok', $pack_data, NET_STREAM_STRING, $cursor);
if (false !== $cursor)
  echo 'length=', $cursor, "\n";

$v = net_stream_get($pack_data, NET_STREAM_UINT_16, 0);
echo 'value=', $v[NET_STREAM_VALUE], "\n";

$v = net_stream_get($pack_data, NET_STREAM_ARRAY, $v[NET_STREAM_CURSOR], $type, $key);
if (is_null($v)) die("NULL\n");
echo var_export($v, true), "\n";

$v = net_stream_get($pack_data, NET_STREAM_STRING, $v[NET_STREAM_CURSOR]);
if (is_null($v)) die("NULL\n");
echo var_export($v, true), "\n";

