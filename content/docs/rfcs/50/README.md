---
slug: 50
title: 50/MME
name: ZeroMQ Multipart Message Encoding
status: draft
editor: Gudmundur F. Adalsteinsson <ofpgummi@yahoo.com>
---

This document specifies the semantics of encoding and decoding multiple ZeroMQ messages as a single message.

## Preamble

Copyright (c) 2020 Gudmundur F. Adalsteinsson

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the wire protocol for encoding and decoding of the content of multiple ZeroMQ messages as a single message. This formalizes the protocol used by `zmsg_encode` and `zmsg_decode` in `czmq`.

## Implementation

A sequence of zero or more messages is serialized as a sequence of serialized frames. A frame is the content of a message part.

A frame of 255 or more octets (large message) is serialized as a single octet 0xFF, followed by the size as an unsigned 32-bit integer, network byte order, followed by the frame. A frame of 254 or less octets (small message) is serialized as the size as a single octect, followed by the frame.

* An encoder MUST support encoding zero or more frames.
* A decoder MUST support decoding zero or more frames.
* A count of zero frames is encoded as a message of size 0.
* The encoder MUST report size overflow for frames sizes exceeding the largest value an unsigned 32-bit integer can hold as an error. The total message size does not have a specified limit.
* The decoder MUST report invalid encodings (underflow or overflow) as an error.
* An implementation MAY encode messages smaller than 255 octets (small message) as a large message.

The following diagram shows the layout of a serialized frame with a length of 0 to 254 octets:

```
            +----------------+
 Octet 0    | Length         |
            +----------------+- ... ---------------------+
 Octets 1+  | Content                      Length octets |
            +------------------ ... ---------------------+
```

The following diagram shows the layout of a serialized frame with length of 255 or more octets:

```
            +----------------+
 Octet 0    | 0xFF           |
            +----------------+- ... ---------------------+
 Octets 1-4 | Length                          4 octets   |
            +------------------ ... ---------------------+
 Octets 5+  | Content                      Length octets |
            +------------------ ... ---------------------+
```

## Limitations

The protocol only supports message parts of size up to 4GiB, which is sufficient for majority of users. If data larger than that must be encoded, then the message part must be split up.

## Security Aspects

This specification has no security aspects.
