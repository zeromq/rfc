---
title: 13/ZMTP
aliases: [/spec:13/ZMTP]
name: ZeroMQ Message Transport Protocol
status: deprecated
editor: Pieter Hintjens <ph@imatix.com>
---

The ZeroMQ Message Transport Protocol (ZMTP) is a transport layer protocol for exchanging messages between two peers over a connected transport layer such as TCP. This document describes ZMTP/1.0 as implemented by the 0MQ/2.x generation of software.

## License

Copyright (c) 2009-2012 iMatix Corporation

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

The ZeroMQ Message Transport Protocol (ZMTP) is a transport layer protocol for exchanging messages between two peers over a connected transport layer such as TCP. This document describes ZMTP/1.0 as implemented by the 0MQ/2.x generation of software.

In theory, ZMTP should allow full interoperability between products that implement it. However, parts of the necessary semantics are defined only in libzmq's code. We hope that over time these semantics will be properly extracted, abstracted, documented, and proven by independent code.

The primary goal of this specification is to allow interoperability between independent stacks and the libzmq stack. A secondary goal is to act as a container and catalyst for this standardization process.

## Architecture

ZMTP consists of these layers:

* A *framing layer* that imposes a size-prefixed regularity on the underlying transport.
* A *connection layer* that allows two peers to exchange messages.
* A *content layer* that define how application data is formatted, according to the socket type.

### The Framing Layer

#### Preamble

The framing layer underlies everything in ZMTP. The basic transport layer (e.g. provided by TCP) is a stream. ZMTP's framing layer turns that into a series of frames, in either direction. Frames are length-specified, so that peers can safely reject frames that are oversized. ZMTP's framing design is optimized for bandwidth and performance.

Framing is used to create structured messages, rather than to break large messages into fragments. ZMTP assumes that peers will read and process all frames of a message, or none at all. Framing is particularly used to separate message content from message address envelopes (see the Content Layer).

The framing layer is consistent no matter what work is happening on the connection. That is, it can be fully interpreted from the information sent on the wire.

#### Specification

A ZMTP **message** consists of 1 or more frames.

A ZMTP frame consists of a **length**, followed by a **flags** field and a frame **body** of (length - 1) octets. *Note: the length includes the flags field, so an empty frame has a length of 1.*

For frames with a length of 1 to 254 octets, the length SHOULD BE encoded as a single octet. The minimum valid length of a frame is 1 octet, thus a length of 0 is invalid and such frames SHOULD be discarded silently.

For frames with lengths of 255 and greater, the length SHALL BE encoded as a single octet with the value 255, followed by the length encoded as a 64-bit unsigned integer in network byte order. For frames with lengths of 1 to 254 octets this encoding MAY be also used.

The **flags** field consists of a single octet containing various control flags. Bit 0 is the least significant bit.

* Bit 0 (MORE): *More frames to follow*. A value of 0 indicates that there are no more frames to follow. A value of 1 indicates that more frames will follow. On messages consisting of a single frame the MORE flag MUST be 0.

* Bits 1-7: *Reserved*. Bits 1-7 are reserved for future use and SHOULD be zero.

The following ABNF grammar defines a ZMTP message:

```
message     = *more-frame final-frame
more-frame  = length more body
final-frame = length final body
length      = OCTET / (%xFF 8OCTET)
more        = %x01
final       = %x00
body        = *OCTET
```

The following diagram shows the layout of a frame with a length of 1 to 254 octets:

```
            +----------------+
 Octet 0    | Length         |
            +----------------+
 Octet 1    | Flags          |
            +----------------+- ... ---------------------+
 Octets 2+  | Body                     Length - 1 octets |
            +------------------ ... ---------------------+
```

The following diagram shows the layout of a frame with length of 255 or more octets:

```
            +----------------+
 Octet 0    | 0xff           |
            +----------------+- ... ---------------------+
 Octets 1-8 | Length                          8 octets   |
            +------------------ ... ---------------------+
 Octet 9    | Flags          |
            +----------------+- ... ---------------------+
 Octets 10+ | Body                     Length - 1 octets |
            +------------------ ... ---------------------+
```

### The Connection Layer

#### Preamble

The connection layer provides a way for peers to identify each other at TCP connection time. A ZMTP connection is equivalent to a TCP connection. If peers disconnect and reconnect, this acts as two separate ZMTP connections.

#### Specification

