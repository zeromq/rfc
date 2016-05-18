This document specifies Z85, a format for representing binary data as printable text. Z85 is a derivative of existing [Ascii85 encoding mechanisms](http://en.wikipedia.org/wiki/Ascii85), modified for better usability, particularly for use in source code.

* Name: http://rfc.zeromq.org/spec:32/Z85
* Editor: Pieter Hintjens <ph@imatix.com>

## Preamble

Copyright (c) 2013 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Problem Statement

When representing binary values in code and in text files (such as configuration data), the developer must choose a printable representation. The simplest choice is Base16, where each byte is printed as two hexadecimal values.

For better efficiency than Base16, the developer might choose Base64. This is a common choice but problematic because it has [more than a dozen variants](http://en.wikipedia.org/wiki/Base64). It works poorly with binary data since it works on three-byte chunks, whereas most binary data is chunked to 4 or 8 bytes. Base64 implementations are therefore more complex than they might be, and not necessarily interoperable.

A more logical encoding is Ascii85, which works on 4-byte chunks. However it is not string-safe, so cannot be used cleanly in source code, XML, JSON, and so on.

Our ideal encoding is designed for 4-byte chunks like Ascii85, but is safe for strings, and removes all ambiguity about whether or not implementations will interoperate. Z85 is thus designed to be more compact than Base16, more reliable than Base64, and more usable than Ascii85.

The specific goals of this specification are:

* To provide the most efficient textual representation possible.
* To be easy to use in source code, when enclosed in double or single quotes.
* To be safe to pass on the command line, when enclosed in single quotes.
* To be easy to implement in any programming language.

Additionally, binary zero strings should be easily visible (we achieve this by mapping zero to the ASCII character '0').

## Formal Specification

A Z85 implementation takes a binary frame and encodes it as a printable ASCII string, or takes an ASCII encoded string and decodes it into a binary frame.

The binary frame SHALL have a length that is divisible by 4 with no remainder. The string frame SHALL have a length that is divisible by 5 with no remainder. It is up to the application to ensure that frames and strings are padded if necessary.

The encoding and decoding SHALL use this representation for each base-85 value from zero to 84:

```
 0 -  9:  0 1 2 3 4 5 6 7 8 9
10 - 19:  a b c d e f g h i j
20 - 29:  k l m n o p q r s t
30 - 39:  u v w x y z A B C D
40 - 49:  E F G H I J K L M N
50 - 59:  O P Q R S T U V W X
60 - 69:  Y Z . - : + = ^ ! /
70 - 79:  * ? & < > ( ) [ ] {
80 - 84:  } @ % $ #
```

To encode a frame, an implementation SHALL take four octets at a time from the binary frame and convert them into five printable characters. The four octets SHALL be treated as an unsigned 32-bit integer in network byte order (big endian). The five characters SHALL be output from most significant to least significant (big endian).

To decode a string, an implementation SHALL take five characters at a time from the string and convert them into four octets of data representing a 32-bit unsigned integer in network byte order. The five characters SHALL each be converted into a value 0 to 84, and accumulated by multiplication by 85, from most to least significant.

## Test Case

As a test case, a frame containing these 8 bytes:

```
+------+------+------+------+------+------+------+------+
| 0x86 | 0x4F | 0xD2 | 0x6F | 0xB5 | 0x59 | 0xF7 | 0x5B |
+------+------+------+------+------+------+------+------+
```

SHALL encode as the following 10 characters:

```
+---+---+---+---+---+---+---+---+---+---+
| H | e | l | l | o | W | o | r | l | d |
+---+---+---+---+---+---+---+---+---+---+
```

## Reference Implementation

A reference implementation in C is provided in the RFC repository at https://github.com/zeromq/rfc/blob/master/src/spec_32.c.

## Security Considerations

Implementations MUST take care that buffers intended to receive encoded and decoded data are large enough. Clearly, a properly-crafted encoded string can create any arbitrary binary sequence, which makes Z85 an easy vector for attack into a badly-designed implementation.
