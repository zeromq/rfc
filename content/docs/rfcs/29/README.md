---
slug: 29
title: 29/PUBSUB
name: ZeroMQ Publish-Subscribe
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

This document specifies the semantics of the ZeroMQ publish-subscribe pattern, which covers the PUB, XPUB, SUB, and XSUB socket types. This specification is intended to guide implementations of these socket types so that users can depend on reliable semantics.

## Preamble

Copyright (c) 2013-2014 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

This specification is intended to formally document the names and expected behaviour of the PUB, XPUB, SUB, and XSUB socket types, which together form the ZeroMQ publish-subscribe pattern. Conforming implementations of these sockets SHOULD respect this specification, thus ensuring that applications can depend on predictable behavior. This specification is not transport specific, but not all behaviour will be reproducible on all transports.

## Overall Goals of this Pattern

The pattern is intended for event and data distribution, usually from a small number of publishers to a large number of subscribers, but also from many publishers to a few subscribers. For many-to-many use-cases the pattern provides raw socket types (XPUB, XSUB) to construct distribution proxies, also called brokers.

The exact subscription and filtering mechanisms depend on the transport protocol and are defined in the relevant documents. For TCP, refer to http://rfc.zeromq.org/spec:23/ZMTP.

## The PUB Socket Type

The PUB socket type provides basic one-way broadcasting to a set of subscribers. Over TCP, it does filtering on outgoing messages but nonetheless a message will be sent multiple times over the network to reach multiple subscribers. PUB is used mainly for transient event distribution where stability of the network (e.g. consistently low memory usage) is more important than reliability of traffic.

General behavior:

* MAY be connected to any number of SUB or XSUB subscribers, and SHALL only send messages.
* SHALL maintain a single outgoing message queue for each connected subscriber.
* SHALL create a queue when initiating an outgoing connection to a subscriber, and SHALL maintain the queue whether or not the connection is established.
* SHALL create a queue when a subscriber connects to it. If this subscriber disconnects, the PUB socket SHALL destroy its queue and SHALL discard any messages it contains.
* SHOULD constrain queue sizes to a runtime-configurable limit.
* SHALL silently discard any messages that subscribers send it.

For processing outgoing messages:

* SHALL not modify outgoing messages in any way.
* MAY, depending on the transport, send all messages to all subscribers.
* MAY, depending on the transport, send messages only to subscribers who have a matching subscription.
* SHALL perform a binary comparison of the subscription against the start of the first frame of the message.
* SHALL silently drop the message if the queue for a subscriber is full.
* SHALL NOT block on sending.

For processing subscriptions:

* SHALL receive subscribe and unsubscribe requests from subscribers depending on the transport protocol used.
* SHALL NOT deliver these commands to its calling application.

## The XPUB Socket Type

The XPUB socket type extends the PUB socket with the ability to receive messages from anonymous subscribers, and the exposure of subscription commands to the application. XPUB is usually used in proxies but is also useful for advanced applications.

General behavior:

* MAY be connected to any number of SUB or XSUB subscribers, and MAY both send and receive messages.
* SHALL maintain a double queue for each connected subscriber, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a subscriber, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a subscriber connects to it. If this subscriber disconnects, the XPUB socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing outgoing messages:

* SHALL not modify outgoing messages in any way.
* MAY, depending on the transport, send all messages to all subscribers.
* MAY, depending on the transport, send messages only to subscribers who have a matching subscription.
* SHALL perform a binary comparison of the subscription against the start of the first frame of the message.
* SHALL silently drop the message if the queue for a subscriber is full.
* SHALL NOT block on sending.

For processing incoming messages:

* SHALL receive incoming messages from its subscribers using a fair-queuing strategy.
* SHALL deliver these messages to its calling application.

For processing subscriptions:

* SHALL receive subscribe and unsubscribe requests from subscribers depending on the transport protocol used.
* SHALL deliver these commands to its calling application.
* MAY, depending on configuration, normalize commands delivered to its calling application so that multiple identical subscriptions result in a single command only.
* SHALL, if the subscriber peer disconnects prematurely, generate a suitable unsubscribe request for the calling application.

## The SUB Socket Type

The SUB socket type provides a basic one-way listener for a set of publishers.

General behavior:

* MAY be connected to any number of PUB or XPUB publishers, and SHALL only receive messages.
* SHALL maintain a single incoming message queue for each connected publisher.
* SHALL create a queue when initiating an outgoing connection to a publisher, and SHALL maintain the queue whether or not the connection is established.
* SHALL create a queue when a publisher connects to it. If this publisher disconnects, the SUB socket SHALL destroy its queue and SHALL discard any messages it contains.
* SHOULD constrain queue sizes to a runtime-configurable limit.

For processing incoming messages:

* SHALL silently discard messages if the queue for a publisher is full.
* SHALL receive incoming messages from its publishers using a fair-queuing strategy.
* SHALL not modify incoming messages in any way.
* MAY, depending on the transport, filter messages according to subscriptions, using a prefix match algorithm.
* SHALL deliver messages to its calling application.

For processing subscriptions:

* MAY send subscribe and unsubscribe requests to publishers depending on the transport protocol used.

## The XSUB Socket Type

The XSUB socket type extends the SUB socket with the ability to send messages and subscription commands upstream to publishers. XSUB is usually used in proxies but is also useful for advanced applications.

General behavior:

* MAY be connected to any number of PUB or XPUB publishers, and MAY both send and receive messages.
* SHALL maintain a double queue for each connected publisher, allowing outgoing and incoming messages to be queued independently.
* SHALL create a double queue when initiating an outgoing connection to a publisher, and SHALL maintain the double queue whether or not the connection is established.
* SHALL create a double queue when a publisher connects to it. If this publisher disconnects, the XSUB socket SHALL destroy its double queue and SHALL discard any messages it contains.
* SHOULD constrain incoming and outgoing queue sizes to a runtime-configurable limit.

For processing incoming messages:

* SHALL not modify incoming messages in any way.
* SHALL silently discard messages if the incoming queue for a publisher is full.
* SHALL receive incoming messages from its publishers using a fair-queuing strategy.
* MAY, depending on the transport, filter messages according to subscriptions, using a prefix match algorithm.
* SHALL deliver messages to its calling application.

For processing outgoing messages:

* SHALL not modify outgoing messages in any way.
* SHALL send all messages to all connected publishers.
* SHALL silently drop the message if the outgoing queue for a publisher is full.
* SHALL NOT block on sending.

For processing subscriptions:

* SHALL accept subscription commands from its calling application.
* SHALL send subscribe and unsubscribe requests to publishers depending on the transport protocol used.
* When closing a connection to a publisher SHOULD send unsubscribe requests for all subscriptions.

## Security Aspects

This specification has no security aspects.
