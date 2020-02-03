---
title: 41/CLISRV
aliases: [/spec:41/CLISRV]
status: draft
editor: Pieter Hintjens <ph@imatix.com>
---

This document specifies the semantics of the ZeroMQ request-reply pattern, which covers the CLIENT and SERVER socket types. This specification is intended to guide implementations of these socket types so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2015 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the CLIENT and SERVER socket types, which together form the ZeroMQ client-server pattern. Conforming implementations of these sockets SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

The client-server pattern is intended for service-oriented architectures of various kinds. It provides an asynchronous two-way message flow between a CLIENT and a SERVER socket. All flows are initiated by the CLIENT. A flow consists of at least one request, followed by zero or more replies. The CLIENT-SERVER pattern is a building block for higher-level protocols.

This pattern is meant to deprecate and eventually replace the request-reply pattern.

## The CLIENT Socket Type

The CLIENT socket type talks to one or more SERVER peers. If connected to multiple peers, it scatters sent messages among these peers in a round-robin fashion. On reading, it reads fairly, from each peer in turn. It is reliable, insofar as it does not drop messages in normal cases.

General behavior:

* MAY be connected to any number of SERVER peers, and MAY both send and receive messages.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for each connected peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the CLIENT socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing outgoing messages:

* SHALL consider a peer as available only when it has a outgoing queue that is not full.
* SHALL route outgoing messages to available peers using a round-robin strategy.
* SHALL block on sending, or return a suitable error, when it has no available peers.
* SHALL not accept further messages when it has no available peers.
* SHALL NOT discard messages that it cannot queue.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL deliver these to its calling application.

## The SERVER Socket Type

The SERVER socket type talks to zero or more CLIENT peers. Each outgoing message is sent to a specific peer CLIENT.

General behavior:

* MAY be connected to any number of CLIENT peers, and MAY both send and receive messages.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for each connected peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the SERVER socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHALL identify each double queue using a unique "routing id", which is a non-zero 32-bit unsigned integer value.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL deliver the resulting messages to its calling application.
* SHALL provide the application with an API to retrieve the message routing-id.

For processing outgoing messages:

* SHALL provide the application with an API to set the message routing-id.
* SHALL route the message to the outgoing queue if that queue exists, and is not full.
* SHALL return an error if the queue does not exist.
* SHALL block on sending, if the queue is full, unless otherwise configured.
* SHALL NOT discard messages that it cannot queue.

## Security Aspects

This specification has no security aspects.
