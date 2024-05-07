## Arbitrary width integer and floating point types

This library provides the data types `APInt` and `APFloat` which model integer and floating point types of arbitrary width. 

`APInt` is a 2's complement integer type of runtime specified bit width. `APFloat` at this point is not properly implemented and only supports 32 bit and 64 bit modes, implemented using the builtin `float` and `double` types. 

Many arithmetic operations are provided. The library does not provide operator overloads, partly because `APInt` models both signed and unsigned integers and the sign interpretation is encoded into the operations. For example `sdiv` performs signed and `udiv` performs unsigned division.     

The main purpose of the library is to emulate target arithmetic in a compiler.
Performance should be reasonable but is not the main focus.
