<?php

$pack_data = net_stream_pack(null, null, null, 48, 48);

$value = 1001;
$cursor = net_stream_set($value, $pack_data, NET_STREAM_UINT_16, 0);

$list = array();
$list[0] = array('02s2', 'name/passwd',
  array('name' => 'phper', 'passwd' => '123456'));

$list[1] = array('s', null, array('phper', '123456'));

$list[2] = array('2s2', 'name/passwd',
  array(array('name' => 'phper1', 'passwd' => '123456'), array('name' => 'phper2', 'passwd' => '456789'), array('name' => 'phper3', 'passwd' => '654321')));

$list[3] = array('2s2', null,
  array(array('phper1', '123456'), array('phper2', '456789'), array('phper3', '654321')));

$list[4] = array('3si2', 'x/y/z',
  array(array('x' => 'a', 'y' => 1000, 'z' => 1), array('x' => 'b', 'y' => 2000, 'z' => 2)));

$list[5] = array('06si2si2', null, array('a', 1000, 1, 'b', 2000, 2));

$length = count($list);
$index = isset($_SERVER['argv'][1]) ? (int)$_SERVER['argv'][1] - 1 : -1;
if (0 > $index || $index >= $length)
  die('Enter: php get.php [1-' . $length . "]\n");

$type =& $list[$index][0];
$key =& $list[$index][1];
$arr =& $list[$index][2];
$cursor = net_stream_set($arr, $pack_data, NET_STREAM_ARRAY, $cursor, $type, $key);

$cursor = net_stream_set('ok', $pack_data, NET_STREAM_STRING, $cursor);
if (false !== $cursor)
  echo 'length=', $cursor, "\n";
echo 'Example #', ($index+1), ': \'', $type, '\', \'', $key, '\'', "\n";

$v = net_stream_get($pack_data, NET_STREAM_UINT_16, 0);
echo 'value=', $v[NET_STREAM_VALUE], "\n";

$v = net_stream_get($pack_data, NET_STREAM_ARRAY, $v[NET_STREAM_CURSOR], $type, $key);
if (is_null($v)) die("NULL\n");
echo var_export($v, true), "\n";

$v = net_stream_get($pack_data, NET_STREAM_STRING, $v[NET_STREAM_CURSOR]);
if (is_null($v)) die("NULL\n");
echo var_export($v, true), "\n";

