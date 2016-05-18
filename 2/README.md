SPB is a minimalist format for framing opaque binary blobs.  The blobs have no properties and no content structure, and are represented as a size field followed by a blob content.

* Name: http://rfc.zeromq.org/spec:2/SPB
* Editor: Pieter Hintjens <ph@imatix.com>

## License

Copyright (c) 2009-10 iMatix Corporation

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

SPB is designed to be a portable optimal wire-framing format for opaque blobs of data carried over streaming protocols such as TCP/IP.

## Architecture

A SPB frame consists of a frame length, extension point, and frame data.  The size of the frame data MUST correspond to the frame length.

For frames of 0 to 254 octets, the length is represented by single octet.

```
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Message size  | Extensions    |  Message body ...             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Message body ...
+-+-+-+-+-+-+-...

```

For frames of 255 or more octets the length is represented by a single octet %xFF followed by a 64-bit unsigned integer length in network byte order.

```
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
| 0xff          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-...+-+
| Message size (8 bytes)                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-...+-+
| Extensions    |  Message body ...                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Message body ...
+-+-+-+-+-+-+-+...
```

SPB is defined by this grammar:

```
frame      = length extensions data
length     = OCTET | escape 8*OCTET
escape     = %xFF
extensions = %x00
data       = *OCTET
```

## Reference implementation

A reference implementation for SPB can be found in the [0MQ](http://www.zeromq.org) project.

