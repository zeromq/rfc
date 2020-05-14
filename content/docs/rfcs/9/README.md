---
slug: 9
title: 9/TSP
name: Titanic Service Protocol
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

The Titanic Service Protocol (TSP) defines a set of services, requests, and replies that implement the Titanic pattern for disconnected persistent messaging across a network of arbitrarily connected clients and workers.

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

The Titanic Service Protocol (TSP) defines a set of services, requests, and replies that implement the Titanic pattern for disconnected persistent messaging across a network of arbitrarily connected clients and workers.

The Titanic pattern is developed in Chapter 4 of the Guide (see "[ØMQ - The Guide](http://zguide.zeromq.org)"), as a simple design for disk-based reliable messaging. Titanic allows clients and workers to work without being connected to the network at the same time, and defines handshaking for safe storage of requests, and retrieval of replies.

TSP aims to formally specify the interface between client applications and arbitrary implementations of the Titanic pattern.

## Architecture

### Overview

Titanic is a layer built on top of the Majordomo Protocol ([/spec:7/MDP 7/MDP]). TSP clients use MDP/Client to talk to an MDP broker. Titanic requires no modifications to workers, which use the MDP/Worker protocol to speak to an MDP broker.

The Titanic pattern places the persistence outside the broker, as a proxy service that looks like a worker to clients, and a client to workers:

![Figure](/rfcs/9/1.png)

### Services

A Titanic implementation (the "server") MUST implement these services:

* **titanic.request** - whereby a client asks the server to store a new request.
* **titanic.reply** - whereby a client queries the server for a reply, if available.
* **titanic.close** - whereby a client tells the server that it has finished processing a reply.

#### The titanic.request Service

The titanic.request service accepts a request, stores it persistently, and returns a UUID (universally unique identifier) for the request. It a multipart request message with 2 or more frames, as follows:

* Frame 0: Service name (printable string)
* Frames 1+: Request body (opaque binary)

Note that this request message is carried *over MDP*. The service name is the target service, for example "echo". The titanic.request service MUST reply with 1 or more frames, as follows:

* Frame 0: Status code (explained below)
* Frame 1: UUID for request, if OK

The status code MUST be one of the codes listed below in the section "Status Frames". The UUID MUST be formatted as 32 hexadecimal characters ('0' to '9' and 'A' to 'Z' or 'a' to 'z').

#### The titanic.reply Service

The titanic.reply service accepts a UUID and if a reply exists for that UUID, returns the reply message. It accepts a request message with 1 frame, as follows:

* Frame 0: UUID (as returned by titanic.request)

The titanic.reply service MUST reply with 1 or more frames, as follows:

* Frame 0: Status code (explained below)
* Frames 1+: Request body (opaque binary), if OK

The status code MUST be one of the codes listed below in the section "Status Frames". The UUID MUST be formatted as 32 printable hexadecimal characters ('0' to '9' and 'A' to 'Z' or 'a' to 'z').

The titanic.reply service is idempotent and MUST NOT delete a reply when successfully delivered to the client. Multiple requests to titanic.reply with the same UUID should result in the same response back to the client, until and unless the request is executed. See "Request Execution" below.

#### The titanic.close Service

The titanic.close service accepts a UUID and deletes any request and reply for that UUID. It accepts a request message with 1 frame, as follows:

* Frame 0: UUID (as returned by titanic.request)

The titanic.close service MUST reply with 1 frame, as follows:

* Frame 0: Status code (explained below)

The status code MUST be one of the codes listed below in the section "Status Frames". The UUID MUST be formatted as 32 hexadecimal characters ('0' to '9' and 'A' to 'Z' or 'a' to 'z').

The titanic.close service is idempotent and MUST return "200 OK" if the UUID is not known.

### Request Execution

The server executes requests asynchronously by sending them to the broker as and when services are available. TSP defines no performance characteristics.

### Status Frames

Every reply from a TSP service contains a status frame followed by zero or more content frames. The status frame contains a string formatted as three digits, optionally followed by a space and descriptive text. A client MUST NOT treat the text as significant in any way. Implementations MAY NOT use status codes that are not defined here:

* 200 - OK. The TSP service executed the request successfully. For a titanic.reply service, this additionally means that the 'real' service executed successfully.
* 300 - PENDING. The client SHOULD retry the request at a later time.
* 400 - UNKNOWN. The client is using an invalid or unknown UUID and SHOULD NOT retry.
* 500 - ERROR. The server cannot complete the request due to some internal error. The client SHOULD retry at some later time.

### Reference Implementations

The C99 Titanic examples from Chapter 4 of the Guide (see "[ØMQ - The Guide](http://zguide.zeromq.org)") act as the prime reference implementation for TSP. Translations of the examples into other languages may be available.
