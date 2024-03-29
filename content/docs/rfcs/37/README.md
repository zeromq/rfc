---
slug: 37
title: 37/ZMTP
name: ZeroMQ Message Transport Protocol
status: draft
editor: Pieter Hintjens <ph@imatix.com>
---

The ZeroMQ Message Transport Protocol (ZMTP) is a transport layer protocol for exchanging messages between two peers over a connected transport layer such as TCP. This document describes ZMTP 3.1. This version adds JOIN, LEAVE, SUBSCRIBE, CANCEL, PING and PONG commands, and endpoint resources.

## Preamble

Copyright (c) 2009-2015 iMatix Corporation & Contributors

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

The ZeroMQ Message Transport Protocol (ZMTP) is a transport layer protocol for exchanging messages between two peers over a connected transport layer such as TCP. This document describes version 3.1 of ZMTP. ZMTP solves a number of problems we face when using TCP carry messages:

* TCP carries a stream of octets with no delimiters, but we want to send and receive discrete messages. Thus, ZMTP reads and writes *frames* consisting of a size and a body.

* We need to carry metadata on each frame (such as, whether the frame is part of a multipart message, or not). ZMTP provides a *flags* field in each frame for metadata.

* We need to be able to talk to older implementations, so that our framing can evolve without breaking existing implementations. ZMTP defines a *greeting* that announces the implemented version number, and specifies a method for version negotiation.

* We need security so that peers can be sure of the identity of the peers they talk to, and so that messages cannot be tampered with, nor inspected, by third parties. ZMTP defines a *security handshake* that allows peers to create a secure connection.

* We need a range of security protocols, from clear text (no security, but fast) to fully authenticated and encrypted (secure, but slow). Further, we need the freedom to add new security protocols over time. ZMTP defines a way for peers to agree on an extensible *security mechanism*.

* We need a way to carry metadata about the connection, after the security handshake. ZMTP defines a standard set of *metadata properties* (socket type, identity, etc.) that peers exchange after the security mechanism.

* We want to allow multiple tasks to share a single external unique interface and port, to decrease system administration costs.

* We need to write down these solutions in a way that is easy for teams to implement on any platform and in any language. ZMTP is thus specified as a formal protocol (this document) and made available to teams under a free license.

* We need guarantees that people will not create private forks of ZMTP, thus breaking interoperability. ZMTP is thus licensed under the GPLv3, so that any derived versions must also be made available to users of software that implements it.

### Changes from ZMTP 3.0

ZMTP defines a *resource* metadata property that lets any number of tasks share a single interface and port.

### Related Specifications

* http://rfc.zeromq.org/spec:23/ZMTP defines ZMTP v3.0.
* http://rfc.zeromq.org/spec:24/ZMTP-PLAIN defines the ZMTP-PLAIN security mechanism.
* http://rfc.zeromq.org/spec:25/ZMTP-CURVE defines the ZMTP-CURVE security mechanism.
* http://rfc.zeromq.org/spec:26/CURVEZMQ defines the CurveZMQ authentication and encryption protocol.
* http://rfc.zeromq.org/spec:27/ZAP defines the ZeroMQ Authentication Protocol.
* http://rfc.zeromq.org/spec:28/REQREP defines the semantics of REQ, REP, DEALER and ROUTER sockets.
* http://rfc.zeromq.org/spec:29/PUBSUB defines the semantics of PUB, XPUB, SUB and XSUB sockets.
* http://rfc.zeromq.org/spec:30/PIPELINE defines the semantics of PUSH and PULL sockets.
* http://rfc.zeromq.org/spec:31/EXPAIR defines the semantics of exclusive PAIR sockets.
* http://rfc.zeromq.org/spec:41/CLIENTSERVER defines the semantics of CLIENT and SERVER.
* http://rfc.zeromq.org/spec:48/RADIODISH defines the semantics of RADIO and DISH.
* http://rfc.zeromq.org/spec:49/SCATTERGATHER defines the semantics of SCATTER and GATHER.
* http://rfc.zeromq.org/spec:51/P2P defines the semantics of PEER.
* http://rfc.zeromq.org/spec:52/CHANNEL defines the semantics of CHANNEL.


## Implementation

### Overall Behavior

A ZMTP connection goes through these main stages:

