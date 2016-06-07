---
domain: rfc.zeromq.org
shortname: 38/ZMTP-GSSAPI
name: ZMTP GSSAPI
status: draft
editor: Chris Busbey <cbusbey@connamara.com>
---

The ZMTP GSSAPI mechanism provides secure authentication and confidentiality for [ZMTP 3.0](http://rfc.zeromq.org/spec:23). This mechanism utilizes the [Generic Security Service Application Interface](http://tools.ietf.org/html/rfc2743).

See also: http://tools.ietf.org/html/rfc2743

## Preamble

Copyright (c) 2013 iMatix Corporation.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

The ZMTP GSSAPI mechanism provides secure authentication and confidentiality for [ZMTP 3.0](http://rfc.zeromq.org/spec:23). This mechanism utilizes the [Generic Security Service Application Interface](http://tools.ietf.org/html/rfc2743), which provides security services to callers in a generic fashion.

## Implementation

The GSSAPI mechanism uses the greeting as-server field to identify which peer is "client" and which peer is "server".

The mechanism starts with a HELLO command from client to server, and continues with a handshake until each party has authenticated the other and is ready to send confidential information.

All command bodies consist of an 8-character command name, padded with spaces, followed by formatted binary fields. Command bodies may or may not be encrypted dependenting on the underlying security mechanism. The http://rfc.zeromq.org/spec:23 ZMTP 3.0 specification] defines the grammar for the command size. The [ http://tools.ietf.org/html/rfc2743 GSSAPI specification] defines the grammar for the command bodies.
