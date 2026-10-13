# Powerpank*

## Discovery of the serial protocol of Cyberpower Value1000EI UPS

###### *(wordplay on cyberpower, cyberpunk and powerbank)

I have an old-ish UPS made by Cyberpower - model "Value1000EI" that has USB and RS-232 ports to communicate with a PC. USB protocol is supported in NUT, and there are a number of vibecoded projects for ESP32-S3 (that has USB host support) that will let you put it on network without needing a linux computer. But I wanted to use an old-simple ESP8266 and talk to it directly via serial protocol.

Inside this UPS has ST72F60E1M1 - an 8 bit STM ST7-core MCU that communicates to the main MCU via a pair of optocouplers and exposes the control to a PC via USB HID protocol. RS-232 port in form of DB9 connector has it's own inverting circuit, but the ST7 MCU communicates via normal UART at 2400(!) baud rate with standard settings of 8N1. 

So I connected two USB serials to TX and RX to sniff both request and response. I noticed that when UPS is unplugged from mains power its main MCU only works while the output is enabled. Once it is disabled either via switch on the front panel or via shutdown command, it also loses power, so the USB MCU keeps knocking on the serial trying to get the answer until the main chip responds again. (USB MCU gets it's power from USB, since it's galvanically isolated from the rest of UPS electronics)

The protocol goes as follows:

1. Start of the request - 0x4*n* (*n* - number of bytes in the message)
2. Command byte, for example ``0x3f`` (get designed nominal battery voltage, this commands is used by ST7 MCU to knock for a response when main MCU is powered down)
3. CRC-8 byte, calculated for all previous bytes with a ``0xd5`` polynomial.
4. The main MCU responds with ``0x40`` ACK byte.
5. After a delay of 15-20ms it sends the rest of the response, which starts with the same 0x4*n* byte that tells the number of bytes that will follow and that includes the repeated command byte as the first byte.
6. CRC-8 byte calculated for bytes after ``0x40``
7. Then client responds with ``0x40`` ACK.

This is the format for all available commands, except for two - model name string and firmware version string. They have a response linger than 8 bytes, so they are split in two parts with their own length and CRC bytes each. Also responds to these two commands don't repeat the command byte, and the first *size* byte omits the first ``4`` nibble. This is a response to model name request:

``40 08 56 61 6c 75 65 31 30 30 72 43 30 45 49 da``

Also important part to note, is that client needs to ACK the first part with ``0x40`` in time to get the second part (and then normally ACK it too).

This is a list of read commands that are the same for HID feature request ID and UART command:
|Byte| Command                                             |
|:-: |-                                                    |
|0x1d| Model name                                          |
|0x1e| Firmware version                                    |
|0x20| Battery charge and voltage                          |
|0x21| Runtime in seconds                                  |
|0x22| Status Flags                                        |
|0x23| Voltage Data in/out                                 |
|0x25| Load Percentage                                     |
|0x27| Active power (found in HID descriptor, but no reply)|
|0x28| Flags: Overload, Boost, Buck (AVR)                  |
|0x29| Beep status                                         |
|0x3d| Some unknown flag                                   |
|0x3e| Designed power W/VA                                 |
|0x3f| Designed nominal battery voltage                    |
|0x80| AudibleAlarmControl (1 disabled, 2 enabled, 3 muted)|
|0x82| RemainingTimeLimit, seconds                         |
|0x83| LowVoltageTransfer                                  |
|0x84| HighVolgateTransfer                                 |
|0x85| Test status (6 = no test initiated)                 |
|0x86| DelayBeforeShutdown, seconds (0xffff = disabled)    |
|0x87| DelayBeforeStartup, seconds (0xffff = disabled)     |
|0x88| ConfigVoltage, nominal output voltage               |

Also ST7 sends ``0x09`` command at the start of every communication loop and gets ``0`` as a response, puspose unknown.

This is how process loop:

```
40 43 3f 78 00 04
40 08 56 61 6c 75 65 31 30 30 72 43 30 45 49 da 
40 08 42 5a 41 42 31 30 30 2e 8b 43 36 42 31 09 
40 45 3e 26 02 e8 03 95 
40 42 3d 00 cd 
40 43 88 e6 00 bd
-----------------
40 43 80 29 85 e2 
40 42 29 01 84 
40 42 80 02 63 
40 42 85 06 ba
40 43 86 ff ff e1 
40 43 87 ff ff 62
-----------------
40 41 00 b4 
40 44 20 4e 7b 00 d5 
40 43 21 50 07 e2 
40 42 22 04 ea 
40 45 23 00 00 e6 00 25 
40 42 25 00 25 
40 42 28 00 5a 
40 42 29 01 84 
40 42 80 02 63 
40 43 82 2c 01 07 
40 43 83 a6 00 b4 
40 43 84 18 01 8e 
40 42 85 06 ba 
40 43 86 ff ff e1 
40 43 87 ff ff 62
```

The last set of commands is repeated in a loop.

Now, there are also some registers that can be written. As far as I can tell those are that starts with ``8``, but to write you need to change ``8`` nibble to ``C``, so command to set *shutdown delay* to ``0`` and effectively turn off the output will look like this: ``43 C6 00 00 04``. The response will be only ``0x40`` ACK.

## ESP8266 firmware

For whatever unexplainable reason I decided to write a custom external component for ESPHome for this thing, while it would be much easier to write a simple MQTT client firmware. The worst part was debugging it and making UART timings work to not miss and data in replies. (Each character takes a little longer than 4ms, plus extra delays after ACKs)

The external component files can be found in ``cyberpower`` folder.

## Supported models

From what info I could gather I suppose that this protocol should work for all *Value* models, and the exact series is Value600EI/800EI/1000EI, which as I understand are only different in the maximum power output.
