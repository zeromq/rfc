The Paranoid Pirate Protocol (PPP) defines a reliable request-reply dialog between a client (or client) and a worker peer. PPP covers presence, heartbeating, and request-reply processing. It originated in Chapter 4 of the Guide.

* Name: http://rfc.zeromq.org/spec:6/PPP
* Editor: Pieter Hintjens <ph@imatix.com>

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

PPP implements a reliable request-reply dialog between a client and a worker peer. PPP covers presence, heartbeating, and request-reply processing. It originated in Chapter 4 of the Guide.

The goals of PPP are to:

* Allow both peers to detect disconnection of the other peer, through the use of heartbeating.
* Allow the client to implement a "least recently used" pattern for task distribution to workers.
* Allow the client to recover from dead or disconnected workers by resending requests to other workers.

## Architecture

### Roles

PPP defines two types of peer:

* A "client" issues requests for work. Each request is a message, and is treated independently.

* A "worker" handles requests and responds with replies. Each request results in one reply.

### Overall Conversation

PPP connects a single client and a pool of workers. We do not specify which peers connect to which, but usually workers will connect to the client.

The client may be an intermediary (for example, a queue device) or it may be an application that implements PPP directly.

A PPP conversation consists of two intermingled dialogs, as follows ('C' represents the client, 'W' the worker):

```
    Synchronous dialog:             Asynchronous dialogs:
    ---------------------           ---------------------
    W: READY                        Repeat:
    Repeat:                             W: HEARTBEAT
        C: REQUEST                  Repeat:
        W: REPLY                        C: HEARTBEAT
```

Breaking this down:

* The worker initiates the conversation by sending READY to the client.
* The client responds with a REQUEST.
* The worker replies with a REPLY, and this repeats indefinitely.
* The worker sends HEARTBEAT at regular intervals to the client.
* The client sends HEARTBEAT at regular intervals to the worker.

The first message in any conversation MUST be W:READY.

### Command Specifications

: READY : Consists of a 1-part message containing the single byte 0x01.
: HEARTBEAT : Consists of a 1-part message containing the single byte 0x02.
: REQUEST : Consists of a multipart message containing an optional address stack with null terminator, followed by a message content of 1 or more parts.
: REPLY : Consists of a multipart message containing an optional address stack with null terminator, followed by a message content of 1 or more parts.

### Heartbeating Specification

Peers should send heartbeats at regular and agreed-upon intervals. A peer can consider the other peer "disconnected" if no heartbeat arrives within some multiple of that interval (usually 3-5).

If the worker detects that the client has disconnected, it MUST send READY again to start a new conversation.

If the client detects that the worker has disconnected, it SHOULD stop sending it messages of any type.

