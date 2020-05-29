---
slug: 46
title: 46/DAFKA
name: ZeroMQ Dafka Protocol 1.0
status: draft
editor: Kevin Sapper <mail@kevinsapper.de>
---

Dafka is a decentralized distributed streaming platform. This document describes Dafka 1.0.

## Preamble

Copyright (c) 2020 Kevin Sapper

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

Dafka is a decentralize distributed streaming platform. What exactly does that
mean?

A streaming platform has three key capabilities:

* Publish and subscribe to streams of records, similar to a message queue or
  enterprise messaging system.
* Store streams of records in a fault-tolerant durable way.
* Process streams of records as they occur.

Dafka is generally used for two broad classes of applications:

* Building real-time streaming data pipelines that reliably get data between
  systems or applications
* Building real-time streaming applications that transform or react to the
  streams of data

## Implementation

### Node types

Dafka defines three types of nodes:

* Producers
* Consumers
* Stores

### Node Identification and Life-cycle

A Dafka node represents either a producer, consumer or store. A Dafka node is
identified by a 16-octet universally unique identifier (UUID). Dafka does not
define how a node is created or destroyed but does assume that nodes have
a certain durability.

### Node Discovery and Presence

TBD

### Topics, Partitions and Records

Dafka version 1 defines the following regulations for topics, partitions and
records.

A Dafka cluster SHALL not place any restriction on the total number of topics
other than physicals due to the amount of available memory or disk space. The
name of a Dafka topic can be any byte sequence of arbitrary length. Though it is
RECOMMENDED to use only characters from the ASCII character set.

Each Dafka topic SHALL consist of at least one partition. In order to keep the
protocol simple each producer MUST publish records to exactly one partition and
further only one producer SHALL ever publish to the same partition. Which means
one partition can be mapped to exactly one producer. In order to make this
connection obvious the name of the partition SHALL be the producer's node UUID.

The first record of each partition SHALL have the offset 0. Each following
record's offset SHALL be the previous record's offset incremented by 1.

Dafka does not make any restrictions on the size of a record other than the
amount of memory available in the nodes.

### Interconnection Model

Dafka establishes a mesh network between all its nodes using a publish and
subscribe pattern.

Each node SHALL create a ZeroMQ PUB and a ZeroMQ SUB socket to communicate with
the towers. The PUB socket SHALL be connected to at least one tower's SUB socket
and the SUB socket SHALL be connected to at least one tower's PUB socket.

Each node SHALL create a ZeroMQ XPUB socket and bind it to an address that can
be reached by the other nodes. The node SHALL send this address at a regular
interval to all connected towers. The XPUB socket is used to send messages to
other nodes and get notified about subscriptions.

Each node SHALL create a second ZeroMQ SUB socket. When a node discovers another
node, it SHALL connect this socket, to the other nodes XPUB socket. This SUB
socket is used to receive messages from other nodes.

A node MAY disconnect its SUB socket if the peer has failed to respond within
some time (see Heartbeating).

### Protocol Signature

Every Dafka message sent SHALL start with the ZRE protocol signature, %xAA %xA0.
A node SHALL silently discard any message received that does not start with
these two octets.

This mechanism is designed particularly for applications that bind to ephemeral
ports which may have been previously used by other protocols, and to which there
are still nodes attempting to connect. It is also a general fail-fast mechanism
to detect ill-formed messages.

### Versioning

A version number octet %x01 shall follow the signature. A node SHALL discard
messages that do not contain a valid version number. There is no mechanism for
backwards interoperability.

### Protocol Grammar

