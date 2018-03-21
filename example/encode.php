<?php
$key = 'i0/i1/i2/j0/j1/j2/c0/c1/c2';
$type = 'i2uq3c2n';
$raw_data = array(
  'i0' => 2147483647, 'i1' => -1, 'i2' => -1,
  'j0' => 4294967295, 'j1' => 9223372036854775295, 'j2' => 9223372036854775807,
  'c0' => 127, 'c1' => -1, 'c2' => -1
  );
$pack_data = net_stream_pack($raw_data, $type, $key, 0, NET_STREAM_OUTBUF_SIZE);
$obfuscator = array(36, 19, 80, 12, 37, 90, 26, 51);

function example_1() {
  global $type, $key, $pack_data, $obfuscator;
  $encode_data = net_stream_encode($pack_data, $obfuscator);
  $decode_data = net_stream_decode($encode_data, $obfuscator);
  $unpack_data = net_stream_unpack($decode_data, $type, $key);
  echo "obfuscator:\n", var_export($unpack_data, true), "\n";
}

function example_2() {
  global $type, $key, $pack_data, $obfuscator;
  $encode_data = net_stream_encode($pack_data, $obfuscator, NET_STREAM_COMPRESSED);
  $decode_data = net_stream_decode($encode_data, $obfuscator, NET_STREAM_COMPRESSED);
  $unpack_data = net_stream_unpack($decode_data, $type, $key);
  echo "obfuscator, compressed:\n", var_export($unpack_data, true), "\n";
}

function example_3() {
  global $type, $key, $pack_data;
  $encode_data = net_stream_encode($pack_data, null, NET_STREAM_COMPRESSED);
  $decode_data = net_stream_decode($encode_data, null, NET_STREAM_COMPRESSED);
  $unpack_data = net_stream_unpack($decode_data, $type, $key);
  echo "compressed:\n", var_export($unpack_data, true), "\n";
}

function example_4() {
  global $type, $key, $pack_data, $obfuscator;
  $encode_data = net_stream_encode($pack_data, $obfuscator, 0, NET_STREAM_OUTBUF_SIZE);
  $decode_data = net_stream_decode($encode_data, $obfuscator, 0, NET_STREAM_OUTBUF_SIZE);
  $unpack_data = net_stream_unpack($decode_data, $type, $key);
  echo "obfuscator, customize outbuf size:\n", var_export($unpack_data, true), "\n";
}

function example_5() {
  global $type, $key, $pack_data, $obfuscator;
  $encode_data = net_stream_encode($pack_data, $obfuscator, NET_STREAM_COMPRESSED, 256);
  $decode_data = net_stream_decode($encode_data, $obfuscator, NET_STREAM_COMPRESSED, 256);
  $unpack_data = net_stream_unpack($decode_data, $type, $key);
  echo "obfuscator, compressed, customize outbuf size:\n", var_export($unpack_data, true), "\n";
}

$map = array(1, 2, 3, 4, 5);
$index = isset($_SERVER['argv'][1]) ? (int)$_SERVER['argv'][1] : 0;
$length = count($map);
if (1 > $index || $index > $length)
  die('Enter: php encode.php [1-' . $length . "]\n");
echo 'Example #', $index, ': \'', $type, '\', \'', $key, '\'', "\n", var_export($raw_data, true), "\n";
$fn = 'example_' . $map[$index-1];
$fn();

