# CH55xG-StateLight
CH552 / CH551 USB State Light

## Source Code
Tab size is 4 Spaces

## PC Software
Basically working on Linux, no Messenger sync yet

## Protocol
This is the Target protocol, not 100% ready.

>Sended Text
$ Received Text
All Texts are are terminated with \n, \r or Both (don't matter)

Normal Sequence:
>i
$TODO
Check if Version is correct.

Set all LEDs:
>aRRGGBB
$OK
RRGGBB is an RGB Hex Color
This will turn off blink.

Set single LED:
>sID RRGGBB
$OK
ID is the LED ID, starting from 0, decimal
RRGGBB is an RGB Hex Color
This will turn off blink.

Set auto off time after XXX Seconds
>aXXX
$OK
If there is no command received within XXX Seconds, turn off LEDs

Ping:
>p
$OK
Used to prevent auto turn off

Blink:
>tXXX
$OK
Set blink time to TTT * 100ms
Defaults to 5, => 500ms blink time.

>b255
$OK
Blink all LEDs

>bID
$OK
Blink LED ID, ID is decimal

Write config byte:
>caaa vvv
$OK

aaa Address: decimal, 0 .. 127
vvv Value: decimal, 0 .. 255

Read config byte:
>raaa
$OK vvv
aaa Address: decimal, 0 .. 127
vvv Value: decimal, 0 .. 255


Write value to EEPROM

## EEPROM Config
0 - 1: USB VID
2 - 3: USB PID
4: Checksum VID & PID, VID1 ^ VID2 ^ PID1 ^ PID2

5 - 14: Startup RGB Colors

15: Length Device Name, if < 1 || > 60 the default name is used
16 - 76: Device Name, UTF16












