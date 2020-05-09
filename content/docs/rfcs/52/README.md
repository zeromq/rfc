---
slug: 52
title: 52/CHANNEL
name: ZeroMQ Channel
status: Draft
editor: Doron Somech <somdoron@gmail.com>
---

This document specifies the semantics of the ZeroMQ channel pattern, which covers the CHANNEL socket type. This specification is intended to guide implementations of this socket type so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2020 Doron Somech.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the CHANNEL socket type, which forms the ZeroMQ channel pattern. Conforming implementations of this socket type SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

Channel pattern is member of a new family of thread-safe sockets.
The channel pattern is the thread-safe alternative of the exclusive pair pattern.
CHANNEL is not a general-purpose socket but is intended for specific use cases where the two peers are architecturally stable. This usually limits CHANNEL to use within a single process, for inter-thread communication.

In order for the API to be thread-safe sending and receiving messages MUST be atomic and a single API call to receive or send entire message. Therefore, the client-server pattern (and the rest of the thread-safe family) MUST NOT allow multipart messages.

## The CHANNEL Socket Type

General behavior:

* MAY be connected to at most one CHANNEL peer.
* MAY both send and receive messages in any order.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for its peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the CHANNEL socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.
* MUST be thread-safe and allow receiving and sending from multiple threads.

For processing outgoing messages:

* SHALL consider its peer as available only when it has a outgoing queue that is not full.
* SHALL block on sending, or return a suitable error, when it has no available peer.
* SHALL not accept further messages when it has no available peer.
* SHALL NOT discard messages that it cannot queue.
* MUST NOT send multipart messages.

For processing incoming messages:

* SHALL receive incoming messages from its single peer if it has one.
* SHALL deliver these to its calling application.
* MUST discard any part of a multipart message.

## Security Aspects

This specification has no security aspects.
