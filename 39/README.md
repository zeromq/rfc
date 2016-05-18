The ZeroMQ WebSocket (ZWS) protocol is a mapping for ZeroMQ over WebSocket.

* Name: http://rfc.zeromq.org/spec:39/ZWTP
* Editor: Doron Somech <somdoron@gmail.com>
* Contributors:

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[]()").

## Goals

The ZeroMQ WebSocket (ZWS) protocol is a mapping for ZeroMQ over WebSocket (http://tools.ietf.org/html/rfc6455).

## Implementation

ZWS connection is starting after successful WebSockets handshake between the client and the server.
ZWS message are binary websocket messages (the message opcode must be binary).

### Handshake

ZWS doesn't define the connection initiation and handshake between the client and the server.

However client handshake request MUST include:
* Sec-WebSocket-Protocol set to "ZWS1.0".
* Query string parameter "type" set to valid socket type.

If client failed to include the above the server should closed the connection (with BAD REQUEST, WebSocket close control message or silently drop the TCP connection).
Following is valid handshake request:

```
GET /?type=DEALER HTTP/1.1
Host: server.example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==
Sec-WebSocket-Protocol: ZWS1.0
Sec-WebSocket-Version: 13
Origin: http://example.com
```

Server handshake response MUST include:
* Sec-WebSocket-Protocol set to "ZWS1.0".

Following is vaid server handshake response:
```
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=
Sec-WebSocket-Protocol: ZWS1.0
```

### Formal Grammar

The following ABNF grammar defines the ZWS/1.0 protocol:

```
zws = *connection

connection = identity *messsage

identity = %x00 body

body = *OCTEC

message     = *more-frame final-frame
final-frame = final body
final       = %x00
more-frame  = more body
more        = %x01

```

### Framing

Each ZeroMQ frame maps to one WebSockets message. A frame consists of a MORE byte (either 0 or 1) followed by frame body.

MORE byte indicate if more frames to follow. A value of 0 indicates that there are no more frames to follow. A value of 1 indicates that more frames to follow.
On messages consisting of a single frame the MORE character MUST be 0.

### Socket Compatibility

ZWS Client implemenation doesn't have to implement the all sockets type and can choose which socket type to implement.

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

Any other socket combinations SHOULD be handled by silently disconnecting the other peer and possibly logging the error for debugging purposes.

### Publish-Subscribe

XPUB and XSUB sockets are implemented at the protocol level as PUB and SUB sockets. That is, XPUB and XSUB are API constructs only.
A SUB socket sends a subscription message as 0x01 byte followed by the subscription body, and unsubscription messages as 0x00 byte '0' followed by the subscription body.

### Port Sharing

ZWS implementation might choose to implement port sharing.

If port sharing implemented multiple sockets can bind to same port by specifiy the resource name.

Client may specify resource in the client handshake request.

Example for server binding: "ws://*:80/chat"

Exampel for client handshake request with resource:

```
GET /chat?type=DEALER HTTP/1.1
Host: server.example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==
Sec-WebSocket-Protocol: ZWS1.0
Sec-WebSocket-Version: 13
Origin: http://example.com
```

## Security

ZWS1.0 makes no attempt at security.

However the protocol can be tunneled over SSL.

Clients support tunnel over SSL with specifying wss instead of ws, for example "wss://example.com".

Load balancer might terminate the SSL before arriving on the server.

