---
domain: rfc.zeromq.org
shortname: 8/MMI
name: Majordomo Management Interface
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

The Majordomo Management Interface (MMI) defines a namespace and set of management services that MDP brokers may provide. MMI is layered on top of the 7/MDP protocol.

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

The Majordomo Management Interface (MMI) defines a namespace and set of management services that MDP brokers may provide. MMI is layered on top of the 7/MDP protocol.

The goals of MMI are to:

* Define a namespace for management services provided by a MDP broker to MDP client applications.
* Define a default set of management services that MMI-compatible brokers SHOULD implement.

## Architecture

### Namespace

An MMI broker implementation MUST handle all services that start with "mmi." internally. It MUST disconnect any workers that attempt to register services in this namespace.

### Default services

An MMI broker implementation MUST implement these services:

* **mmi.service** - accepts a request containing a service name, responds with "200" (Found) if there is at least one worker registered for the service, or "404" (Not found) if there are no workers registered for the service.

Any unimplemented services in the "mmi." namespace MUST result in a reply "501" (Not implemented).

### Reference Implementations

The C99 Majordomo examples from Chapter 4 of the Guide (see "[ØMQ - The Guide](http://zguide.zeromq.org)") act as the prime reference implementation for MMI. Translations of the examples into other languages may be available.
