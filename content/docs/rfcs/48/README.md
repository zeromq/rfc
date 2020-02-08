---
slug: 48
title: 48/RADIO-DISH
name: ZeroMQ Radio-Dish
status: draft
editor: Doron Somech <somdoron@gmail.com>
---

This document specifies the semantics of the ZeroMQ Radio-Dish pattern, which covers the RADIO and DISH socket types. This specification is intended to guide implementations of these socket types so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2020 Doron Somech

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the RADIO-DISH socket types, which together form the ZeroMQ radio-dish pattern. Conforming implementations of these sockets SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

Radio-dish pattern is member of a new family of thread-safe sockets.
The raido-dish pattern is the thread-safe alternative of the pubsub pattern.

The pattern is intended for event and data distribution, usually from a small number of publishers (radios) to a large number of subscribers (dishes), but also from many publishers to a few subscribers.

Group membership checks can either happen on the RADIO or DISH side and depend on the transport protocol and are defined in the relevant documents. For TCP, refer to http://rfc.zeromq.org/spec:37/ZMTP.

In order for the API to be thread-safe sending and receiving messages MUST be atomic and a single API call to receive or send entire msg. Therefore, the radio-dish pattern (and the rest of the thread-safe family) MUST NOT allow multipart messages.

## The RADIO Socket Type

The RADIO socket type provides basic one-way broadcasting to a set of dishes. Over TCP, it does filtering on outgoing messages but nonetheless a message will be sent multiple times over the network to reach multiple dishes. RADIO is used mainly for transient event distribution where stability of the network (e.g. consistently low memory usage) is more important than reliability of traffic.

General behavior:

* MAY be connected to any number of DISH sockets, and SHALL only send messages.
* SHALL maintain a single outgoing message queue for each connected socket.
* SHALL create a queue when initiating an outgoing connection to a socket, and SHALL maintain the queue whether or not the connection is established.
* SHALL create a queue when a socket connects to it. If this socket disconnects, the RADIO socket SHALL destroy its queue and SHALL discard any messages it contains.
* SHOULD constrain queue sizes to a runtime-configurable limit.
* SHALL silently discard any messages that dishes send it.
* MUST be thread-safe and allow sending from multiple threads.
* MUST allow attaching a group to a message in a thread-safe way.

For processing outgoing messages:

* SHALL not modify outgoing messages in any way.
* MAY, depending on the transport, send all messages to all dishes.
* MAY, depending on the transport, send messages only to dishes who have a join the message group.
* SHALL perform a group membership check using exact matching algorithm.
* SHALL silently drop the message if the queue for a dish is full.
* SHALL NOT block on sending.

For processing commands:

* SHALL receive join and leave commands from dishes depending on the transport protocol used.
* SHALL NOT deliver these commands to its calling application.

## The DISH Socket Type

The DISH socket type provides a basic one-way listener for a set of radios.

General behavior:

* MAY be connected to any number of DISH sockets, and SHALL only receive messages.
* SHALL maintain a single incoming message queue for each connected radio.
* SHALL create a queue when initiating an outgoing connection to a radio, and SHALL maintain the queue whether or not the connection is established.
* SHALL create a queue when a radio connects to it. If this radio disconnects, the DISH socket SHALL destroy its queue and SHALL discard any messages it contains.
* SHOULD constrain queue sizes to a runtime-configurable limit.
* MUST be thread-safe and allow receiving from multiple threads.
* MUST allow retrieving the group of a message in a thread-safe way.

For processing incoming messages:

* SHALL silently discard messages if the queue for a radio is full.
* SHALL receive incoming messages from its sockets using a fair-queuing strategy.
* SHALL not modify incoming messages in any way.
* MAY, depending on the transport, filter messages according to group membership, using a exact matching algorithm.
* SHALL attach the group to the message.
* SHALL deliver messages to its calling application.

For processing commands:

* MAY send join and leave commands to radios depending on the transport protocol used.

## Group 

* Length MUST be between 0 to 255 bytes.
* Group character MUST be one byte with allowed values 1-255.

```
group = 0*255group-char 
group-char = %d1-255
```

## Security Aspects

This specification has no security aspects.