A ZMTP connection is bidirectional and asynchronous. That is, either peer MAY send a message to the other peer at any time.

Each side of the connection consists of an **greeting** followed by zero or more **contents**. Content messages are formatted according to the *content type*, as explained in the Content Layer specification.

A peer SHALL send a greeting consisting of either an **anonymous** or an **identity** message. An anonymous greeting consists of an empty string. This tells the other peer that the connection has no durability, and all resources associated with it can be deleted when the connection ends. An identity greeting consists of a unique string of 1 to 255 octets. This tells the other peer to associate resources with that identity, and hold them indefinitely when the connection ends.

Identities SHOULD NOT start with a zero octet, which is reserved for peer internal use. Peers MAY reject identities and SHOULD be cautious about the cost of holding resources indefinitely.

The following ABNF grammar defines either direction of a ZMTP connection:

```
connection  = greeting content
greeting    = anonymous / identity
anonymous   = %x01 idflags
identity    = length idflags (%x01-ff) *OCTET
idflags     = %00
```

The flags octet (idflags) of an identity frame SHALL not be validated and SHOULD be set to zero.

### The Content Layer

#### Preamble

The format and semantics of the ZMTP content messages sent across a connection depend on the **content type** of that connection direction, which is not specified in the protocol but must be assumed by both peers.

The following ABNF grammar defines ZMTP content:

```
content     = *broadcast / *addressed / *neutral
```

#### Broadcast Content

**Broadcast content** is used between publishers and subscribers. A publisher SHALL send broadcast content. A subscriber SHALL NOT send any content.

The following ABNF grammar defines ZMTP broadcast content:

```
broadcast   = message
```

The recipient MAY filter messages. Any matching mechanism may be used (prefix, wildcard, regexp). ZMTP does not standardize this, though the current ZeroMQ implementation uses a prefix match.

#### Addressed Content

**Addressed content** is used between peers in a request-reply chain. Any peer MAY send addressed content to any other peer in a request-reply chain.

The following ABNF grammar defines ZMTP addressed content:

```
addressed   = envelope message
envelope    = *more-frame delimiter
delimiter   = %x01 more
```

The envelope SHOULD be used by peers in the following fashion:

* When a peer sends a request to another peer, it should send an envelope that at least contains the delimiter.
* When a peer forwards a request from one peer to another, it should prepend the identity of the original sending peer to the envelope (as an identity frame).
* When a peer accepts a request and responds with a reply, it should unwrap and save the full address envelope up to and including the delimiter, then pass the remaining message to the application, and then re-wrap the application's response with the full address envelope, up to and including the delimiter.

In this way, a chain of peers can push addresses onto the envelope as they forward requests, and pop addresses off the envelope to route replies back.

#### Neutral Content

**Neutral content** is used between peers that do not require routing. Either peer MAY send neutral content to the other peer though specific peer implementations MAY ignore content coming from their peers.

The following ABNF grammar defines ZMTP neutral content:

```
neutral     = message
```

## Full ZMTP Grammar

The following ABNF grammar defines the full ZMTP protocol:

```
zmtp        = *connection

connection  = greeting content
greeting    = anonymous / identity
anonymous   = %x01 idflags
identity    = length idflags (%x01-ff) *OCTET
idflags     = %00

message     = *more-frame final-frame
more-frame  = length more body
final-frame = length final body
length      = OCTET / (%xFF 8OCTET)
more        = %x01
final       = %x00
body        = *OCTET

content     = *broadcast / *addressed / *neutral

broadcast   = message

addressed   = envelope message
envelope    = *more-frame delimiter
delimiter   = %x01 more

neutral     = message
```

## Known Issues

The protocol has no version numbering. This has been fixed in later versions of ZMTP and implementors are recommended to implement at least [ZMTP/2.0](http://rfc.zeromq.org/spec:15).

There is no exchange or validation of content types in the wire format. That is, a peer must *know in advance* the type of content it expects from another peer in order to properly interpret that content as broadcast, addressed, or neutral. This could be resolved by adding a content type indicator to the connection header as proposed above.

The specification of the length field is surprising (and has confused even experts reading the spec). It should not include the flag field, nor other possible header fields. This would allow a zero-length body to be specified with a length of zero.

The restriction of "identities may not start with a binary zero" is inherited from the software implementation and doesn't play an obvious semantic role in ZMTP. This should be clarified.

## Security

ZMTP/1.0 makes no attempt at security, which an application MAY layer on top.