The following ABNF grammar defines the dafka_proto:

    DAFKA           = join-consumer / publish / offsets

    join-consumer   = [S:STORE-HELLO C:CONSUMER-HELLO] / [ C:GET_HEADS ] *( S:DIRECT-HEAD [ consumer-fetch ] )

    consumer-fetch  = C:FETCH 1*( P:DIRECT-RECORD / S:DIRECT-RECORD )

    publish         = P:RECORD [ consumer-fetch / store-fetch S:ACK ]

    store-fetch     = S:FETCH 1*( ( P:DIRECT-RECORD / S:DIRECT-RECORD ) [ S:ACK ] )

    offsets         = P:HEAD [ consumer-fetch / store-fetch ]

    ;  Record from producer to consumers and stores. Topic is the name of the
    ;  topic. Subject is the name of the topic. Address is the address of the
    ;  producer (partition).

    RECORD          = signature %d'M' version address subject sequence content
    signature       = %xAA %xA5             ; two octets
    version         = number-1              ; Version = 1
    address         = string                ;
    subject         = string                ;
    sequence        = number-8              ;
    content         = frame                 ;

    ;  Direct record from a producer or a store to a consumer. Topic is the
    ;  address of the requestor. Subject is the name of the topic. Address is
    ;  the address of the producer (partition).

    DIRECT-RECORD   = signature %d'D' version address subject sequence content
    version         = number-1              ; Version = 1
    address         = string                ;
    subject         = string                ;
    sequence        = number-8              ;
    content         = frame                 ;

    ;  Consumer or store publish this message when a record is missing.
    ;  Either producer or a store can answer. Topic is the address of the
    ;  producer (partition). Subject is the name of the topic. Address is the
    ;  address of this message's sender. Count is the number of messages to
    ;  fetch starting with the record identified by sequence.

    FETCH           = signature %d'F' version address subject sequence count
    version         = number-1              ; Version = 1
    address         = string                ;
    subject         = string                ;
    sequence        = number-8              ;
    count           = number-4              ;

    ;  Ack from a stores to a producer. Topic is the address of the producer
    ;  (partition). Subject is the name of the topic.

    ACK             = signature %d'K' version address subject sequence
    version         = number-1              ; Version = 1
    address         = string                ;
    subject         = string                ;
    sequence        = number-8              ;

    ;  Head from producer to consumers and stores. Topic is the name of the
    ;  topic. Subject is the name of the topic. Address is the address of the
    ;  producer (partition).

    HEAD            = signature %d'H' version address subject sequence
    version         = number-1              ; Version = 1
    address         = string                ;
    subject         = string                ;
    sequence        = number-8              ;

    ;  Head from producer or store to a consumers. Topic is the name of the
    ;  topic. Subject is the name of the topic. Address is the address of the
    ;  producer (partition).

    DIRECT-HEAD     = signature %d'E' version address subject sequence
    version         = number-1              ; Version = 1
    address         = string                ;
    subject         = string                ;
    sequence        = number-8              ;

    ;  Get partition heads from stores send by a consumer. Topic is the name
    ;  of the topic. Address is the address of the consumer.

    GET-HEADS       = signature %d'G' version address
    version         = number-1              ; Version = 1
    address         = string                ;

    ;  Hello message from a consumer to a store. Topic is the store's
    ;  address. Address is the address of the consumer. Subjects is a list of
    ;  all topic the consumer is subscribed to.

    CONSUMER-HELLO  = signature %d'W' version address subjects
    version         = number-1              ; Version = 1
    address         = string                ;
    subjects        = strings               ;

    ;  Hello message from a store to a consumer. Topic is the consumer's
    ;  address. Address is the address of the store.

    STORE-HELLO     = signature %d'L' version address
    version         = number-1              ; Version = 1
    address         = string                ;

    ; A list of string values
    strings         = strings-count *strings-value
    strings-count   = number-4
    strings-value   = longstr

    ; A frame is zero or more octets encoded as a ZeroMQ frame
    frame           = *OCTET

    ; Strings are always length + text contents
    string          = number-1 *VCHAR
    longstr         = number-4 *VCHAR

    ; Numbers are unsigned integers in network byte order
    number-1        = 1OCTET
    number-4        = 4OCTET
    number-8        = 8OCTET

### Dafka Commands

All commands start with a protocol signature (%xAA %xA5), then a command
identifier and then the protocol version number (%x30).

Each command MUST contain as first frame the name of the ZeroMQ topic it is
published to. This frame is call topic frame.

#### The STORE-HELLO Command

When a store receives a store hello subscription on its XPUB socket it SHALL
send a STORE-HELLO command to the subscriber. The STORE-HELLO command has one
field: the address of the store.

The XPUB's topic frame is set to the STORE-HELLO command's ID concatenated with
the address of the consumer received by the subscription message.

#### The CONSUMER-HELLO Command

When a consumer receives a STORE-HELLO command it SHOULD reply with
a CONSUMER-HELLO command. The CONSUMER-HELLO command has two fields: the address
of the consumer and a list of dafka topics it is subscribed to. If the consumer
is not subscribed to any dafka topic it MUST NOT send a CONSUMER-HELLO command.

The XPUB's topic frame is set to the CONSUMER-HELLO command's ID concatenated
with the address of the store the received by the STORE-HELLO command.

