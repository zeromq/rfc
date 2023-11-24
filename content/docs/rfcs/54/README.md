---
slug: 54
title: 54/ZMQ-ADDRV2
name: ZMQ_ADDRV2
status: draft
editor: Justus Ranvier <justus@opentransactions.org>
---

This document specifies an extension to [BIP 155](https://github.com/bitcoin/bips/blob/master/bip-0155.mediawiki) <code>addrv2</code> messages to accommodate gossip of [Blockchain over ZeroMQ](https://rfc.zeromq.org/spec:53/BLOCKCHAIN) peers.

## Preamble

Copyright (c) 2023 The Open-Transactions Project

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <https://www.gnu.org/licenses>.

This Specification is a [free and open standard](https://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization"s [Consensus-Oriented Specification System](https://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](https://tools.ietf.org/html/rfc2119).

## Problem Description

BIP 155 specifies address encoding rules for ipv4, ipv6, tor v2, tor v3, i2p, and cjdns.

None of these are suitable for encoding Blockchain over ZeroMQ peer information since at a minimum the server's public key is also required for a successful connection.

## Formal Specification

Except where noted in this specification and [53/BLOCKCHAIN](https://rfc.zeromq.org/spec:53/BLOCKCHAIN), message construction is identical to [BIP 155](https://github.com/bitcoin/bips/blob/master/bip-0155.mediawiki).

This message type is only applicable to blockchain networks which use BIP-155 format <code>addrv2</code> messages.

### Message Format

For a [53/BLOCKCHAIN](https://rfc.zeromq.org/spec:53/BLOCKCHAIN) peer:

The BIP-155 *Network ID* value peer SHALL have a value of <code>0x5A</code>.

The BIP-155 *Network address* SHALL consist of the following:

    * 32 octets containing the peer's public CurveZMQ key
    * 1 octet subtype
    * 4 - 32 octet address. The length depends on the subtype.

The BIP-155 *Network port* SHALL have a value of 8816.

### Subtypes

The subtype SHALL be treated as a *Network ID* for the purposes of interpreting the encoded address.

Thus, a Blockchain over ZeroMQ peer can be validly represented as a ipv4, ipv6, tor, i2p, or cjdns address using this message, although it may be impossible to translate a tor or i2p address into a valid endpoint.

A subtype of <code>0x5A</code> is invalid and MUST be ignored.

### Translation to ZeroMQ endpoints

An <code>addrv2</code> address entry may be translated to a ZeroMQ <code>tcp://</code> endpoint using the standard representation of the encoded address and the port

## Reference Implementation

This protocol is implemented by [libopentxs](https://github.com/Open-Transactions/opentxs).

## Security Aspects

This specification has no security aspects.

## See also ##

[53/BLOCKCHAIN](https://rfc.zeromq.org/spec:53/BLOCKCHAIN)
