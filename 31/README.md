This document specifies the semantics of the ZeroMQ exclusive pair pattern, which covers the PAIR socket type. This specification is intended to guide implementations of this socket type so that users can depend on reliable semantics.

* Name: http://rfc.zeromq.org/spec:31/EXPAIR
* Editor: Pieter Hintjens <ph@imatix.com>

## Preamble

Copyright (c) 2013 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the PAIR socket type, which forms the ZeroMQ exclusive pair pattern. Conforming implementations of this socket type SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

PAIR is not a general-purpose socket but is intended for specific use cases where the two peers are architecturally stable. This usually limits PAIR to use within a single process, for inter-thread communication.

## The PAIR Socket Type

General behavior:

* MAY be connected to at most one PAIR peers, and MAY both send and receive messages.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for its peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the PAIR socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing outgoing messages:

* SHALL consider its peer as available only when it has a outgoing queue that is not full.
* SHALL block on sending, or return a suitable error, when it has no available peer.
* SHALL not accept further messages when it has no available peer.
* SHALL NOT discard messages that it cannot queue.

For processing incoming messages:

* SHALL receive incoming messages from its single peer if it has one.
* SHALL deliver these to its calling application.

## Security Aspects

This specification has no security aspects.
