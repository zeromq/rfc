---
slug: 49
title: 49/SCATTERGATHER
name: ZeroMQ Scatter-Gather
status: draft
editor: Doron Somech <somdoron@gmail.com>
---

This document specifies the semantics of the ZeroMQ scatter-gather pattern, which covers the SCATTER and GATHER socket types. This specification is intended to guide implementations of these socket types so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2020 Doron Somech

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the SCATTER and GATHER socket types, which together form the ZeroMQ scatter-gather pattern. Conforming implementations of these sockets SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

Scatter-gather pattern is member of a new family of thread-safe sockets.
The scatter-gather pattern is the thread-safe alternative of the pipeline pattern.

The pattern is intended for task distribution, typically in a multi-stage pipeline where one or a few nodes push work to many workers, and they in turn push results to one or a few collectors. The pattern is mostly reliable insofar as it will not discard messages unless a node disconnects unexpectedly. It is scalable in that nodes can join at any time.

In order for the API to be thread-safe sending and receiving messages MUST be atomic and a single API call to receive or send entire message. Therefore, the scatter-gather pattern (and the rest of the thread-safe family) MUST NOT allow multipart messages.

## The SCATTER Socket Type

The SCATTER socket type talks to a set of anonymous GATHER peers, sending messages using a round-robin algorithm.

General behavior:

* MAY be connected to any number of GATHER peers, and SHALL only send messages.
* SHALL not filter or modify outgoing messages in any way.
* SHALL maintain an outgoing message queue for each connected peer.
* SHALL create this queue when initiating an outgoing connection to a peer, and SHALL maintain the queue whether or not the connection is established.
* SHALL create this queue when a peer connects to it. If this peer disconnects, the SCATTER socket SHALL destroy its queue and SHALL discard any messages it contains.
* SHOULD constrain queue sizes to a runtime-configurable limit.
* MUST be thread-safe and allow sending from multiple threads.

For processing outgoing messages:

* SHALL consider a peer as available only when it has a outgoing queue that is not full.
* SHALL route outgoing messages to available peers using a round-robin strategy.
* SHALL block on sending, or return a suitable error, when it has no available peers.
* SHALL not accept further messages when it has no available peers.
* SHALL NOT discard messages that it cannot queue.
* MUST NOT send multi-part messages.

## The GATHER Socket Type

The GATHER socket type talks to a set of anonymous SCATTER peers, receiving messages using a fair-queuing algorithm.

General behavior:

* MAY be connected to any number of SCATTER peers, and SHALL only receive messages.
* SHALL not filter or modify incoming messages in any way.
* SHALL maintain an incoming queue for each connected peer.
* SHALL create this queue when initiating an outgoing connection to a peer, and SHALL maintain the queue whether or not the connection is established.
* SHALL create this queue when a peer connects to it. If this peer disconnects, the GATHER socket SHALL destroy its queue and SHALL discard any messages it contains.
* SHOULD constrain incoming queue sizes to a runtime-configurable limit.
* MUST be thread-safe and allow receiving from multiple threads.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL deliver these to its calling application.
* MUST discard any part of a multipart message, MAY disconnect a peer that is sending multipart messages.

## Security Aspects

This specification has no security aspects.
