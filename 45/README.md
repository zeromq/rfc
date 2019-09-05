---
domain: rfc.zeromq.org
shortname: 45/ZWS 2.0
name: ZeroMQ WebSocket Protocol 2.0
status: draft
editor: Doron Somech <somdoron@gmail.com>
---

The ZeroMQ WebSocket 2.0 (ZWS2.0) protocol is a mapping for ZeroMQ over WebSocket.

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[]()").

## Goals

The ZeroMQ WebSocket Protocol (ZWS) is a transport layer protocol for exchanging messages between two peers over [WebSocket](http://tools.ietf.org/html/rfc6455).
This document describes ZWS/2.0.

## Implementation

ZWS connection is starting after successful WebSockets handshake between the client and the server.
ZWS message are binary websocket messages (the message opcode must be binary).

### Handshake

WebSocket connection initiation and handshake between the client and the server is defined in [rfc6455](http://tools.ietf.org/html/rfc6455).
ZWS handshake request MUST include at least one protocol as part of `Sec-WebSocket-Protocol`.
ZWS define the following protocols:
* ZWS2.0 - No mechanism.
* ZWS2.0/NULL - See [Null mechanism](https://rfc.zeromq.org/spec:23/ZMTP/#the-null-security-mechanism)
* ZWS2.0/PLAIN - See [Plain mechanism](https://rfc.zeromq.org/spec:24/ZMTP-PLAIN/)
* ZWS2.0/BEARER - not yet implemented

Host and path parts of the request MUST be as set in the user in the connect method (e.g `zmq_connect(s, "ws://server.example.com/zeromq")`).

Example Client Request:
```
GET /zeromq HTTP/1.1
Host: server.example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==
Sec-WebSocket-Protocol: ZWS2.0,ZWS2.0/NULL
Sec-WebSocket-Version: 13
Origin: http://example.com
```

Server must choose one of the client protocols and set `Sec-WebSocket-Protocol` to the chosen protocol as part of Websocket handshake.
If none of the protocols are supported by the server the server should closed the connection (with BAD REQUEST, WebSocket close control message or silently drop the TCP connection).

Example Server Reply:
```
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=
Sec-WebSocket-Protocol: ZWS2.0
```

### No Mechanism

All implementations of ZWS2.0 must implement the no mechanism protocol, the reason for this protocol in addition to the NULL protocol is for zeromq ports that don't implement ZMTP3.0 and the various mechanisms.

ZWS2.0 without mechanism is similar to [ZMTP/2](https://rfc.zeromq.org/spec:15/ZMTP/), the first message of each side of the connection must be the Routing Id (Identity), the rest are regular messages.

#### Formal BNF

```
zws = *connection

connection = identity *message

identity = message-last 

;   A message is one or more frames
message = *message-more message-last
message-more = %x01 message-body
message-last = %x00 message-body
message-body = *OCTET
```

### Mechanisms

ZWS define 3 mechanisms that can be used:

* NULL - as specified by [ZMTP3.0 rfc](https://rfc.zeromq.org/spec:23/ZMTP/#the-null-security-mechanism), which implements no authentication.
* PLAIN - as specified by [ZMTP-PLAIN rfc](https://rfc.zeromq.org/spec:24/ZMTP-PLAIN), which implements simple user-name and password authentication.
* BEAER - no specification yet, which implements bearer authentication.

#### Format BNF

```
;   The protocol consists of zero or more connections
zmtp = *connection

;   A connection a handshake, and traffic
connection = handshake traffic

;   The handshake consists of at least one command
;   The actual grammar depends on the mechanism
handshake = 1*command

;   Traffic consists of commands and messages intermixed
traffic = *(command | message)

;   A command is a single frame
command = %x02 command-body
command-body = command-name command-data
command-name = OCTET 1*255command-name-char
command-name-char = ALPHA
command-data = *OCTET

;   A message is one or more frames
message = *message-more message-last
message-more = %x01 message-body
message-last = %x00 message-body
message-body = *OCTET
```

### Framing

Each ZeroMQ frame maps to one WebSockets message. A frame consists of a FLAG byte followed by frame body.

Flags can be:
* 0x00 - Final message
* 0x00 - More message
* 0x02 - Command message 

### Socket Compatibility

ZWS implementations don't have to implement the all sockets type and can choose which socket type to implement.

The implementation SHOULD enforce that an incoming connection has a valid socket type, depending on the socket type of the socket receiving the connection:

* PAIR accepts connections from PAIR.
* PUB accepts connections from SUB.
* SUB accepts connections from PUB.
* REQ accepts connections from REP or ROUTER.
* REP accepts connections from REQ or DEALER.
* DEALER accepts connections from REP, DEALER, or ROUTER.
* ROUTER accepts connections from REQ, DEALER, or ROUTER.
* PULL accepts connections from PUSH.
* PUSH accepts connections from PULL.
* SERVER accepts connections from CLIENT.
* CLIENT accepts connections from SERVER.
* RADIO accepts connections from DISH.
* DISH accepts connections from RADIO.
* SCATTER accepts connections from GATHER.
* GATHER accepts connections from SCATTER.

Any other socket combinations SHOULD be handled by silently disconnecting the other peer and possibly logging the error for debugging purposes.

### Publish-Subscribe

XPUB and XSUB sockets are implemented at the protocol level as PUB and SUB sockets. That is, XPUB and XSUB are API constructs only.
A SUB socket sends a subscription message as 0x01 byte followed by the subscription body, and unsubscription messages as 0x00 byte '0' followed by the subscription body.

## Security

The various mechanisms provide authentication. 
However, ZWS2.0 makes no attempt at encryption of the wire.

Clients support tunnel over SSL with specifying wss instead of ws, for example "wss://example.com".

Load balancer might terminate the SSL before arriving on the server.