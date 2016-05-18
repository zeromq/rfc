This document proposes a Message Transport Layer (MTL), a connection-oriented protocol that supports broker-based messaging. MTL connects a set of clients with a central message broker, allowing clients to issue commands to the broker, send messages to the broker, and receive messages back from the broker.

* Name: http://rfc.zeromq.org/spec:11/MTL
* Editor: Pieter Hintjens <ph@imatix.com>

## License

Copyright (c) 2011 the Authors.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

Note that in this document we do not use the term "message" formally since the term is overloaded at several levels. The name of this specification needs to be reviewed.

## Goals

MTL is a connection-oriented protocol that supports broker-based messaging. It connects a set of clients with a broker, allowing clients to issue commands to the broker, send content to the broker, and receive content back from the broker.

The main goals of MTL are:

* To support arbitrary messaging semantics based on extensible *profiles*.
* To provide a simple synchronous flow for commands from the client to the server.
* To provide a fast asynchronous flow for data from the client to or from the server.
* To provide authentication of clients using SASL (see "[RFC 2222 - Simple Authentication and Security Layer (SASL)](http://www.faqs.org/rfcs/rfc2222.html)").
* To provide safe forwards and backwards compatibility.
* To provide detection of errors such as dead or blocked peers.
* To provide robust error handling.

## General Operation

MTL separates all client-server activity into two distinct flows:

* A synchronous low-volume *control flow*. The control flow is strictly request-response, where one request by the client always results in one response from the server. The server never sends a response without a request from the client. The client detects a dead or absent server using a timeout on its request socket.

* An asynchronous high-volume *data flow*. The data flow is bidirectional and asynchronous, where either client or server may send various types of frame at any time. Either client or server may use heart-beating to detect a dead peer.

The control flow carries JSON (see "[]()") commands. The intention is to make control content trivial to parse, extend, and debug.

The data flow carries binary-encoded commands and content. The intention is to make data content fast to decode and encode, and compact on the wire.

## Control Flow

Control commands go over a request-reply flow from client to server. The server opens a 0MQ ROUTER socket, and binds to its well-known endpoint. The client opens a REQ socket and connects to this endpoint. The client then issues a request, and the server a response. The connection lasts forever, or until the client closes its socket, or until the server unbinds from its endpoint.

Every request from a client to the server follows this format:

* Frame 0: command name (string)
* Frame 1: request body (JSON string)

Every reply from the server to a client follows this format:

* Frame 0: status code (three digits, 2xx-5xx)
* Frame 1: reply body (JSON string)

Commands are broken into *classes*, where each class provides a set of *methods*. Each method consists of a client request and a server reply, serialized as JSON. Server replies are categorized by status code.

Command names are case-insensitive.

### Client Authentication

On a new connection, the client MUST first send a Connection.Open command. This requests access to a specific *virtual host* on the server:

```
Connection.Open
------------------------------------------------------------
{
    "protocol": {
        "name": "MTL",
        "version": 1
    },
    "virtual-host": "test-env"
}
```

Clients are authenticated using the SASL challenge-response model. A server MAY accept to work with unauthenticated clients. It will then reply 201 Ready:

```
201
------------------------------------------------------------
{
    "status": "Ready",
    "profiles": [ "profile name", "profile name",... ]
}
```

To challenge a client, the server responds with 401 Unauthorized and specifies a list of SASL mechanisms that it accepts:

```
401
------------------------------------------------------------
{
    "status": "Unauthorized",
    "mechanisms": [ "mechanism name", "mechanism name",... ]
}
```

The server MAY issue a 401 response at any time, e.g. if a client's authentication information changed, or expired. The client MUST be able to re-authenticate at any time.

To authenticate, a client sends Connection.Authorize:

```
Connection.Authorize
------------------------------------------------------------
{
    "mechanism" : "mechanism name",
    "response" : "BASE64-encoded blob"
}
```

If the SASL challenge/response cycle finished, the server replies with 201 Ready. Otherwise the server responds with 402 Challenged, after which the client MUST re-send a new Connection.Authorize with new response data:

```
402
------------------------------------------------------------
{
    "status": "Challenged",
    "mechanism": "mechanism name",
    "challenge" : "BASE64-encoded blob"
}
```

The SASL challenge-response cycle can repeat multiple times.

