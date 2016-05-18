The File Message Queuing Protocol (FILEMQ) governs the delivery of files between a 'client' and a 'server'. FILEMQ runs over the ZeroMQ [ZMTP v3 protocol](http://rfc.zeromq.org/spec:23/ZMTP). This is version 2 of the FILEMQ protocol.

* Name: http://rfc.zeromq.org/spec:35/FILEMQ
* Editor: Pieter Hintjens <ph@imatix.com>

## Preamble

Copyright (c) 2009-2014 iMatix Corporation

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119).

## Goals

The FILEMQ protocol defines a mechanism for wide-area file distribution using a publish-subscribe pattern. Its goals are:

* To allow clients to "subscribe" to server-hosted directories with zero out-of-band knowledge of the files that will be created on those directories.
* To provide high-speed chunked file delivery to clients.
* To allow the clients to cancel and restart file transfers arbitrarily.
* To be fully secure, using the ZMTP v3 security mechanisms.

## Implementation

### Formal Grammar

The following ABNF grammar defines the FILEMQ protocol:

```
FILEMQ          = open-peering *use-peering [ close-peering ]

open-peering    = C:OHAI ( S:OHAI-OK / error )

use-peering     = C:ICANHAZ ( S:ICANHAZ-OK / error )
                / C:NOM
                / S:CHEEZBURGER
                / C:HUGZ S:HUGZ-OK
                / S:HUGZ C:HUGZ-OK

close-peering   = C:KTHXBAI / S:KTHXBAI

error           = S:SRSLY / S:RTFM

;     Client opens peering
ohai            = signature %d1 protocol version
signature       = %xAA %xA3             ; two octets
protocol        = string                ; Constant "FILEMQ"
version         = number-2              ; Protocol version 2

;     Server grants the client access
ohai_ok         = signature %d4

;     Client subscribes to a path
icanhaz         = signature %d5 path options cache
path            = string                ; Full path or path prefix
options         = dictionary            ; Subscription options
cache           = dictionary            ; File SHA-1 signatures

;     Server confirms the subscription
icanhaz_ok      = signature %d6

;     Client sends credit to the server
nom             = signature %d7 credit sequence
credit          = number-8              ; Credit, in bytes
sequence        = number-8              ; Chunk sequence, 0 and up

;     The server sends a file chunk
cheezburger     = signature %d8 sequence operation filename offset eof headers chunk
sequence        = number-8              ; File offset in bytes
operation       = number-1              ; Create=%d1 delete=%d2
filename        = string                ; Relative name of file
offset          = number-8              ; File offset in bytes
eof             = number-1              ; Last chunk in file?
headers         = dictionary            ; File properties
chunk           = chunk                 ; Data chunk

;     Client or server sends a heartbeat
hugz            = signature %d9

;     Client or server answers a heartbeat
hugz_ok         = signature %d10

;     Client closes the peering
kthxbai         = signature %d11

;     Server refuses client due to access rights
srsly           = signature %d128 reason
reason          = string                ; Printable explanation

;     Server tells client it sent an invalid message
rtfm            = signature %d129 reason
reason          = string                ; Printable explanation

; A list of name/value pairs
dictionary      = dict-count *( dict-name dict-value )
dict-count      = number-4
dict-value      = longstr
dict-name       = string

; A chunk has 4-octet length + binary contents
chunk           = number-4 *OCTET

; Strings are always length + text contents
string          = number-1 *VCHAR
longstr         = number-4 *VCHAR

; Numbers are unsigned integers in network byte order
number-1        = 1OCTET
number-2        = 2OCTET
number-4        = 4OCTET
number-8        = 8OCTET
```

### Interconnection Model

#### ZeroMQ Socket Types

The server SHALL create a ROUTER socket and SHOULD bind it to port 5670, which is the registered Internet Assigned Numbers Authority (IANA) port for FILEMQ. The server MAY bind its ROUTER socket to other ports in the ephemeral port range (%C000x - %FFFFx). The client SHALL create a DEALER socket and connect it to the server ROUTER host and port.

Note that the ROUTER socket provides the caller with the connection identity of the sender for any message received on the socket, as an identity frame that precedes other frames in the message.

#### Protocol Signature

Every ZeroMQ message SHALL start with the FILEMQ protocol signature, %xAA %xA3. The server and client SHALL silently discard any message received that does not start with these two octets.

