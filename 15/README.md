---
domain: rfc.zeromq.org
shortname: 15/ZMTP
name: ZeroMQ Message Transport Protocol
status: deprecated
editor: Pieter Hintjens <ph@imatix.com>
contributors:
  - Martin Hurton <mh@imatix.com>
  - Paul Colomiets <paul@colomiets.name>
---

The ZeroMQ Message Transport Protocol (ZMTP) is a transport layer protocol for exchanging messages between two peers over a connected transport layer such as TCP. This document describes ZMTP/2.0.

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

The ZeroMQ Message Transport Protocol (ZMTP) is a transport layer protocol for exchanging messages between two peers over a connected transport layer such as TCP. This document describes ZMTP/2.0.

In theory, ZMTP should define fully interoperable behavior between implementations. However, parts of the necessary semantics are defined only in libzmq's code and reference manual. We hope that over time these semantics will be properly extracted, abstracted, documented, and proven by independent code.

## Implementation

### Formal Grammar

The following ABNF grammar defines the ZMTP/2.0 protocol:

```
zmtp        = *connection

connection  = greeting *message

greeting    = signature revision socket-type identity
signature   = %xFF 8%x00 %x7F
revision    = %x01

socket-type = PAIR | PUB | SUB | REQ | REP | DEALER | ROUTER | PULL | PUSH
PAIR        = %X00
PUB         = %X01
SUB         = %X02
REQ         = %X03
REP         = %X04
DEALER      = %X05
ROUTER      = %X06
PULL        = %X07
PUSH        = %X08

identity    = final-short body
final-short = %x00 OCTET
body        = *OCTET

message     = *more-frame final-frame
final-frame = final body
final       = final-short | final-long
final-long  = %x02 8OCTET
more-frame  = more body
more        = more-short | more-long
more-short  = %x01 OCTET
more-long   = %x03 8OCTET
```

### Framing

ZMTP delimits the TCP stream as 'frames'. A message can consist of multiple frames, for purposes of structuring. A frame consists of a flags field, followed by a length field and a frame body of length octets. The length does not include the flags field, nor itself, so an empty frame has a length of zero.

The flags field consists of a single octet containing various control flags. Bit 0 is the least significant bit (rightmost bit):

* Bit 0 (MORE): *More frames to follow*. A value of 0 indicates that there are no more frames to follow. A value of 1 indicates that more frames will follow. On messages consisting of a single frame the MORE bit MUST be 0.

* Bit 1 (LONG): *Long message*. A value of 0 indicates that the message length is encoded as a single octet. A value of 1 indicates that the message length is encoded as a 64-bit unsigned integer in network byte order.

* Bits 2-7: *Reserved*. Bits 2-7 are reserved for future use and MUST be zero.

The following diagram shows the layout of a final frame with a length of 0 to 255 octets:

```
            +-----------------+
 Octet 0    | 0 0 0 0 0 0 0 0 |
            +-----------------+
 Octet 1    | Length          |
            +-----------------+- ... -----------------+
 Octets 2+  | Body                      Length octets |
            +------------------- ... -----------------+
```

The following diagram shows the layout of a final LONG frame:

```
            +-----------------+
 Octet 0    | 0 0 0 0 0 0 1 0 |
            +-----------------+
 Octets 1-8 | Length                       8 octets   |
            +------------------ ... ------------------+
 Octets 9+  | Body                      Length octets |
            +------------------ ... ------------------+
```

### Socket Compatibility

The implementation SHOULD enforce that an incoming connection has a valid socket type, depending on the socket type of the socket receiving the connection:

* PAIR accepts connections from PAIR.
* PUB accepts connections from SUB.
* SUB accepts connections from PUB.
* REQ accepts connections from REP or ROUTER.
* REP accepts connections from REQ or DEALER.
* DEALER accepts connections from REP, DEALER, or ROUTER.
* ROUTER accepts connections from REQ, DEALER, or ROUTER.
* PULL accepts connections from PUSH.
* PUSH accepts connections from PULL.

Any other socket combinations SHOULD be handled by silently disconnecting the other peer and possibly logging the error for debugging purposes.

### Publish-Subscribe

XPUB and XSUB sockets are implemented at the protocol level as PUB and SUB sockets. That is, XPUB and XSUB are API constructs only. A SUB socket sends a subscription message as one byte %x01 followed by the subscription body, and unsubscription messages as one byte %x00 followed by the subscription body.

### Backwards Interoperability

ZMTP/2.0 uses a single octet to indicate the protocol revision number. ZMTP/2.0 is considered revision 1 of the protocol. ZMTP/1.0 (see "[13/ZMTP - ZeroMQ Message Transport Protocol](http://rfc.zeromq.org/spec:13)") did not have any version information. However, implementations can detect and interoperate with ZMTP/1.0 implementations.

If an implementation does not want backwards compatibility with ZMTP/1.0 peers, it should use the signature defined in the grammar, above. To detect and interoperate with a ZMTP/1.0 peer, an implementation should immediately after opening a TCP socket:

* Send a 10-octet signature consisting of "%xFF length %x7F" where 'length' is the length of the sender's identity (0 or more octets) plus 1. The length MUST be 8 octets in network byte order.

* Read the first octet from the other peer.

* If this octet is not %FF, then the other peer is using ZMTP/1.0.

* If this octet is %FF, then we read nine further octets, and inspect the last octet (the 10th). If the least significant bit is 0, then the other peer is using ZMTP/1.0.

* If the least significant bit is not 0, the peer is using ZMTP/2.0 or a later version. We read two further octets, which indicate the protocol revision, and the socket type of the other peer. We then encode/decode all further frames on that connection using the ZMTP/2.0 framing syntax.

* When we have detected a ZMTP/1.0 peer, we have already sent 10 octets, which the other peer interprets as the start of an identity frame. We continue by sending the body of the identity frame (zero or more octets). From then, we encode and decode all frames on that connection using the ZMTP/1.0 framing syntax.

## Security

ZMTP/2.0 makes no attempt at security, which an application MAY layer on top.
