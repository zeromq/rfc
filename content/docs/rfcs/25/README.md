---
title: 25/ZMTP-CURVE
aliases: [/spec:25/ZMTP-CURVE]
name: ZMTP CURVE
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

The ZMTP CURVE mechanism provides secure authentication and confidentiality for [ZMTP 3.0](http://rfc.zeromq.org/spec:23). This mechanism implements the [CurveZMQ security protocol](http://curvezmq.org). It is intended for use on public networks where security requirements are high.

See also: http://rfc.zeromq.org/spec:23/ZMTP, http://rfc.zeromq.org/spec:24/ZMTP-PLAIN.

## Preamble

Copyright (c) 2013 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

The ZMTP CURVE mechanism provides secure authentication and confidentiality for [ZMTP 3.0](http://rfc.zeromq.org/spec:23). This mechanism implements the [CurveZMQ security protocol](http://curvezmq.org), which is a mapping of the [CurveCP](http://curvecp.org) handshake, adapted for TCP transports.

## Implementation

The CURVE mechanism uses the greeting as-server field to identify which peer is "client" and which peer is "server".

The mechanism starts with a HELLO command from client to server, and continues with a handshake until each party has authenticated the other and is ready to send confidential information.

All command bodies consist of an 8-character command name, padded with spaces, followed by formatted binary fields. The http://rfc.zeromq.org/spec:23 ZMTP 3.0 specification] defines the grammar for the command size. The [CurveZMQ specification](http://rfc.zeromq.org/spec:26) defines the grammar for the command bodies.
