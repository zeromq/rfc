The Majordomo Protocol (MDP) defines a reliable service-oriented request-reply dialog between a set of client applications, a broker and a set of worker applications. MDP covers presence, heartbeating, and service-oriented request-reply processing. It originated from the Majordomo pattern defined in Chapter 4 of the Guide. This is MDP version 0.2, which adds support for multiple replies for a single request.

* Name: http://rfc.zeromq.org/spec:18/MDP
* Editor: Pieter Hintjens <ph@imatix.com>
* Contributors: Chuck Remes <cremes@mac.com>, Guido Goldstein <zmqdev@a-nugget.de>

## License

Copyright (c) 2012 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

The Majordomo Protocol (MDP) defines a reliable service-oriented request-reply dialog between a set of client applications, a broker and a set of worker applications. MDP covers presence, heartbeating, and service-oriented request-reply processing. It originated from the Majordomo pattern defined in Chapter 4 of the Guide (see "[ØMQ - The Guide](http://zguide.zeromq.org)"). The Majordomo pattern has no relation to the open source mailing list software with the same name.

MDP is an evolution of [6/PPP](http://rfc.zeromq.org/spec:6), adding name-based service resolution and more structured protocol commands.

MDP/0.2 is an evolution of MDP/0.1, adding support for multiple replies to a single request. MDP/0.2 is not compatible with MDP/0.1. The significant differences are (a) replacement of REPLY command with PARTIAL and FINAL commands, and (b) removal of empty frames at start of commands.

The goals of MDP are to:

* Allow requests to be routed to workers on the basis of abstract service names.
* Allow both peers to detect disconnection of the other peer, through the use of heartbeating.
* Allow the broker to implement a "least recently used" pattern for task distribution to workers for a given service.
* Allow the broker to recover from dead or disconnected workers by resending requests to other workers.

## Architecture

![Figure](1.png)

### Overall Topology

MDP connects a set of client applications, a single broker device and a pool of workers applications. Clients connect to the broker, as do workers. Clients and workers do not see each other, and both can come and go arbitrarily. The broker MAY open two sockets (ports), one front-end for clients, and one back-end for workers. However MDP is also designed to work over a single broker socket.

We define 'client' applications as those issuing requests, and 'worker' applications as those processing them. MDP makes these assumptions:

* Workers are idempotent, i.e. it is safe to execute the same request more than once.
* Workers will handle at most one request a time, and will issue exactly one reply for each successful request.

The Majordomo broker handles a set of shared request queues, one per service. Each queue has multiple writers (clients) and multiple readers (workers). The broker SHOULD serve clients on a fair basis and MAY deliver requests to workers on any basis, including round robin and least-recently used.

MDP consists of two sub-protocols:

* MDP/Client, which covers how the MDP broker communicates with client applications.
* MDP/Worker, which covers how the MDP broker communicates with workers applications.

The broker may be an intermediary (a device) or it may be a client application that implements MDP/Worker directly. Similarly, the broker may integrate services directly rather than using MDP/Worker.

### ROUTER Addressing

The broker MUST use a ROUTER socket to accept requests from clients, and connections from workers. The broker MAY use a separate socket for each sub-protocol, or MAY use a single socket for both sub-protocols.

From the ØMQ Reference Manual (see "[]()"):

> When receiving messages a ROUTER socket shall prepend a message part containing the identity of the originating peer to the message before passing it to the application. When sending messages a ROUTER socket shall remove the first part of the message and use it to determine the identity of the peer the message shall be routed to.

This extra frame is not shown in the sub-protocol commands explained below.

### MDP/Client

MDP/Client is a strictly synchronous dialog initiated by the client (where 'C' represents the client, and 'B' represents the broker):

```
Repeat:
    C: REQUEST
    B: *PARTIAL
    B: FINAL
    ...
```

A **REQUEST** command consists of a multipart message of 4 or more frames, formatted on the wire as follows:

* Frame 0: "MDPC02" (six bytes, representing MDP/Client v0.2)
* Frame 1: 0x01 (one byte, representing REQUEST)
* Frame 2: Service name (printable string)
* Frames 3+: Request body (opaque binary)

A **PARTIAL** command consists of a multipart message of 4 or more frames, formatted on the wire as follows:

* Frame 0: "MDPC02" (six bytes, representing MDP/Client v0.2)
* Frame 1: 0x02 (one byte, representing PARTIAL)
* Frame 2: Service name (printable string)
* Frames 3+: Reply body (opaque binary)

A **FINAL** command consists of a multipart message of 4 or more frames, formatted on the wire as follows:

* Frame 0: "MDPC02" (six bytes, representing MDP/Client v0.2)
* Frame 1: 0x03 (one byte, representing FINAL)
* Frame 2: Service name (printable string)
* Frames 3+: Reply body (opaque binary)

Clients MUST use a DEALER socket. The client MUST be prepared to handle zero or more PARTIAL commands from the broker. After a FINAL command, the broker SHALL not send any further commands to the client.

Clients MAY use any suitable strategy for recovering from a non-responsive broker. One recommended strategy is:

* To use polling instead of blocking receives on the request socket.
* If there is no reply within some timeout, to close the request socket and open a new socket, and resend the request on that new socket.
* If there is no reply after several retries, to signal the transaction as failed.

The service name is a 0MQ string that matches the service name specified by a worker in its READY command (see MDP/Worker below). The broker SHOULD queue client requests for which no service has been registered and SHOULD expire these requests after a reasonable and configurable time if no service has been registered.

### MDP/Worker

MDP/Worker is a mix of a synchronous request-reply dialog, initiated by the service worker, and an asynchronous heartbeat dialog that operates independently in both directions. This is the synchronous dialog (where 'S' represents the worker, and 'B' represents the broker):

```
W: READY
Repeat:
    B: REQUEST
    W: *PARTIAL
    W: FINAL
    ...
```

The asynchronous heartbeat dialog operates on the same sockets and works thus:

```
Repeat:                 Repeat:
    W: HEARTBEAT            B: HEARTBEAT
    ...                     ...
W: DISCONNECT           B: DISCONNECT
```

A **READY** command consists of a multipart message of 3 frames, formatted on the wire as follows:

* Frame 0: "MDPW02" (six bytes, representing MDP/Worker v0.2)
* Frame 1: 0x01 (one byte, representing READY)
* Frame 2: Service name (printable string)

A **REQUEST** command consists of a multipart message of 5 or more frames, formatted on the wire as follows:

* Frame 0: "MDPW02" (six bytes, representing MDP/Worker v0.2)
* Frame 1: 0x02 (one byte, representing REQUEST)
* Frame 2: Client address (envelope stack)
* Frame 3: Empty (zero bytes, envelope delimiter)
* Frames 4+: Request body (opaque binary)

A **PARTIAL** command consists of a multipart message of 5 or more frames, formatted on the wire as follows:

* Frame 0: "MDPW02" (six bytes, representing MDP/Worker v0.2)
* Frame 1: 0x03 (one byte, representing PARTIAL)
* Frame 2: Client address (envelope stack)
* Frame 3: Empty (zero bytes, envelope delimiter)
* Frames 4+: Reply body (opaque binary)

A **FINAL** command consists of a multipart message of 5 or more frames, formatted on the wire as follows:

* Frame 0: "MDPW02" (six bytes, representing MDP/Worker v0.2)
* Frame 1: 0x04 (one byte, representing FINAL)
* Frame 2: Client address (envelope stack)
* Frame 3: Empty (zero bytes, envelope delimiter)
* Frames 4+: Reply body (opaque binary)

A **HEARTBEAT** command consists of a multipart message of 2 frames, formatted on the wire as follows:

* Frame 0: "MDPW02" (six bytes, representing MDP/Worker v0.2)
* Frame 1: 0x05 (one byte, representing HEARTBEAT)

A **DISCONNECT** command consists of a multipart message of 2 frames, formatted on the wire as follows:

* Frame 0: "MDPW02" (six bytes, representing MDP/Worker v0.2)
* Frame 1: 0x06 (one byte, representing DISCONNECT)

#### Opening and Closing a Connection

* The worker is responsible for opening and closing a logical connection. One worker MUST connect to exactly one broker using a single ØMQ DEALER socket.

* Since ØMQ automatically reconnects peers after a failure, every MDP command includes the protocol header to allow proper validation of all messages that a peer receives.

* The worker opens the connection to the broker by creating a new socket, connecting it, and then sending a READY command to register a service. One worker handles precisely one service, and many workers MAY handle the same service. The worker MUST NOT send a further READY.

* There is no response to a READY. The worker SHOULD assume the registration succeeded until or unless it receives a DISCONNECT, or it detects a broker failure through heartbeating.

* The worker MAY send DISCONNECT at any time, including before READY. When the broker receives DISCONNECT from a worker it MUST send no further commands to that worker.

* The broker MAY send DISCONNECT at any time, by definition after it has received at least one command from the worker.

* The broker MUST respond to any valid but unexpected command by sending DISCONNECT and then no further commands to that worker. The broker SHOULD respond to invalid messages by dropping them and treating that peer as invalid.

* When the worker receives DISCONNECT it must send no further commands to the broker; it MUST close its socket, and reconnect to the broker on a new socket. This mechanism allows workers to re-register after a broker failure and recovery.

#### Request and Reply Processing

* The worker SHALL send zero or more PARTIAL commands for a single REQUEST, followed by exactly one FINAL command.

* The REQUEST, PARTIAL and FINAL commands SHALL contain precisely one client address frame. This frame MUST be followed by an empty (zero sized) frame.

* The address of each directly connected client is prepended by the ROUTER socket to all request messages coming from clients. That ROUTER socket also expects a client address to be prepended to each reply message sent to a client.

#### Heartbeating

* HEARTBEAT commands are valid at any time, after a READY command.

* Any received command except DISCONNECT acts as a heartbeat. Peers SHOULD NOT send HEARTBEAT commands while also sending other commands.

* Both broker and worker MUST send heartbeats at regular and agreed-upon intervals. A peer MUST consider the other peer "disconnected" if no heartbeat arrives within some multiple of that interval (usually 3-5).

* If the worker detects that the broker has disconnected, it SHOULD restart a new conversation.

* If the broker detects that the worked has disconnected, it SHOULD stop sending it messages of any type.

### Reliability

The Majordomo pattern is designed to extend the basic ØMQ request-reply pattern with the ability to detect and recover from a specific set of failures:

* Worker applications which crash, run too slowly, or freeze.
* Worker applications that are disconnected from the network (temporarily or permanently).
* Client applications that are temporarily disconnected from the network.
* A queue broker that crashes and is restarted.
* A queue broker that suffers a permanent failure.
* Requests or replies that are lost due to any of these failures.

The general approach is to retry and reconnect, using heartbeating when needed. Majordomo supports live-live broker failover indirectly, by keeping no significant state in a broker. Actual failover to alternate brokers is handled by the clients and workers, not the protocol. For a fuller discussion of these failures and the strategies that Majordomo uses to detect and recover, see Chapter 4 of the Guide (see "[ØMQ - The Guide](http://zguide.zeromq.org)").

### Scalability and Performance

Majordomo is designed to be scalable to large numbers (thousands) of workers and clients, limited only by system resources on the broker. Partitioning of workers by service allows for multiple applications to share the same broker infrastructure.

Throughput performance for a single client application will be limited to tens of thousands, not millions, of request-reply transactions per second due to round-trip costs and the extra latency of a broker-based approach. The larger the request and reply messages, the more efficient Majordomo will become. Majordomo may be complemented by high-speed data delivery architectures.

System requirements for the broker are moderate: no more than one outstanding request per client will be queued, and message contents can be switched between clients and workers without copying or processing. A single broker thread can therefore switch several million messages per second, and multithreaded implementations (offering multiple virtual brokers, each on its own port) can scale to as many cores as required.

### Security

MDP does not implement any authentication, access control, or encryption mechanisms and should not be used in any deployment where these are required. However, all these can be layered on top of MDP, and may be the subject of future RFCs.

### Reference Implementations

The reference implementation for MDP/0.2 is at https://github.com/zeromq/majordomo.

### Known Weaknesses

* The heartbeat rate must be set to similar values in broker and worker, or false disconnections will occur. A better heartbeat design will be developed later.
* The use of multiple frames for command formatting has a performance impact. Future versions of MDP may place commands into single frames.