* The two peers agree on the version and security mechanism of the connection by sending each other data and either continuing the discussion, or closing the connection.

* The two peers handshake the security mechanism by exchanging zero or more commands. If the security handshake is successful, the peers continue the discussion, otherwise one or both peers closes the connection.

* Each peer then sends the other metadata about the connection as a final command. The peers may check the metadata and each peer decides either to continue, or to close the connection.

* Each peer is then able to send the other messages. Either peer may at any moment close the connection.

### Formal Grammar

The following ABNF grammar defines the protocol:

```
;   The protocol consists of zero or more connections
zmtp = *connection

;   A connection is a greeting, a handshake, and traffic
connection = greeting handshake traffic

;   The greeting announces the protocol details
greeting = signature version mechanism as-server filler

signature = %xFF padding %x7F
padding = 8OCTET        ; Not significant

version = version-major version-minor
version-major = %x03
version-minor = %x01

;   The mechanism is a null padded string
mechanism = 20mechanism-char
mechanism-char = "A"-"Z" | DIGIT
    | "-" | "_" | "." | "+" | %x0

;   Is the peer acting as server for security handshake?
as-server = %x00 | %x01

;   The filler extends the greeting to 64 octets
filler = 31%x00             ; 31 zero octets

;   The handshake consists of at least one command
;   The actual grammar depends on the security mechanism
handshake = 1*command

;   Traffic consists of commands and messages intermixed
traffic = *(command | message)

;   A command is a single long or short frame
command = command-size command-body
command-size = %x04 short-size | %x06 long-size
short-size = OCTET          ; Body is 0 to 255 octets
long-size = 8OCTET          ; Body is 0 to 2^63-1 octets
command-body = command-name command-data
command-name = short-size 1*255command-name-char
command-name-char = ALPHA
command-data = *OCTET

;   A message is one or more frames
message = *message-more message-last
message-more = ( %x01 short-size | %x03 long-size ) message-body
message-last = ( %x00 short-size | %x02 long-size ) message-body
message-body = *OCTET
```

### Version Negotiation

ZMTP provides asymmetric version negotiation. A ZMTP peer MAY attempt to detect and work with older versions of the protocol. It MAY also demand ZMTP capabilities from its peers.

In the first case, after making or receiving a connection, the peer SHALL send to the other a partial greeting sufficient to trigger version detection. This is the first 11 octets of the greeting (signature and major version number). The peer SHALL then read the first eleven octets of greeting sent by the other peer, and determine whether to downgrade or not. The specific heuristics for each older ZMTP version are explained in the section "Backwards Interoperability". In this case the peer MAY use the padding field for older protocol detection (we explain the specific known case below).

In the second case, after making or receiving a connection, the peer SHALL send its entire greeting (64 octets) and SHALL expect a matching 64 octet greeting. In this case the peer SHOULD set the padding field to binary zeros.

In either case, note that:

* A peer SHALL NOT assign any significance to the padding field and MUST NOT validate this nor interpret it in any way whatsoever.

* A peer MUST accept higher protocol versions as valid. That is, a ZMTP peer MUST accept protocol versions greater or equal to 3.1. This allows future implementations to safely interoperate with current implementations.

* A peer SHALL always use its own protocol (including framing) when talking to an equal or higher protocol peer.

* A peer MAY downgrade its protocol to talk to a lower protocol peer.

* If a peer cannot downgrade its protocol to match its peer, it MUST close the connection.

### Topology

ZMTP is by default a peer-to-peer protocol that makes no distinction between the clients and servers.

However, the security mechanisms (which are extension protocols, and explained below) MAY define distinct roles for client peers and server peers. This difference reflects the general model of concentrating authentication on servers.

Traditionally in TCP topologies, the "server" is the peer which binds, and the "client" is the peer that connects. ZMTP allows this but also the opposite topology in which the client binds, and the server connects.

If the chosen security mechanism does not specify client and server roles, the as-server field SHALL have no significance, and SHOULD be zero for all peers.

### Authentication and Confidentiality