### Selecting a Profile

To start working with a specific profile, the client issues a Connection.Profile command:

```
Connection.Profile
------------------------------------------------------------
{
    "profile" : "profile name"
}
```

The profile name must be one of the profiles specified by the server when it issued 201 Ready.

If the profile was valid, the server responds with 200 OK. If not, the server responds with 400 Bad Request.

```
200
------------------------------------------------------------
{
    "status": "OK"
}
```

Profile names are case-insensitive.

Each profile defines a set of classes that the client can use to work with server-side resources. A "test" profile is defined later in this document.

### Requesting a Data Lease

A data flow can either read from a set of resources, or write to a set of resources. To open a data flow, the client requests a "lease", using either the Connection.Reader or Connection.Writer command:

```
Connection.Reader
------------------------------------------------------------
{
    "resources": [ "resource name", "resource name", ...],
    "confirm": "none | full"
}
```

```
Connection.Writer
------------------------------------------------------------
{
    "resources": [ "resource name", "resource name", ...],
    "confirm": "none | full"
}
```

The client MUST have already selected a profile before making a Connection.Reader or Connection.Writer request.

The resources list specifies which resources the request applies to. An empty list means "any and all resources on the server".

The "confirm" field specifies whether messages are confirmed or not. If the server accepts the Connection.Reader or Connection.Writer request it responds with a 202 Data Lease:

```
202
------------------------------------------------------------
{
    "status": "Data Lease",
    "port": port number
    "lease" : "data lease"
}
```

If the profile does not support the requested confirmation model, the server MUST respond with a 501 Not Implemented:

```
501
------------------------------------------------------------
{
    "status": "Not implemented"
}
```

### Control Flow Error Handling

The server SHOULD silently drop ill-formed requests, that is, requests with the wrong number of frames, or with content that is not valid JSON. The server MUST respond to well-formed requests that are invalid for any reason with a suitable error reply.

## Data Flow

Data commands go over an asynchronous bidirectional flow between client to server. The server opens a 0MQ ROUTER socket and binds it to a port, which it communicates to the client in a 202 reply. The client opens a DEALER socket and connects to the server endpoint, using this port. The client and server can then issue data commands. The connection lasts forever, or until the client closes its socket, or until the server unbinds from its endpoint.

The server MAY handle data flows on the same socket as control flows, or it MAY open separate sockets for data flows.

### Data Command Encoding

Every data command follows this format:

* Frame 0: header (binary encoded)
* Frame 1: body (binary content)

The header frame is encoded thus:

* 1-byte command type
* Optional fields

Depending on the command type, the command type is followed by zero or more optional fields, whose encoding and meaning are specified per command type.

### Initializing a Data Flow

When a client has opened a data connection it must send a Lease command as follows:

* Type = 0x01, Lease
 * Field 1: command sequence (integer)
 * Field 2: data lease provided by server 202 response (short string)
* Body: unused

The server MUST reply with a Response command that echoes the command sequence used by the client, as follows:

* Type = 0x02, Response
 * Field 1: command sequence (integer)
* Body: three-digit response code

Where the response code can be:

* "200" - the lease was successful
* "403" - the lease was forbidden

The purpose of the command sequence in this and other commands is to allow the client to pipeline commands, i.e. to send multiple commands without waiting for the response to each. The server MAY return Response commands in any order.

### Summoning a Resource

To start receiving content from a resource, the client sends a Summon command as follows:

* Type = 0x03, Summon
 * Field 1: command sequence (integer)
 * Field 2: resource name (short string)
* Body: unused

Where the resource name MUST be one of the resources specified by the Connection.Reader request for this data lease. The client MAY NOT summon the same resource twice without an intervening dismiss.

The server MUST reply with a Response command, where the response code can be:

* "200" - the Summon command was successful
* "400" - the resource cannot be summoned.

Following a successful Summon command, the server-side resource MAY send Content commands to the client.

### Dismissing a Resource

To stop receiving content from a resource, the client sends a Dismiss command as follows:

* Type = 0x04, Dismiss
 * Field 1: command sequence (integer)
 * Field 2: resource name (short string)
* Body: unused

Where the resource name MUST be one of the resources specified by the Connection.Reader request for this data lease. The client MAY NOT dismiss the same resource twice without an intervening summon.

