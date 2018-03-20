<?php
/*
  NET_STREAM_INT_8        'c'
  NET_STREAM_INT_16       'r'
  NET_STREAM_INT_32       'i'
  NET_STREAM_INT_64       'q'
  NET_STREAM_UINT_8       'n'
  NET_STREAM_UINT_16      'm'
  NET_STREAM_UINT_32      'u'
  NET_STREAM_FLOAT        'f'
  NET_STREAM_DOUBLE       'd'
  NET_STREAM_STRING       's'
  NET_STREAM_ARRAY        'a'
  NET_STREAM_KEY_ARRAY    'k'
*/

$data = array();

$data[0] = array('iacs', 'id/arr/msg',
  array('id' => 12, 'arr' => array(1, 3, 2), 'msg' => 'ok'));

$data[1] = array('ia1ss', 'id/arr/msg',
  array('id' => 123, 'arr' => array(array('a'), array('b'), array('c')), 'msg' => 'ok'));

$data[2] = array('ia2s2s', 'id/arr/msg',
  array('id' => 123, 'arr' => array(array('a', 'A'), array('b', 'B'), array('c', 'C')), 'msg' => 'ok'));

$data[3] = array('rk03c3s', 'id/arr/a/b/c/msg',
  array('id' => 123, 'arr' => array('a' => 1, 'b' => 2, 'c' => 3), 'msg' => 'ok'));

$data[4] = array('ik2s2s', 'd/k/x/y/z',
  array('d' => 7, 'k' => array(array('x' => 'a1', 'y' => 'a2'), array('y' => 'b1', 'x' => 'b2'), array('x' => 'c1', 'y' => 'c2')), 'z' => 'ok'));

$length = count($data);
$index = isset($_SERVER['argv'][1]) ? (int)$_SERVER['argv'][1] - 1 : -1;
if (0 > $index || $index >= $length)
  die('Enter: php example.php [1-' . $length . "]\n");

$type =& $data[$index][0];
$key =& $data[$index][1];
$raw_data =& $data[$index][2];

$pack_data = net_stream_pack($raw_data, $type, $key);
$unpack_data = net_stream_unpack($pack_data, $type, $key);

echo 'Example #', ($index+1), "\n";
echo var_export($raw_data, true), "\n";
echo var_export($unpack_data, true), "\n";