ZMTP provides extensible authentication and confidentiality through the use of a negotiated security mechanism that is loosely based on the IETF [Simple Authentication and Security Layer (SASL)](http://tools.ietf.org/html/rfc4422). The peer MAY support any or all of the following mechanisms:

* "NULL", specified later in this document, which implements no authentication, and no confidentiality.

* "PLAIN", specified by [rfc.zeromq.org/spec:24/ZMTP-PLAIN](http://rfc.zeromq.org/spec:24/), which implements simple user-name and password authentication, in clear text.

* "CURVE", specified by [rfc.zeromq.org/spec:25/ZMTP-CURVE](http://rfc.zeromq.org/spec:25/), which implements full authentication and confidentiality using the [CurveZMQ security protocol](http://curvezmq.org).

The security mechanism is an ASCII string, null-padded as needed to fit 20 octets. Implementations MAY define their own mechanisms for experimentation and internal use. All mechanisms intended for public interoperability SHALL be defined as 0MQ RFCs. Mechanism names SHALL be assigned on a first-come first-served basis. Mechanism names SHALL consist only of uppercase letters A to Z, digits, and embedded hyphens or underscores.

A peer announces precisely one security mechanism, unlike SASL, which lets servers announce multiple security mechanisms. Security in ZMTP is *assertive* in that all peers on a given socket have the same, required level of security. This prevents downgrade attacks and simplifies implementations.

Each security mechanism defines a protocol consisting of zero or more commands sent by either peer to the other until the handshaking is complete or either of the peers refuses to continue, and closes the connection.

Commands are single frames consisting of a flags octet, a size field, and a body. If the body is 0-255 octets, the command SHOULD use a short size field (%x04 followed by 1 octet). If the body is 256 or more octets, the command SHOULD use a long size field (%x06 followed by 8 octets).

The flags octet and the size field is always in clear text. The body may be partially or fully encrypted. ZMTP does not define the syntax nor semantics of commands. These are fully defined by the security mechanism protocol.

The supported mechanism is not considered sensitive information. A peer that reads a full greeting, including mechanism, MUST also send a full greeting including mechanism. This avoids deadlocks in which two peers each wait for the other to divulge the remainder of their greeting.

If the mechanism that the peer received does not exactly match the mechanism it sent, it MUST close the connection.

### Error Handling

ZMTP allows an explicit fatal error response during the mechanism handshake, using the ERROR command. The peer SHALL treat an incoming ERROR command as fatal, and act by closing the connection, and not re-connecting using the same security credentials.

An implementation SHOULD signal any other error, e.g. overloaded, temporarily refusing connections, etc. by closing the connection. The peer SHALL treat an unexpected connection close as a temporary error, and SHOULD reconnect.

To avoid connection storms, peers should reconnect after a short and possibly randomized interval. Further, if a peer reconnects more than once, it should increase the delay between reconnects. Various strategies are possible.

### Framing

Following the greeting, which has a fixed size of 64 octets, all further data is sent as *frames*, which carry commands or messages. The frame design is meant to be efficient for small frames but capable of handling extremely large data as well.

A frame consists of a flags field (1 octet), followed by a size field (one octet or eight octets) and a frame body of size octets. The size does not include the flags field, nor itself, so an empty frame has a size of zero.

A short frame has a body of 0 to 255 octets. A long frame has a body of 0 to 2^63-1 octets.

The flags field consists of a single octet containing various control flags. Bit 0 is the least significant bit (rightmost bit):

* Bits 7-3: *Reserved*. Bits 7-3 are reserved for future use and MUST be zero.

* Bit 2 (COMMAND): *Command frame*. A value of 1 indicates that the frame is a command frame. A value of 0 indicates that the frame is a message frame.

* Bit 1 (LONG): *Long frame*. A value of 0 indicates that the frame size is encoded as a single octet. A value of 1 indicates that the frame size is encoded as a 64-bit unsigned integer in network byte order.

* Bit 0 (MORE): *More frames to follow*. A value of 0 indicates that there are no more frames to follow. A value of 1 indicates that more frames will follow. This bit SHALL be zero on command frames.

#### Commands

Commands are used by the ZMTP implementation and not generally visible to the application except in some cases. Commands always consist of one frame, containing a printable command name, a null octet separator, and data.

These are the commands that this specification defines:

* READY and ERROR - implements the NULL security handshake, see "The NULL Security Mechanism" below.
* SUBSCRIBE and CANCEL - subscription management, see "The Publish-Subscribe Pattern", below.
* PING and PONG - heartbeat requests and replies, see "Connection Heartbeating" below.

ZMTP supports extensible security mechanisms and these may define their own commands. Security mechanisms MAY use any command names they need to.

#### Messages

Messages carry application data and are not generally created, modified, or filtered by the ZMTP implementation except in some cases. Messages consist of one or more frames and an implementation SHALL always send and deliver messages atomically, that is, all the frames of a message, or none of them.

### The NULL Security Mechanism

The NULL mechanism implements no authentication and no confidentiality. The NULL mechanism SHOULD NOT be used on public infrastructure without transport-level security (e.g. over a VPN).

When a peer uses the NULL security mechanism, the as-server field MUST be zero. The peer that binds SHALL be the server, and connecting peer SHALL be the client.

To complete a NULL security handshake, the client SHALL send a READY command and then wait for a READY command in reply. The server SHOULD parse, and MAY validate the READY command. If there is no error, it MUST send a READY command in reply. Either or both peer MAY choose to close the connection if validation failed. A peer MAY start to send messages immediately after completing its handshake, that is, having both sent and received a READY command.

The following ABNF grammar defines the NULL security handshake:

```
null = ready *message | error
ready = command-size %d5 "READY" metadata
metadata = *property
property = name value
name = short-size 1*255name-char
name-char = ALPHA | DIGIT | "-" | "_" | "." | "+"
value = 4OCTET *OCTET       ; Size in network byte order
error = command-size %d5 "ERROR" error-reason
error-reason = short-size 0*255VCHAR
```

The message and command-size are defined by the ZMTP grammar previously explained.

The body of the READY command consists of a list of properties consisting of name and value as size-specified strings.

The name SHALL be 1 to 255 characters. Zero-sized names are not valid. The case (upper or lower) of names SHALL NOT be significant.

The value SHALL be 0 to 2,147,483,647 (2^31-1 or INT32_MAX in C/C++) octets of opaque binary data. Zero-sized values are allowed. The semantics of the value depend on the property. The value size field SHALL be four octets, in network byte order. Note that this size field will mostly not be aligned in memory.

The body of the ERROR command contains an error reason that can be logged. It has no defined semantic value.

### Connection Metadata

The security mechanism SHALL provide a way for peers to exchange metadata in the form of a key-value dictionary. The precise encoding of the metadata depends on the mechanism.

Metadata names SHALL be case-insensitive.

These metadata properties are defined:

* "Socket-Type", which specifies the sender's socket type. See the section "The Socket Type Property" below. The sender SHOULD specify the Socket-Type.

* "Identity", which specifies the sender's socket identity. See the section "The Identity Property" below. The sender MAY specify an Identity.

* "Resource", which specifies the a resource to connect to. See the section "The Resource Property" below. The sender MAY specify a Resource.

The implementation MAY provide other metadata properties such as implementation name, platform name, and so on. For interoperability, metadata names and semantics MAY be defined as RFCs.

Metadata names starting with "X-" SHALL be reserved for application use.

#### The Socket-Type Property

The Socket-Type announces the ZeroMQ socket type of the sending peer. The Socket-Type SHALL match this grammar:

```
socket-type = "REQ" | "REP"
            | "DEALER" | "ROUTER"
            | "PUB" | "XPUB"
            | "SUB" | "XSUB"
            | "PUSH" | "PULL"
            | "PAIR"
            | "CLIENT" | "SERVER"
            | "RADIO" | "DISH"
            | "SCATTER" | "GATHER"
            | "PEER" 
            | "CHANNEL"
```

The peer SHOULD enforce that the other peer is using a valid socket type. For each socket type, its legal peer types are as follows:

* **REQ**: REP, ROUTER<br>
  **REP**: REQ, DEALER<br>
  **DEALER**: REP, DEALER, ROUTER<br>
  **ROUTER**: REQ, DEALER, ROUTER

* **PUB**: SUB, XSUB<br>
  **XPUB**: SUB, XSUB<br>
  **SUB**: PUB, XPUB<br>
  **XSUB**: PUB, XPUB

* **PUSH**: PULL<br>
  **PULL**: PUSH

* **PAIR**: PAIR

* **CLIENT**: SERVER<br>
  **SERVER**: CLIENT

* **RADIO**: DISH<br>
  **DISH**: RADIO

* **SCATTER**: GATHER<br>
  **GATHER**: SCATTER

* **PEER**: PEER

* **CHANNEL**: CHANNEL

When a peer validates the socket type, it SHOULD handle errors by returning an ERROR command, and then disconnecting the peer.

#### The Identity Property

A REQ, DEALER, or ROUTER peer connecting to a ROUTER MAY announce its identity, which is used as an addressing mechanism by the ROUTER socket. For all other socket types, the Identity property shall be ignored.

The Identity SHALL match this grammar:

```
identity = 0*255OCTET
```

The first octet of the Identity SHALL NOT be zero: identities starting with a zero octet are reserved for implementations' internal use.

#### **NEW:** The Resource Property

The Resource Property addresses the common problem of running multiple services (within a single process) on a single network endpoint. This is particularly desirable when using public-facing ports, which can be expensive to manage due to firewall issues. Without any protocol support, one service requires one port number to bind to. With protocol support, multiple services can share a single port.

ZMTP implements this sharing using the concept of "Resource". Resources are chosen by end-user applications and conceptually extend the zmq_bind or zmq_connect API calls. For example:

```
zmq_bind (server_socket, "tcp://eth0:6000/system/name-service/test");
```

And:

```
zmq_connect (client_socket, "tcp://192.168.55.212:6000/system/name-service/test");
```

Where the resource is then "system/name-service/test".

The Resource SHALL match this grammar:

```
resource = resource-name * ( "/" resource-name )
resource-name = 1*resource-char
resource-char = ALPHA | DIGIT | "$" | "-" | "_" | "@" | "." | "&" | "+"
```

The implementation SHALL accept all resource requests, as a resource may become available at an arbitrary time after the connection has been established.

Since resource evaluation happens *after* security handshaking, all services that use the same endpoint SHALL use the same security credentials. One strategy for achieving this is:

* Create a listener for the first bind to a particular interface:port.
* Store security credentials on a per-listener basis.
* Handle new incoming connections on a per-listener basis.
* Resolve the resource name after security handshaking, and hand-off connections to the appropriate service.

## Socket Semantics

In order to ensure interoperability, we aim to define both the socket-specific semantics of the API to calling applications, and the behavior of peers talking to each other over the network.

These rules apply to all sockets:

* All sockets SHALL accept connections (binding to an address) and make connections.

* All sockets SHALL establish connections opportunistically, that is: they connect to an endpoint asynchronously, and if the connection is broken, SHOULD reconnect after a suitable delay.

* A message SHALL be sent or received atomically; that is, all frames or none. On sending, the peer SHALL queue all frames of a message in memory until the final frame is sent.

* A message SHALL NOT be delivered more than once to any peer.

* All messages between two immediate peers SHALL be delivered in order.

### The Request-Reply Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:28/REQREP for the semantics of REQ, REP, DEALER and ROUTER sockets.

### The Publish-Subscribe Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:29/PUBSUB for the semantics of PUB, XPUB, SUB and XSUB sockets.

When using ZMTP, message filtering SHALL happen at the publisher side (the PUB or XPUB socket). To create a subscription, the SUB or XSUB peer SHALL send a SUBSCRIBE command, which has this grammar:

```
subscribe = command-size %d9 "SUBSCRIBE" subscription
subscription = *OCTET
```

To cancel a subscription, the SUB or XSUB peer SHALL send a CANCEL command, which has this grammar:

```
cancel = command-size %d6 "CANCEL" subscription
```

The subscription is a binary string that specifies what messages the subscriber wants. A subscription of "A" SHALL match all messages starting with "A". An empty subscription SHALL match all messages.

Subscriptions SHALL be additive and SHALL NOT be idempotent. That is, subscribing to "A" and "" is the same as subscribing to "" alone. Subscribing to "A" and "A" counts as two subscriptions, and would require two CANCEL commands to undo.

### The Pipeline Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:30/PIPELINE for the semantics of PUSH and PULL sockets.

### The Exclusive Pair Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:31/EXPAIR for the semantics of exclusive PAIR sockets.

### The Client-Server Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:47/CLIENTSERVER for the semantics of CLIENT and SERVER sockets.

### The Radio-Dish Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:48/RADIODISH for the semantics of RADIO and DISH sockets.

When using ZMTP, group membership checks SHALL happen at the RADIO side. To join a group, the DISH peer SHALL send a JOIN command, which has this grammar:

```
join = command-size %d4 "JOIN" group
group = 0*255group-char 
group-char = %d1-255
```

To leave a group, the DISH peer SHALL send a LEAVE command, which has this grammar:

```
leave = command-size %d5 "LEAVE" group
```

The group is a string, a message is sent to a group and all members of the group will receieve the message. Group matching is exact.

When using ZMTP, RADIO send the group and the message are over the wire as a two parts message, which has this grammar:

```
radio-dish-message = group-part body-part
group-part = %x01 short-size group
body-part = message-last
group = 0*255group-char 
group-char = %d1-255
```

### The Scatter-Gather Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:49/SCATTERGATHER for the semantics of SCATTER and GATHER sockets.

### The Peer-to-Peer Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:51/P2P for the semantics of PEER socket.

### The CHANNEL Pattern

The implementation SHOULD follow http://rfc.zeromq.org/spec:52/CHANNEL for the semantics of CHANNEL socket.

### **NEW:** Thread-Safe Socket Types

Thread-safe is a new family of socket types which are safe to use from multiple threads, both for sending and receiving.

For thread-safety the sockets API MUST be atomic, a call to send MUST send the entire message and a call to receive MUST receive the entire message. Therefore thread-safe sockets disallow multipart messages. 

Thread-safe socket MUST disallow the sending of multipart messages and MUST discard any multipart messages received from the wire. 

Any additional metadata (e.g routing-id or group) must be attached to the message.

Socket types that are part of the thread-safe family:
* CLIENT
* SERVER
* RADIO
* DISH
* SCATTER
* GATHER
* PEER
* CHANNEL

#### Multipart Message Definition

A multipart message is multiple sequential ZMTP messages, where all but the last message has the MORE flag set.

## Connection Heartbeating

ZMTP/3.1 provides connection heartbeating to address some specific problems:

* Network connections can go stale and die without reporting TCP errors. By detecting the lack of incoming traffic, a peer can deduce that the connection has gone stale.

* Processes can become blocked, especially if they run out of memory. TCP will not report an error but as for a stale connection, the lack of traffic can be used to deduce a fatal error.

To invoke a single heartbeat, the peer MAY, at any point after the security handshake is complete, send a PING command:

```
ping = command-size %d4 "PING" ping-ttl ping-context
ping-ttl = 2OCTET
ping-context = 0*16OCTET
```

The ping-ttl is a 16-bit unsigned integer in network order that MAY be zero or MAY contain a time-to-live measured in tenths of seconds. The ping-ttl provides a strong hint to the other peer to disconnect if no further traffic is received after that time. As a consequence, the maximum TTL is 6553.5 seconds.

When a peer receives a PING command it SHALL respond with a PONG command that echoes the ping-context, which may be empty and MUST not exceed 16 octets:

```
pong = command-size %d4 "PONG" ping-context
ping-context = 0*16OCTET
```

When a peer does not receive a reply after a reasonable interval, it MAY consider the connection dead, and close it. The interval SHOULD be selected to suit the relevant application use case. This time-out interval will usually be a small multiple of the PING interval.

Since PONG replies may be arbitrarily delayed behind already queued traffic, a peer SHOULD treat any incoming traffic (not just a PONG reply) as a sign of life.

To avoid PONG storms, a peer should not send more than a small number of PING commands without receiving PONG replies.

A peer SHOULD consider the connection dead in these cases:

* If it sends a PING command and does not receive any traffic within some timeout.
* If it receives a PING command with a non-zero TTL and then does not receive any further traffic within that TTL.

After a failed heartbeat, the peer SHOULD reconnect as normal - there is no specific strategy for recovery after such an event.

## Backwards Interoperability

To detect and work with older versions of ZMTP, we define two strategies; one will detect only 2.0 peers, and the second will detect both 1.0 and 2.0 peers.

When a peer does not need backwards interoperability, it SHOULD send its full greeting immediately upon establishing a new peer connection.

### Detecting ZMTP 2.0 Peers

From [[ZMTP 2.0](http://rfc.zeromq.org/spec:13) onwards, the protocol contains the version number immediately after the signature. To detect and interoperate with ZMTP 2.0 peers, the implementation MAY use this strategy:

* Send the 10-octet signature followed by the major version number (the single octet %x03).

* Wait for the other peer to send its greeting.

* If the peer version number is 1 or 2, the peer is using ZMTP 2.0, so send the ZMTP 2.0 socket type and identity and continue with ZMTP 2.0.

* If the peer version number is 3 or higher, the peer is using ZMTP 3.x, so send the rest of the greeting and continue with ZMTP 3.1 or 3.0.

Here is the sequence of commands showing two ZMTP 3.1 peers using this algorithm:

```
C:signature + major-version             S:signature + major-version
C:rest of greeting                      S:rest of greeting
C:ready                                 S:ready
C:message...                            S:message...
```

### Detecting ZMTP 1.0 and 2.0 Peers

[ZMTP 1.0](http://rfc.zeromq.org/spec:15) did not have any version information. To detect and interoperate with a ZMTP 1.0 and 2.0 peers, an implementation MAY use this strategy:

* Send a 10-octet pseudo-signature consisting of "%xFF size %x7F" where 'size' is the number of octets in the sender's identity (0 or greater) plus 1. The size SHALL be 8 octets in network byte order and occupies the padding field.

* Read the first octet from the other peer.

* If this first octet is not %FF, then the other peer is using ZMTP 1.0, and has sent us a short frame length for its identity. We read that many octets.

* If this first octet is %FF, then we read nine further octets, and inspect the last octet (the 10th in total). If the least significant bit is 0, then the other peer is using ZMTP 1.0 and has sent us a long length for its identity. We read that many octets.

* If the least significant bit is 1, the peer is using ZMTP 2.0 or later and has sent us the ZMTP signature. We read a further octet, which indicates the ZMTP version. If this is 1 or 2, we have a ZMTP 2.0 peer. If this is 3, we have a ZMTP 3.x peer.

* If the implementation's security mechanism is anything other than NULL, and it detects a ZMTP 1.0 or 2.0 peer, it MUST immediately close the connection. A ZMTP 1.0 or 2.0 peer can only request NULL security.

* When we have detected a ZMTP 1.0 peer, we have already sent 10 octets, which the other peer interprets as the start of an identity frame. We continue by sending the body of the identity frame (zero or more octets). From then, we encode and decode all frames on that connection using the ZMTP 1.0 framing syntax.

* When we have detected a ZMTP 2.0 peer, we continue with version negotiation as explained above, by sending our version number and then socket type and identity as defined by the ZMTP 2.0 specification.

* When we have detected a ZMTP 3.x peer, we continue by sending the remainder of the greeting (octets 10-64) and then continue with the security handshake as normal.

## Worked Example

A DEALER client connects to a ROUTER server. Both client and server are running ZMTP and the implementation has backwards compatibility detection. The peers will use the NULL security mechanism to talk to each other.

* The client sends a partial greeting (11 octets) greeting to the server, and at the same time (before receiving anything from the client), the server also sends a partial greeting:

```
 signature                   major
+------+-------------+------+------+
| %xFF | %x00...%x00 | %x7F | %x03 |
+------+-------------+------+------+
   0        1 - 8       9      10
```

* The client and server read the major version number (%x03) and send the rest of their greeting to each other:

```
 minor   mechanism  as-server  filler
+------+-----------+---------+-------------+
| %x01 |  "NULL"   |  %x00   | %x00...%x00 |
+------+-----------+---------+-------------+
   11     12 - 31      32        33 - 63
```

* The client and server now perform the NULL security handshake. First the client sends a READY command to the server that specifies the "DEALER" Socket-Type and empty Identity properties:

```
+------+----+
| %x04 | 41 |
+------+----+
   0     1
 flags  size

+------+---+---+---+---+---+
| %x05 | R | E | A | D | Y |
+------+---+---+---+---+---+
   2     3   4   5   6   7
 Command name "READY"

+----+---+---+---+---+---+---+---+---+---+---+---+
| 11 | S | o | c | k | e | t | - | T | y | p | e |
+----+---+---+---+---+---+---+---+---+---+---+---+
  8    9  10  11  12  13  14  15  16  17  18  19
 Property name "Socket-Type"

+------+------+------+------+---+---+---+---+---+---+
| %x00 | %x00 | %x00 | %x06 | D | E | A | L | E | R |
+------+------+------+------+---+---+---+---+---+---+
   20     21     22     23    24  25  26  27  28  29
 Property value "DEALER"

+----+---+---+---+---+---+---+---+---+
| 8  | I | d | e | n | t | i | t | y |
+----+---+---+---+---+---+---+---+---+
  30  31  32  33  34  35  36  37  38
 Property name "Identity"

+------+------+------+------+
| %x00 | %x00 | %x00 | %x00 |
+------+------+------+------+
   39     40     41     42
 Property value ""
```

* The server validates the socket type, accepts it, and replies with a READY command that contains only the Socket-Type property (ROUTER sockets do not send an identity):

```
+------+----+
| %x04 | 28 |
+------+----+
+------+---+---+---+---+---+
| %x05 | R | E | A | D | Y |
+------+---+---+---+---+---+
+----+---+---+---+---+---+---+---+---+---+---+---+
| 11 | S | o | c | k | e | t | - | T | y | p | e |
+----+---+---+---+---+---+---+---+---+---+---+---+
+------+------+------+------+---+---+---+---+---+---+
| %x00 | %x00 | %x00 | %x06 | R | O | U | T | E | R |
+------+------+------+------+---+---+---+---+---+---+
```

As soon as the server has sent its own READY command, it may also send messages to the client. As soon as the client has received the READY command from the server, it may send messages to the server.

## Wire Analysis of the Connection

A ZMTP connection is either clear text, or encrypted, according to the security mechanism. On a clear text connection, message data SHALL be sent as message frames. All clear text mechanisms SHALL use the message framing defined in this specification, so that wire analysis does not require knowledge of the specific mechanism.

On an encrypted connection, message data SHALL be encoded as commands so that wire analysis is not possible.

On all ZMTP connections, command names SHALL be visible and command frames SHALL be printable.

## Security Considerations

* An implementation MUST guard against downgrade attacks, which may take the form of crafting a ZMTP 1.0 or 2.0 protocol header, or asking for a less secure mechanism than the application has requested.

* Attackers may overwhelm a peer by repeated connection attempts. Thus, a peer MAY log failed accesses, and MAY detect and block repeated failed connections from specific originating IP addresses.

* Attackers may try to cause memory exhaustion in a peer by holding open connections. Thus, a peer MAY allocate memory to a connection only after the security handshake is complete, and MAY limit the number and cost of in-progress handshakes.

* Attackers may try to make small requests that generate large responses back to faked originating addresses (the real target of the attack). This is called an "amplification attack". Thus, peers MAY limit the number of in-progress handshakes per originating IP address.

* Attackers may try to uncover vulnerable versions of an implementation from its metadata. Thus, ZMTP sends metadata *after* security handshaking.

* Attackers can use the size of messages (even without breaking encryption) to collect information about the meaning. Thus, on an encrypted connection, message data SHOULD be padded to a randomized minimum size.

* Attackers can use the simple presence or absence of traffic to collect information about the peer ("Person X has come on line"). Thus, on an encrypted connection, the peer SHOULD send random garbage data ("noise") when there is no other traffic. To fully mask traffic, a peer MAY throttle messages so there is no visible difference between noise and real data.

## Topics for Discussion

This draft is open for discussion. The following topics at least are still on the table:

* If we move the socket type back into the greeting, we can formalize the grammar to include things like subscription messages for XSUB sockets. This is rather hard when socket types are stored as metadata.

* Do we need strategies to protect REQ sockets against blocking when a peer dies?

* Do we need strategies to protect REP sockets against malformed requests?

* Do we want DEALER and PUSH sockets to requeue undeliverable messages to other peers? This can improve reliability but results in out-of-order messages, and is not robust against messages lost in transit, or already delivered to the other peer.

* Credits, as a signaling mechanism for asynchronous flow control. This allows fine control over the amount of queued data. In this scenario, the receiving peer (DEALER, PULL, REP) would send credit which would be used up by messages routed to it. The minimal use-case is for PUSH/DEALER-to-PULL/DEALER round-robin routing. Credit could be octets, or messages.

* Commands or other strategies to allow wire analysis on unencrypted networks (without some additional information it is not possible to determine when messages start).