The server MUST reply with a Response command, where the response code can be:

* "200" - the Consume command was successful
* "400" - the resource cannot be dismissed.

Following a successful Dismiss command, the server-side resource MUST NOT send further Contents to the client.

### Sending Content to a Resource

To send content to a resource, the client sends a Content command as follows:

* Type = 0x05, Content
 * Field 1: content sequence (integer)
 * Field 2: resource name (short string), may be empty
 * Field 3: properties (field table)
* Body: opaque binary content

If the Connection.Writer request specified more than one resource, then the resource name is required.

### Receiving Content from a Resource

When a resource has content to send to a client, the server sends a Content command as follows:

* Type = 0x05, Content
 * Field 1: content sequence (integer)
 * Field 2: resource name (short string), may be empty
 * Field 3: properties (field table)
* Body: opaque binary content

The server MUST send the Response to a Summon command before any Content from that resource, and the server MUST NOT send any Content from a resource after a successful Response to a Dismiss command. If the Connection.Reader request specified more than one resource, then the resource name is required.

### Content Confirmation

All Content commands in one direction on a data flow are either confirmed, or not confirmed, depending on the "confirm" value specified in the corresponding Connection.Reader or Connection.Writer:

* "none" - Content commands are not confirmed.
* "full" - Content commands are confirmed.

The confirmation works as follows:

* One peer sends one or more Content commands.
* The recipient confirms safe receipt of one or more Content commands with a Confirm command.
* The recipient rejects one or more Content commands with a Reject command.

The Confirm command has this format:

* Type = 0x06, Confirm
 * Field 1: content sequence (integer)
 * Field 2: multiple (Boolean)
* Body: unused

If the 'multiple' field is TRUE, the Confirm command covers all Contents up to and including the specified content sequence. If the 'multiple' field is FALSE, the Confirm covers exactly one Content.

There is no response to a Confirm command. Invalid Confirm commands are ignored silently.

The Reject command has this format:

* Type = 0x07, Reject
 * Field 1: content sequence (integer)
 * Field 2: multiple (Boolean)
 * Field 3: recycle (Boolean)
* Body: unused

If the 'multiple' field is TRUE, the Reject command covers all Contents up to and including the specified content sequence. If the 'multiple' field is FALSE, the Reject covers exactly one Content.

if the 'recycle' field is TRUE, the Content will be resent using a heuristic that depends on the profile.

There is no response to a Reject command. Invalid Reject commands are ignored silently.

### Heartbeating

The client or server may heartbeat an otherwise silent data connection by sending Ping commands:

* Type = 0x08, Ping
* Body: unused

The recipient of a Ping command should respond with a Pong command:

* Type = 0x09, Pong
* Body: unused

A client or server SHOULD send Ping commands when it is not otherwise sending commands. The frequency of Ping commands should be configurable.

A client or server SHOULD consider its peer "disconnected" if no Pong arrives within some specific time period.

### Field Encodings

These are the possible field encodings:

* Short string: a single byte followed by 0 to 255 bytes of printable characters.
* Integer: an eight-byte (64-bit) unsigned value represented in network byte order.
* Shortint: a two-byte (16-bit) unsigned value represented in network byte order.
* Boolean: a single byte with the possible values 0 (FALSE) or 1 (TRUE).
* Field table: to be defined.

### Data Flow Error Handling

The server SHOULD silently drop ill-formed requests, that is, requests with the wrong number of frames, or with content that is not valid. The server MUST respond to well-formed requests that are invalid for any reason with a suitable error reply.

## Request and Reply Routing

Servers MUST use ROUTER sockets. From the ØMQ Reference Manual (see "[]()"):

> When receiving messages a ROUTER socket shall prepend a message part containing the identity of the originating peer to the message before passing it to the application. When sending messages a ROUTER socket shall remove the first part of the message and use it to determine the identity of the peer the message shall be routed to.

This extra frame is ignored in this discussion.

## The Test Profile

The "test" profile defines the following routing and queuing semantics for content:

* The server provides exactly one resource, called "resource".
* Content commands are never confirmed.
* Content is fanned-out (duplicated) from all writers to all readers.

The server and the client MAY implement the test profile.

## Implementations

* https://github.com/imatix/zmetal

