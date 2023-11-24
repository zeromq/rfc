---
slug: 53
title: 53/BLOCKCHAIN
name: Blockchain over ZeroMQ
status: draft
editor: Justus Ranvier <justus@opentransactions.org>
---

This document specifies a system for encapsulating the native message types of various blockchain network protocols which allows messages corresponding to multiple independent blockchain networks to be multiplexed over a single connection.

## Preamble

Copyright (c) 2023 The Open-Transactions Project

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <https://www.gnu.org/licenses>.

This Specification is a [free and open standard](https://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](https://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](https://tools.ietf.org/html/rfc2119).

## Overall Design

### Problem Description

The launch of the original blockchain currency Bitcoin in 2009 created a new p2p network using tcp port 8333. At some later point Bitcoin developers decided to create an independent test network using port 18333. Around the same time other developers decided to fork Bitcoin's code and create independent currencies. Usually these independent currencies switch to using a new tcp port, however in a few cases projects have elected to retain the same tcp port as the network from which they have forked.

While the original Bitcoin p2p network design did not contain any explicit provisions for coexistence with alternate networks, the header format for the network messages does include a 4 byte start string (a.k.a magic bytes) patter. As with the port issue, however, not all cryptocurrencies use unique start strings.

In addition to blockchain networks that are derived from Bitcoin's protocol, several other completely independent protocols exist. This includes most notably but not limited to, the Ethereum family of currencies.

Port / address management can be become a significant annoyance for entities which need to operate blockchain nodes for many different networks especially when the set of networks to be operated includes those with colliding port requirements. It is impossible, for example, for both a Bitcoin node and a Bitcoin Cash node to listen for incoming connections on the same ip address because they both claim the same port for their respective networks.

In addition to the port collision issue described above, the original Bitcoin p2p protocol has other limitations which can be addressed by encapsulating the messages in ZeroMQ.

The goal of this protocol is to create a blockchain overlay network which allows nodes to participate in an arbitrary number of independent networks using a single tcp port.

### Unique Identification

Multiple networks can not be multiplexed over the same connection unless it is possible to uniquely identify the network so that the messages can be demultiplexed correctly.

As mentioned in the previous section, neither the native port number nor the protocol start string is collision-free.

The hash of the genesis block is more plausible as a uuid, however given the demonstrated existence of ledger forks this is also falls short.

For the purposes of this protocol, the task of providing unique network identification is outsourced to [SLIP-0044](https://github.com/satoshilabs/slips/blob/master/slip-0044.md) based on the historical observation that even when one project forks from another while attempting to claim various aspects of the original project, they always desire hardware wallet compatibility in order to attract users and thus reliably seek registration in the SLIP-0044 list.

## Formal Specification

### Architecture

Every participant in this protocol is a peer which MAY accept incoming connections.

In the context of any particular connection one node acts as the listening peer and the other node acts as the connecting peer.

* The listening peer SHALL use a ROUTER socket.
* The listening peer SHALL bind a tcp endpoint using port 8816.
* The listening peer SHALL act as a CurveZMQ server per [26/CURVEZMQ](https://rfc.zeromq.org/spec:26/CURVEZMQ).
* The listening peer SHALL perform ZAP authentication using the "blockchain" domain per [27/ZAP](https://rfc.zeromq.org/spec:27/ZAP).
* The listening peer MAY implement an access control policy based on whitelisting, blacklisting, or another other method.

* The connecting peer SHALL use a ROUTER or DEALER socket.
* The connecting peer SHALL connect to the listening peer's endpoint.
* The connecting peer SHALL act as a CurveZMQ client per [26/CURVEZMQ](https://rfc.zeromq.org/spec:26/CURVEZMQ).
* The connecting peer SHALL perform ZAP authentication using the "blockchain" domain per [27/ZAP](https://rfc.zeromq.org/spec:27/ZAP).

### Message Format

Each native blockchain protocol messages is encapsulated into a single multi-part ZeroMQ message.

The message SHALL consist of the following message frames:

* An address delimiter frame, which SHALL have a length of zero.
* The protocol identifier frame, which SHALL contain the two octets <code>0x000C</code> (3072 little endian encoded).
* The *base chain identifier*, which SHALL contain four octets.
* The *subchain identifier*, which SHALL contain one octet.
* The *message type*, which SHALL contain between 1 and 16 octets
* The *payload*, additional data for the specified message type which MAY be empty

#### Base chain identifier

The base chain identifier is the SLIP-0044 code for the network whose message is being encapsulated if the network is a main chain, or the associated main net if the network in question is a test network. For example the base chain identifier of BTC testnet3 is 1.

The base chain identifier SHALL be encoded as a two octet little endian integer.

#### Subchain identifier

The subchain identifier is used to distinguish between related networks which share a common base chain.

The currently defined values for the subchain identifier are:

| Subchain  | value |
| --------- | ----- |
| primary   | 1     |
| testnet1  | 128   |
| testnet2  | 129   |
| testnet3  | 130   |
| testnet4  | 131   |
| testnet5  | 132   |
| testnet6  | 133   |
| testnet7  | 134   |
| testnet8  | 135   |
| testnet9  | 136   |
| testnet10 | 137   |
| chipnet   | 252   |
| scalenet  | 253   |
| signet    | 254   |
| regtest   | 255   |

#### Message type ####

This field SHALL consist of the message identifier used by the native blockchain protocol, typically an ASCII string. The field SHALL NOT be zero-terminated.

#### Payload ####

The payload SHALL consist of exactly the payload bytes which occur in the native protocol message. Native header bytes are not included since these can be generated from the other fields in the ZeroMQ message if desired.

## Security Aspects

Blockchain over ZeroMQ messages have the same security considerations as the un-encapsulated messages. Blockchain applications generally assume all received messages are potentially attacker-controlled as a standard practice.

The use of CurveZMQ and ZAP in this protocol provides and additional tool for access control by allowing for the possibility of user-based authentication instead of address-based authentication.

CurveZMQ eliminates the need for protocol-level checksums and secures the messages in transmit from eavesdropping, which is not generally the case for the un-encapsulated messages.

## Reference Implementation

This protocol is implemented by [libopentxs](https://github.com/Open-Transactions/opentxs) and is used by [Métier-server](https://github.com/Open-Transactions/metier-server) and libopentxs-based wallet applications.

## See also ##

[54/ZMQ-ADDRV2](https://rfc.zeromq.org/spec:54/ZMQ-ADDRV2)
