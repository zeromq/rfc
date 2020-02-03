---
slug: 28
title: 28/REQREP
aliases: [/spec:28/REQREP]
name: ZeroMQ Request-Reply
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

This document specifies the semantics of the ZeroMQ request-reply pattern, which covers the REQ, REP, DEALER, and ROUTER socket types. This specification is intended to guide implementations of these socket types so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2013 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the REQ, REP, DEALER, and ROUTER socket types, which together form the ZeroMQ request-reply pattern. Conforming implementations of these sockets SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

The request-reply pattern is intended for service-oriented architectures of various kinds. It comes in two basic flavors: synchronous (REQ and REP), and asynchronous (DEALER and ROUTER), which may be mixed in various ways. The DEALER and ROUTER sockets are building blocks for many higher-level protocols such as rfc.zeromq.org/spec:18/MDP.

## The REQ Socket Type

The REQ socket type acts as the client for a set of anonymous services, sending requests and receiving replies using a lock-step round-robin algorithm. It is designed for simple request-reply models where reliability against failing peers is not an issue.

General behavior:

* MAY be connected to any number of REP or ROUTER peers.
* SHALL send and then receive exactly one message at a time.

The request and reply messages SHALL have this format on the wire:

* A delimiter, consisting of an empty frame, added by the REQ socket.
* One or more data frames, comprising the message visible to the application.

For processing outgoing messages:

* SHALL prefix the outgoing message with an empty delimiter frame.
* SHALL route outgoing messages to connected peers using a round-robin strategy.
* SHALL block on sending, or return a suitable error, when it has no connected peers.
* SHALL NOT discard messages that it cannot send to a connected peer.

For processing incoming messages:

* SHALL accept an incoming message only from the last peer that it sent a request to.
* SHALL discard silently any messages received from other peers.

## The REP Socket Type

The REP socket type acts as as service for a set of client peers, receiving requests and sending replies back to the requesting peers. It is designed for simple remote-procedure call models.

General behavior:

* MAY be connected to any number of REQ or DEALER peers.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL receive and then send exactly one message at a time.

The request and reply messages SHALL have this format on the wire:

* An address envelope consisting of zero or more frames, each containing one identity.
* A delimiter, consisting of an empty frame.
* One or more data frames, comprising the message visible to the application.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL remove and store the address envelope, including the delimiter.
* SHALL pass the remaining data frames to its calling application.

For processing outgoing messages:

* SHALL wait for a single reply message from its calling application.
* SHALL prepend the address envelope and delimiter.
* SHALL deliver this message back to the originating peer.
* SHALL silently discard the reply, or return an error, if the originating peer is no longer connected.
* SHALL not block on sending.

## The DEALER Socket Type

The DEALER socket type talks to a set of anonymous peers, sending and receiving messages using round-robin algorithms. It is reliable, insofar as it does not drop messages. DEALER works as an asynchronous replacement for REQ, for clients that talk to REP or ROUTER servers. It is also used in request-reply proxies.

General behavior:

* MAY be connected to any number of REP or ROUTER peers, and MAY both send and receive messages.
* SHALL not filter or modify outgoing or incoming messages in any way.
* SHALL maintain a double queue for each connected peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the DEALER socket SHALL destroy its double queue and SHALL discard any messages it contains.
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

## The ROUTER Socket Type

The ROUTER socket type talks to a set of peers, using explicit addressing so that each outgoing message is sent to a specific peer connection. ROUTER works as an asynchronous replacement for REP, and is often used as the basis for servers that talk to DEALER clients.

General behavior:

* MAY be connected to any number of REQ, DEALER, or ROUTER peers, and MAY both send and receive messages.
* SHALL maintain a double queue for each connected peer, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a peer, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a peer connects to it. If this peer disconnects, the ROUTER socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHALL identify each double queue using a unique "identity" binary string.
* SHOULD allow the peer to specify its identity explicitly through the Identity metadata property.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing incoming messages:

* SHALL receive incoming messages from its peers using a fair-queuing strategy.
* SHALL prefix each incoming message with a frame containing the identity of the originating double queue.
* SHALL deliver the resulting messages to its calling application.

For processing outgoing messages:

* SHALL remove the first frame from each outgoing message and use this as the identity of a double queue.
* SHALL route the message to the outgoing queue if that queue exists, and has space.
* SHALL either silently drop the message, or return an error, depending on configuration, if the queue does not exist, or is full.
* SHALL NOT block on sending.

## Security Aspects

This specification has no security aspects.