#### The RECORD Command

When a producer wishes to publish a record it SHALL use the RECORD command. The
RECORD command contains three fields: the dafka topic this record is published
to, the offset of this record in the partition and the records content defined
as one ZeroMQ frame. Dafka does not support multi-frame message contents.

A producer MUST NOT delete send records before receiving at least one ACK
command.

The XPUB's topic frame is set to the RECORD command's ID concatenated with the
dafka topics name the record is published to.

#### The HEAD Command

After a producer published its first record it SHALL send the HEAD command at
regular a interval. The HEAD command contains two fields: the dafka topic this
producers publishes records to and the offset of the last published record.

The XPUB's topic frame is set to the HEAD command's ID concatenated with the
address of the producer (partition).

#### The FETCH Command

A consumer or store SHALL send the FETCH command if it detects it missed
a record in a partition. The FETCH command contains three fields: the dafka
topic it missed the topic, the offset first missed record and the count of
missed records.

The XPUB's topic frame is set to the FETCH command's ID concatenated with the
consumers address.

#### The DIRECT-RECORD Command

Upon receiving a FETCH command a producer or store SHALL check if it has stored
the requested record(s). In case a requested record is available it SHALL send
a DIRECT-RECORD command. The DIRECT-RECORD command contains three fields: the
dafka topic this record was published to, the offset of the record in the
partition and the records content defined as one ZeroMQ frame.

The records requested by the FETCH command SHALL be send according to their
offset in ascending order.

The XPUB's topic frame is set to the DIRECT-RECORD command's ID concatenated
with the address of the producer or store.

#### The GET-HEADS Command

If a consumer subscribes to a dafka topic it SHALL sent a GET-HEADS command in
order to get the heads for each partition of that topic.

The XPUB's topic frame is set to the GET-HEADS command's ID concatenated with
the topic for which partition heads are requested for.

#### The DIRECT-HEAD Command

After receiving a GET-HEADS command a producer SHALL send a DIRECT-HEAD command
if it publishes records to dafka topic requested by the GET-HEADS command.

After receiving a GET-HEADS command a store SHALL send one DIRECT-HEAD command
for each partition it has records stored on the requested dafka topic by the
GET-HEADS command.

The XPUB's topic frame is set to the DIRECT-HEAD command's ID concatenated with
address of the producer or store.

### ZeroMQ Subscriptions

Because Dafka nodes are connected through PUB and SUB sockets each node MUST
register subscriptions with the other nodes in order to receive messages. The
following sections describe which node type requires which subscriptions.

#### Producer Subscriptions

A producer SHALL subscribe to ACK commands for its own partition by
concatenating the ACK command ID and the partition name which equals the
producer's address.

A producer SHALL subscribe to FETCH commands for its own partition by
concatenating the FETCH command ID and the partition name which equals the
producer's address.

#### Consumer Subscriptions

A consumer SHALL subscribe to DIRECT-RECORD commands addressed to it by
concatenating the DIRECT-RECORD command ID and its consumer address.

A consumer SHALL subscribe to DIRECT-HEAD commands addressed to it by
concatenating the DIRECT-HEAD command ID and its consumer address.

A consumer SHALL subscribe to STORE-HELLO commands by concatenating the
STORE-HELLO command ID and the consumer address.

For each dafka topic a consumer is subscribed to it SHALL subscribe to RECORD
commands by concatenating the RECORD command ID and topic name.

For each dafka topic a consumer is subscribed to it SHALL subscribe to HEAD
commands by concatenating the HEAD command ID and topic name.

For each partition of each dafka topic a consumer is subscribed to it MAY
subscribe to ACK commands by concatenating the ACK command ID and the partition
name which equals the producer's address.

#### Store Subscriptions

A store SHALL subscribe to all RECORD commands by subscribing the RECORD command
ID.

A store SHALL subscribe to all HEAD commands by subscribing the HEAD command ID.

A store SHALL subscribe to DIRECT-RECORD commands addressed to it by
concatenating the DIRECT-RECORD command ID and its store address.

A store SHALL subscribe to all FETCH commands by subscribing to the FETCH
command ID.

A store SHALL subscribe to GET-HEADS commands by subscribing to the GET-HEADS
command ID.

A store SHALL subscribe to CONSUMER-HELLO commands by subscribing to the
CONSUMER-HELLO command ID
