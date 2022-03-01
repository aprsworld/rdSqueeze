# decode

## Purpose

Decode is a program that decodes files that have been encoded by `encode`.   It is important that both `decode` and `encode` use
the same compression tables.   In this case the compression table is contained in the header file `clare.h`

If `argv[1,2,3...] = "data/3005.bin.enc"` then the file outputed will be `"data/3005.bin.enc.dec"`.

## Machine Character Encoding

I have written encode and decode so they should work correctly even if the encode and/decode machine
uses a different 8 bit character set.   The header file HC.h is important and should included in any
program that encodes data to be sent to a decoding machine.  The expansion table was built using `HC.h`

These programs have only been tested with the ansi character set.   It is unknow whether they work correctly
on an ebcdic machine.

## Build


`make`

## Command line switches

There are no command line switches.

## Command line switches

switch|Required/Optional|argument|description
---|---|---|---
dump-table|Optional|none|stdout of compression and expansion table
lookup|Optional|hex index| stdout lookup one line in expansion table using a hexidecimal index

## Example

``decode data/*bin.enc``

or

``decode file1.bin.enc file2.bin.enc ....``


