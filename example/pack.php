<?php

function example_1() {
  $data = array(
    array('s2a3s2ni2rci5q2i4uid', 'name/passwd/list/money/i/h/c/t/tt/ttt/t4/t5/j/jj/t6/t7/t8/t9/t0/t1/t2'),
    array('s2k3s2ni2rci5q2i4uid', 'name/passwd/list/a/b/c/money/i/h/c/t/tt/ttt/t4/t5/j/jj/t6/t7/t8/t9/t0/t1/t2'));

  $raw_data = array('name' => 'Bruce Li', 'passwd' => '123456789o',
    'list' => array(array('aaa', 'bbb', 255), array('AAA', 'BBB', 128)),
    'money' => 987654321, 'i' => 2147483647, 'h' => 32767, 'c' => 127,
    'j' => 9876543210123, 'jj' => 9223372036854775807, 't' => 123, 'tt' =>4321,
    'ttt' => 65535, 't4' => 4, 't5' => 5, 't6' => 6, 't7' => 7, 't8' => 8, 't9' => 9,
    't0' => 4294967295, 't1' => 4294967295, 't2' => 18446744073709551614);

  $t1 =& $data[0][0];
  $k1 =& $data[0][1];
  $t2 =& $data[1][0];
  $k2 =& $data[1][1];

  $pack_data = net_stream_pack($raw_data, $t1, $k1);
  $unpack_data = net_stream_unpack($pack_data, $t2, $k2);

  echo var_export($raw_data, true), "\n\n";
  echo var_export($unpack_data, true), "\n\n";
}

function example_2() {
  $type = 'mqs';
  $key = 'id/money/msg';
  $raw_data = array('id' => 123, 'money' => 9999999999, 'msg' => 'ok');
  $pack_data = net_stream_pack($raw_data, $type, $key, 0, NET_STREAM_OUTBUF_SIZE);
  echo var_export($raw_data, true), "\n\n";

  $unpack_data = net_stream_unpack($pack_data, $type, $key);
  echo var_export($unpack_data, true), "\n\n";
  $unpack_data = net_stream_unpack($pack_data, 'qs', 'money/msg', 2);
  echo var_export($unpack_data, true), "\n\n";
  $unpack_data = net_stream_unpack($pack_data, 's', 'msg', 10);
  echo var_export($unpack_data, true), "\n\n";
}

function example_3() {
  $raw_data = array('id' => 123, 'money' => 9999999999, 'msg' => 'ok');
  $pack_data = net_stream_pack($raw_data, 'qs', 'money/msg', 2, 16);
  echo var_export($raw_data, true), "\n\n";

  $cursor = net_stream_set($raw_data['id'], $pack_data, NET_STREAM_UINT_16, 0);
  if (false === $cursor)
    die("example_3 ERROE\n");
  $unpack_data = net_stream_unpack($pack_data, 'mqs', 'id/money/msg');
  echo var_export($unpack_data, true), "\n\n";
}

$map = array(1, 2, 3);
$index = isset($_SERVER['argv'][1]) ? (int)$_SERVER['argv'][1] : 0;
$length = count($map);
if (1 > $index || $index > $length)
  die("Enter: php example.php [1-3]\n");
echo 'Example #', $index, "\n";
$fn = 'example_' . $map[$index-1];
$fn();

