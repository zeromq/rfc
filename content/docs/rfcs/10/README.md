---
slug: 10
title: 10/FLP
name: Freelance Protocol
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

The Freelance Protocol (FLP) defines brokerless reliable request-reply dialogs across an N-to-N network of clients and servers. It originated in Chapter 4 of the Guide.

## License

Copyright (c) 2011 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

The Freelance Protocol (FLP) defines brokerless reliable request-reply dialogs across an N-to-N network of clients and servers. It originated in Chapter 4 of the Guide. (see "[ØMQ - The Guide](http://zguide.zeromq.org)")

The goals of FLP are:

* To enable an N-to-N network of clients and servers, connected to each other in peer-to-peer fashion
* To operate without an intermediary broker or devices.
* To enable multi-threaded server implementations.
* To enable server failover and recovery.

## Architecture

### Overall Operation

FLP connects a (normally large) set of clients with a (normally small) set of servers, each capable of replacing the others. Though clients may prioritise servers (primary, secondary, etc.) this is irrelevant to FLP.

In the Freelance pattern, clients connect to servers, and address them explicitly. Servers can only reply to clients that have first sent a command.

Clients can issue two types of commands:

* **Ping commands**, which are formatted as a single frame containing the four characters "PING". The server MUST respond with a message containing the four characters "PONG".
* **Request commands**, which are formatted as two or more frames, where the first frame is the Client Control Frame (CCF), and the following frames are the request body. The server MUST return the CCF unmodified, followed by one or more frames of reply body.

Clients MAY use the CCF for any purpose, including request sequence numbering.

### Request and Reply Routing

Both clients and servers MUST use ROUTER (XREP) sockets. From the ØMQ Reference Manual (see "[]()"):

> When receiving messages a ROUTER (XREP) socket shall prepend a message part containing the identity of the originating peer to the message before passing it to the application. When sending messages a ROUTER (XREP) socket shall remove the first part of the message and use it to determine the identity of the peer the message shall be routed to.

This extra frame is ignored in this discussion.

Clients use *transient* sockets, and MUST not set an identity. Servers use *durable* sockets and MUST set an identity.

Server identities are their *public endpoints*. This is the address string that clients will use to connect to the server, e.g. "tcp://192.168.55.162:5055".

### Server Reliability

Clients MAY send ping commands at regular intervals. A client SHOULD consider a server "disconnected" if no "pong" arrives within some multiple of that interval (usually 2-3).
