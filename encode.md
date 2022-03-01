# encode

## Purpose

Encode reads every file on the command line and encodes the file.   The output is chunks of encoded data and unencoded data.
Each chunk of encoded data is two bytes.   Byte[0] is the escapeChar.   Byte[1] is the entry into the compression table.
A naked escapeChar in the input is output as escapeChar and 0x255.   Therefore any escapeChar in the output must be followed
with a byte.   If `argv[1,2,3...] = "data/3005.bin"` then the file outputed will be `"data/3005.bin.enc"`.

Encode is not meant for production but to test the compression table.
The function encode in encode.c is what is important.

## Machine Character Encoding

I have written encode and decode so they should work correctly even if the encode and/decode machine
uses a different 8 bit character set.   The header file HC.h is important and should included in any
program that encodes data to be sent to a decoding machine.  The function prepKey uses HC.

## Build


`make`

## Command line switches

There are no command line switches.

## Example

``encode data/*bin``

or

``encode file1.bin file2.bin ....``


