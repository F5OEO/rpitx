<?php

// a chunk = 8 bytes for freq drift in hz + 4 bytes for duration + 4 bytes of 0

function debug_ft($chunk)
{
  echo bin2hex($chunk);
  //for($i = 0; $i < strlen($chunk); $i++)
  //{
  //  printf("%02x ", substr($chunk, $i, 0));
  //}
  echo "\n";
}

function send_ft($fd, $chunk)
{
  fputs($fd, $chunk);
  debug_ft($chunk);
}

function ft_open($filename)
{
  $fd = fopen($filename, "wb");
  return $fd;
}

function ft_close($fd)
{
  fclose($fd);
}
//         
$Came_before = "\x00\x00\x00\x00\x00\x00\x00\x00". // gap
          "\x00\x98\x96\x80". // duration : 10 000 000 ns
          "\x00\x00\x00\x00"; // padding;
$Came_S = "\x00\x00\x00\x00\x00\x00\xf0\x3f". // pulse
          "\x00\x04\xe2\x00". // duration : 320 000 ns
          "\x00\x00\x00\x00". // padding
          "\x00\x00\x00\x00\x00\x00\x00\x00". // gap
          "\x00\x04\xe2\x00". // duration : 320 000 ns
          "\x00\x00\x00\x00"; // padding
$Came_0 = "\x00\x00\x00\x00\x00\x00\x00\x00". // gap
          "\x00\x04\xe2\x00". // duration : 320 000 ns
          "\x00\x00\x00\x00". // padding
          "\x00\x00\x00\x00\x00\x00\xf0\x3f". // pulse
          "\x00\x09\xc4\x00". // duration : 640 000 ns
          "\x00\x00\x00\x00"; // padding
$Came_1 = "\x00\x00\x00\x00\x00\x00\x00\x00". // gap
          "\x00\x09\xc4\x00". // duration : 640 000 ns
          "\x00\x00\x00\x00". // padding
          "\x00\x00\x00\x00\x00\x00\xf0\x3f". // pulse
          "\x00\x04\xe2\x00". // duration : 320 000 ns
          "\x00\x00\x00\x00"; // padding
$Came_after = "\x00\x00\x00\x00\x00\x00\x00\x00". // gap
          "\x00\x98\x96\x80". // duration : 10 000 000 ns
          "\x00\x00\x00\x00"; // padding
$data = "010001110011";

$fd = ft_open("came.ft");
// sudo ./rpitx  -f 440000 -m RF -i came.ft

// before
send_ft($fd, $Came_before);
// Send start bit
send_ft($fd, $Came_S);
for($i = 0; $i < strlen($data); $i++)
{
  $bit = $data[$i];
  if($bit == '0')
    send_ft($fd, $Came_0);
  else if($bit == '1')
    send_ft($fd, $Came_1);
}
// after
send_ft($fd, $Came_after);
ft_close($fd);