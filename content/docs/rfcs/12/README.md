---
title: 12/CHP
aliases: [/spec:12/CHP]
name: Clustered Hashmap Protocol
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

The Clustered Hashmap Protocol (CHP) defines a cluster-wide key-value hashmap, and mechanisms for sharing this across a set of clients. CHP allows clients to work with subtrees of the hashmap, to update values, and to define ephemeral values. CHP originated from the Clone pattern defined in Chapter 5 of the Guide.

* Name: http://rfc.zeromq.org/spec:12/CHP
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

CHP is meant to provide a basis for reliable pub-sub across a cluster of clients connected over a 0MQ network. It defines a "hashmap" abstraction consisting of key-value pairs. Any client can modify any key-value pair at any time, and changes are propagated to all clients. A client can join the network at any time.

## Architecture

CHP connects a set of client applications and a set of servers. Clients connect to the server. Clients do not see each other. Clients can come and go arbitrarily.

### Ports and Connections

The server MUST open three ports as follows:

* A SNAPSHOT port (0MQ ROUTER socket) at port number P.
* A PUBLISHER port (0MQ PUB socket) at port number P + 1.
* A COLLECTOR port (0MQ SUB socket) at port number P + 2.

The client SHOULD open at least two connections:

* A SNAPSHOT connection (0MQ DEALER socket) to port number P.
* A SUBSCRIBER connection (0MQ SUB socket) to port number P + 1.

The client MAY open a third connection, if it wants to update the hashmap:

* A PUBLISHER connection (0MQ PUB socket) to port number P + 2.

From the ØMQ Reference Manual (see "[]()"):

> When receiving messages a ROUTER socket shall prepend a message part containing the identity of the originating peer to the message before passing it to the application. When sending messages a ROUTER socket shall remove the first part of the message and use it to determine the identity of the peer the message shall be routed to.

This extra frame is not shown in the commands explained below.

### State Synchronization

The client MUST start by sending a ICANHAZ command to its snapshot connection. This command consists of two frames as follows:

```
ICANHAZ command
-----------------------------------
Frame 0: "ICANHAZ?"
Frame 1: subtree specification
```

Both frames are 0MQ strings. The subtree specification MAY be empty. If not empty, it consists of a slash followed by one or more path segments, ending in a slash.

The server MUST respond to a ICANHAZ command by sending zero or more KVSYNC commands to its snapshot port, followed with a KTHXBAI command. The server MUST prefix each command with the identity of the client, as provided by 0MQ with the ICANHAZ command. The KVSYNC command specifies a single key-value pair as follows:

```
KVSYNC command
-----------------------------------
Frame 0: key, as 0MQ string
Frame 1: sequence number, 8 bytes in network order
Frame 2: <empty>
Frame 3: <empty>
Frame 4: value, as blob
```

The sequence number has no significance and may be zero.

The KTHXBAI command takes this form:

```
KTHXBAI command
-----------------------------------
Frame 0: "KTHXBAI"
Frame 1: sequence number, 8 bytes in network order
Frame 2: <empty>
Frame 3: <empty>
Frame 4: subtree specification
```

The sequence number MUST be the highest sequence number of the KVSYNC commands previously sent.

When the client has received a KTHXBAI command it SHOULD start to receive messages from its subscriber connection, and apply them.

### Server-to-Client Updates

When the server has an update for its hashmap it MUST broadcast this on its publisher socket as a KVPUB command. The KVPUB command has this form:

```
KVPUB command
-----------------------------------
Frame 0: key, as 0MQ string
Frame 1: sequence number, 8 bytes in network order
Frame 2: UUID, 16 bytes
Frame 3: properties, as 0MQ string
Frame 4: value, as blob
```

The sequence number MUST be strictly incremental. The client MUST discard any KVPUB commands whose sequence numbers are not strictly greater than the last KTHXBAI or KVPUB command received.

The UUID is optional and frame 2 MAY be empty (size zero). The properties field is formatted as zero or more instances of "name=value<newline>". If the key-value pair has no properties, the properties field is empty.

If the value is empty, the client SHOULD delete its key-value entry with the specified key.

In the absence of other updates the server SHOULD send a HUGZ command at regular intervals, e.g. once per second. The HUGZ command has this format:

```
HUGZ command
-----------------------------------
Frame 0: "HUGZ"
Frame 1: 00000000
Frame 2: <empty>
Frame 3: <empty>
Frame 4: <empty>
```

The client MAY treat the absence of HUGZ as an indicator that the server has crashed, see Reliability below.

### Client-to-Server Updates

When the client has an update for its hashmap, it MAY send this to the server via its publisher connection as a KVSET command. The KVSET command has this form:

```
KVSET command
-----------------------------------
Frame 0: key, as 0MQ string
Frame 1: sequence number, 8 bytes in network order
Frame 2: UUID, 16 bytes
Frame 3: properties, as 0MQ string
Frame 4: value, as blob
```

The sequence number has no significance and may be zero. The UUID SHOULD be a universally unique identifier, if a reliable server architecture is used.

If the value is empty, the server MUST delete its key-value entry with the specified key.

The server SHOULD accept the following properties:

* "ttl" - specifies a time-to-live in seconds. If the KVSET command has a ttl property, the server SHOULD delete the key-value pair and broadcast a KVPUB with an empty value, to delete this from all clients, when the TTL has expired.

### Reliability

CHP may be used in a dual-server configuration where a backup server takes over if the primary server fails. CHP does not specify the mechanisms used for this failover but the Binary Star pattern from the Guide may be helpful.

To assist server reliability, the client MAY:

* Set a UUID in every KVSET command.
* Detect the lack of HUGZ over time time period and use this as an indicator that the current server has failed.
* Connect to a backup server and re-request a state synchronization.

### Scalability and Performance

CHP is designed to be scalable to large numbers (thousands) of clients, limited only by system resources on the broker. Since all updates pass through a single server the overall throughput will be limited to some millions of updates per second, at peak, and probably less.

### Security

CHP does not implement any authentication, access control, or encryption mechanisms and should not be used in any deployment where these are required.

### Reference Implementations

The C99 Clone examples from Chapter 5 of the Guide (see "[ØMQ - The Guide](http://zguide.zeromq.org)") act as the prime reference implementation for CHP.
