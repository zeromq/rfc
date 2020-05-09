---
slug: 47
title: 47/CLIENTSERVER
name: ZeroMQ Client-Server
status: Draft
editor: Doron Somech <somdoron@gmail.com>
---

This document specifies the semantics of the ZeroMQ client-server pattern, which covers the CLIENT AND SERVER socket types. This specification is intended to guide implementations of these socket types so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2020 Doron Somech

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).


## Goals

This specification is intended to formally document the names and expected behaviour of the CLIENT & SERVER socket types. Conforming implementations of these sockets SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

Client-server pattern is member of a new family of thread-safe sockets.
The client-server pattern is the thread-safe alternative of the router-dealer pattern.
The client-server pattern is intended for service-oriented architectures of various kinds and it is asynchronous (like the router-dealer pattern). It provides an asynchronous two-way message flow between a CLIENT and a SERVER socket.

In order for the API to be thread-safe sending and receiving messages MUST be atomic and a single API call to receive or send entire msg. Therefore, the client-server pattern (and the rest of the thread-safe family) MUST NOT allow multipart messages.

## The CLIENT Socket Type

The CLIENT socket type talks to one or more SERVER peers. If connected to multiple peers, it scatters sent messages among these peers in a round-robin fashion. On reading, it reads fairly, from each peer in turn. It is reliable, insofar as it does not drop messages in normal cases.

General behavior:

* MAY be connected to any number of SERVER peers, and MAY both send and receive messages.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for each connected peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the CLIENT socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.
* MUST be thread-safe and allow receiving and sending from multiple threads.

For processing outgoing messages:

* SHALL consider a peer as available only when its outgoing queue is not full.
* SHALL route outgoing messages to available peers using a round-robin strategy.
* SHALL block on sending, or return a suitable error, when it has no available peers.
* SHALL not accept further messages when it has no available peers.
* SHALL NOT discard messages that it cannot queue.
* MUST NOT send multipart messages.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL deliver these to its calling application.
* MUST discard any part of a multipart message.
* MAY disconnect a peer that is sending multipart messages.

## The SERVER Socket Type

The SERVER socket type talks to zero or more CLIENT peers, using an explicit routing-id so that each outgoing message is sent to a specific peer CLIENT.

General behavior:

* MAY be connected to any number of CLIENT peers, and MAY both send and receive messages.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for each connected peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the SERVER socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHALL identify each double queue using a unique "routing id", which is a non-zero 32-bit unsigned integer value.
* SHALL NOT allow the peer to specify its routing id explicitly.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL deliver the resulting messages to its calling application.
* SHALL provide the application with an API to retrieve the message routing-id.
* MUST discard any part of a multipart message.
* MAY disconnect a peer that is sending multipart messages.

For processing outgoing messages:

* SHALL provide the application with an API to set the message routing-id.
* SHALL route the message to the outgoing queue if that queue exists, and is not full.
* SHALL return an error if the queue does not exist.
* SHALL block on sending, if the queue is full, unless otherwise configured.
* SHALL NOT discard messages that it cannot queue.
* MUST NOT send multi-part messages.

## Security Aspects

This specification has no security aspects.