This mechanism is designed particularly for servers that bind to ephemeral ports which may have been previously used by other protocols, and to which there are still peers attempting to connect. It is also a general fail-fast mechanism to detect ill-formed messages.

#### Connection State

The server SHALL reply to an unexpected command with a RTFM command. The client SHALL respond to an RTFM command by closing its DEALER connection and starting a new connection.

### FILEMQ Commands

#### The OHAI Command

The client SHALL start a new connection by sending the OHAI command to the server. This command identifies the protocol and version. This is designed to allow version detection.

If the server does not support the request protocol version it SHALL reply with a RTFM command. The client MAY try again with a lower protocol version.

If the server accepts the OHAI command it SHALL reply with an OHAI-OK command. If the server does not accept the OHAI command for other reasons, it SHALL reply with a SRSLY command.

#### The OHAI-OK Command

When the server grants the client access after an OHAI command, it SHALL reply with an OHAI-OK command.

#### The ICANHAZ Command

The client MAY subscribe to any number of virtual paths by sending ICANHAZ commands to the server. The client MAY specify a full path, or it MAY specify a partial path, which is used as a prefix match. Paths MUST start with "/", thus the path "/" subscribes to *everything*.

The 'path' does not have to exist in the server. That is, clients can request paths which will exist in the server at some future time.

The 'options' field provides additional information to the server. The server SHOULD implement these options:

* <tt>RESYNC=1</tt> - if the client sets this, the server SHALL send the full contents of the virtual path to the client, except files the client already has, as identified by their SHA-1 digest in the 'cache' field.

When the client specifies the RESYNC option, the 'cache' dictionary field tells the server which files the client already has. Each entry in the 'cache' dictionary is a "filename=digest" key/value pair where the digest SHALL be a SHA-1 digest in printable hexadecimal format. If the filename starts with '/' then it SHOULD start with the path, otherwise the server MUST ignore it. If the filename does not start with '/' then the server SHALL treat it as relative to the path.

#### The ICANHAZ-OK Command

When a server accepts a subscription it MUST reply with an ICANHAZ-OK command. If the server refuses a subscription it SHALL reply with a SRSLY command, and discard any further commands from this client.

#### The NOM Command

The client MUST initiate the transfer of data by sending credit to the server. The server SHALL only send as much data to the client as it has credit for. The credit is an amount in bytes that corresponds to actual file content (but not bytes used by commands themselves).

The client MAY sent NOM commands at any point after it has received an OHAI-OK from the server. The server SHALL not respond directly to NOM commands.

#### The CHEEZBURGER Command

The server SHALL send file content to the client using CHEEZBURGER commands. Each CHEEZBURGER command shall deliver a chunk of file data starting at a specific offset. The server MUST send the content of a single file as consecutive chunks and clients MAY depend on this behavior.

The headers field is reserved for future use.

#### The HUGZ Command

The server or client MAY sent heartbeat commands at any point after the server has sent OHAI-OK to the client, which has received it.

The HUGZ command acts as a heartbeat, indicating that the peer is alive. The server and client SHALL treat *any* command from a peer as a sign that the peer is alive.

A peer may thus choose to only send HUGZ to another peer when it is not sending any other traffic to that peer.

#### The HUGZ-OK Command

A peer SHALL respond to a HUGZ command with a HUGZ-OK command. This allows one peer to be responsible for all heartbeating.

#### The KTHXBAI Command

The client MAY end a connection by sending the KTHXBAI command to the server. The server SHALL not respond to this command.

#### The SRSLY Command

The server SHALL respond to any failed attempt to access a resource on the server with a SRSLY command. This includes failed subscriptions. When a client receives a SRSLY command it SHOULD close the connection and if needed, reconnect with new authentication credentials.

#### The RTFM Command

The server SHALL respond to an invalid command by sending RTFM. Note that the server SHALL not send RTFM to clients which send an invalid protocol signature. When a client receives a RTFM command it SHOULD close the connection and not reconnect.

## Security Aspects

FILEMQ v2 uses the ZMTP v3 transport layer security mechanisms (NULL, PLAIN, CURVE, etc.) The SHA-1 digest used for file cache identification has no security implications.

## Reference Implementation

The reference implementation for this protocol is at [filemq.org](http://filemq.org).
