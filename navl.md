# navl

## Purpose

Using an avltree and sorting, the program reads through command-line arguements.
Each input file is broken into chucks of three to six bytes.  The chunks are hex encoded
with the first hex being the length of the chunk.   The remainer of the chunk is the data. The first pass then is sorted and passed through uniq and counted.   

A second pass is made where the count is multiplied by the bytes of compression, with the assumption
that product of count and compression represents the value of using that chuck. Again this is sorted.

Pass three feeds the entire pass two into a tree and the feed all of the command line files to be tested against the tree.  If a chunk is found in the tree then its visited flag is incremented.   This pass is biased for
the longer chunks that are common in multiple files.

If there are less than 255 nodes that have been visited in the tree then we start testing the pass one file
against the tree until the table has 255 ebtires or we have exhausted the pass one file.

if --output-file=something.h then something.h is written which contains cmptab which just a list of enodings
that were visited in the tree and exptab which is just a list of how to expand the coded files.


## Build


`make`

## Command line switches

switch|Required/Optional|argument|description
---|---|---|---
output-filename|Optional|path/filename|Header file for encode and decode.  Existing file will be overwritten.   
quiet|Optional|NONE|turns off debugging output (default)
verbose|Optiona|NONE|turns on debugging output
help|Otional|NONE|displays a help message to stdout and exits.
pass-two|NOT REQUIRED|none|used internally in pipes and is not of interest to user.

## Example

Assume that sample binarary files are in data/

``./navl --output-file whatever.h data/*bin``

Will read through all of the `data/*bin` files and output a `whatever.h`.

It is important the any input files be representative of the population of files to be encoded.

