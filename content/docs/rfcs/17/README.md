---
slug: 17
title: 17/ZDCF
aliases: [/spec:17/ZDCF]
name: ZeroMQ Device Configuration File
status: stable
editor: Pieter Hintjens <ph@imatix.com>
contributors:
  - Steffen Mueller <smueller@cpan.org>
---

The ZeroMQ Device Configuration File (ZDCF) specifies a standard language for configuring 0MQ devices. It provides information to configure a 0MQ context, and a set of 0MQ sockets. This specification aims to make it easier to build, share, and reuse 0MQ devices and build systems for device administration.

## License

Copyright (c) 2010-2012 iMatix Corporation and contributors

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

ZDCF aims to:

* Provide a standard reusable semantic for device configuration.
* Allow compatible expression in any tree-structured syntax.
* Be as widely accessible as possible for different programming languages.
* Cover all 0MQ context and socket configuration options.

## Architecture

ZDCF uses tree-structured semantics that can be implemented using arbitrary syntaxes such as ZPL (see "[ZFL Property Language](http://rfc.zeromq.org/spec:4)"), JSON (see "[Introducing JSON](http://json.org/)"), XML, or others. Despite the fact that this document and the name "ZDCF" refer to files, there is no technical reason against ZDCF implementations that accept in-memory data structures such as nested dictionaries or hashmaps.

In the following specification, any reference to a dictionary is to be understood as any data structure that maps a string key to another data structure.

### Global properties

At the top level, a ZDCF structure is a dictionary. An empty dictionary is special cased to represent an empty ZDCF; this is a valid JSON-specified file:

```
{
}
```

Any other ZDCF file is required to include the version of the ZDCF specification that it was written against as a floating point number in the "version" attribute of the top level dictionary. See below for a description on how implementations are to handle version information.

In addition to the specification version, the top level dictionary of a ZDCF file may contain "apps", a dictionary of application names mapping to the definition of a stand-alone application. Each such application contains an optional context object and zero or more device objects. Conceptually, one application maps to one process, consisting of a single context and zero or more device threads.

Here is a typical example of a ZDCF file expressed in JSON:

```
{
    "version": 1.0,
    "apps": {
        "listener": {
            "context": {
                "iothreads": 1,
                "verbose": true
            },
            "devices": {
                "main": {
                    "type": "zmq_queue",
                    "sockets": {
                        "frontend": {
                            "type": "SUB",
                            "option": {
                                "hwm": 1000,
                                "swap": 25000000
                            },
                            "bind": "tcp://eth0:5555"
                        },
                        "backend": {
                            "bind": "tcp://eth0:5556"
                        }
                    }
                }
            }
        }
    }
}
```

Here is the same property tree expressed in ZPL (see "[ZFL Property Language](http://rfc.zeromq.org/spec:4)"):

```
version = 1.0
apps
    listener
        context
            iothreads = 1
            verbose = 1
        devices
            main
                type = zmq_queue
                sockets
                    frontend
                        type = SUB
                        option
                            hwm = 1000
                            swap = 25000000
                        bind = tcp://eth0:5555
                    backend
                        bind = tcp://eth0:5556
```

And in simple XML:

```
<zdcf>
    <version>1.0</version>
    <apps>
        <context iothreads = "1" verbose = "1" />
        <main type = "zmq_queue">
            <sockets>
                <frontend type = "SUB">
                    <option hwm = "1000" swap = "25000000" />
                    <bind>tcp://eth0:5555</bind>
                </frontend>
                <backend>
                    <bind>tcp://eth0:5556</bind>
                </backend>
            </sockets>
        </main>
    </apps>
</zdcf>
```

### The Application Object

As stated above, each application object represents one process that may contain multiple 0MQ devices running in threads. An application may have a context object as the "context" entry of the dictionary. It may have a dictionary "devices" mapping device names to device objects. In summary an application can have these properties:

* Its name in the "apps" dictionary.
* "context" - An optional context object
* "devices" - A dictionary of zero or more device names mapped to device objects

### The Context Object

The context object is optional in each application and has these properties:

* Its name is "context" in the application dictionary.
* "iothreads" - (integer) - specifies the number of I/O threads for the context. Defaults to 1 if not specified or if there is no explicit context object in the application.
* "verbose" - (boolean) - if "true", the program parsing the JSON should output tracing information. Defaults to "false" if not specified.

### The Device Object

Device objects can occur zero or more times in the "devices" section of an application and have these properties:

* Their name is unique in the "devices" dictionary of their application.
* "type" - (string) - optional. Specifies the device type. Types starting with "z" are reserved for built-in 0MQ devices. Other device types may be defined by the application as needed.
* "sockets" - A dictionary of zero or more socket names mapped to socket objects

The built-in device types that exist at time of writing are:

* "zmq_queue" - ZMQ_QUEUE
* "zmq_forwarder" - ZMQ_FORWARDER
* "zmq_streamer" - ZMQ_STREAMER

See [zmq_device(3)](http://api.zeromq.org/zmq_device.html) for details on the built-in device types.

### The Socket Object

Socket objects can occur zero or more times in the "sockets" section of a device object, and have these properties:

* The name is unique within the enclosing device.
* "type" - (string) - specifies the type of the socket.
* "bind" - (string) - specifies zero or more endpoints to bind the socket to.
* "connect" - (string) - specifies zero or more endpoints to connect the socket to.
* "option" - (object) - a socket option object that specifies configuration of the socket.

The socket types that exist at time of writing are:

* "sub" - ZMQ_SUB
* "pub" - ZMQ_PUB
* "req" - ZMQ_REQ
* "rep" - ZMQ_REP
* "dealer" - ZMQ_DEALER
* "router" - ZMQ_ROUTER
* "push" - ZMQ_PUSH
* "pull" - ZMQ_PULL
* "pair" - ZMQ_PAIR

See [zmq_socket(3)](http://api.zeromq.org/zmq_socket.html) for details.

### The Socket Option Object

A socket option object is optional inside a socket object. It has these properties:

* Its name is "option".
* "hwm" - (integer) - specifies the ZMQ_HWM option.
* "swap" - (integer) - specifies the ZMQ_SWAP option.
* "affinity" - (integer) - specifies the ZMQ_AFFINITY option.
* "identity" - (string) - specifies the ZMQ_IDENTITY option.
* "subscribe" - (string) - specifies the ZMQ_SUBSCRIBE option.
* "rate" - (integer) - specifies the ZMQ_RATE option.
* "recovery_ivl" - (integer) - specifies the ZMQ_RECOVERY_IVL option.
* "mcast_loop" - (Boolean) - specifies the ZMQ_MCAST_LOOP option.
* "sndbuf" - (integer) - specifies the ZMQ_SNDBUF option.
* "rcvbuf" - (integer) - specifies the ZMQ_RCVBUF option.

See [zmq_setsockopt(3)](http://api.zeromq.org/zmq_setsockopt.html) for details.

### Value Arrays

Properties may be specified as value arrays where it makes sense and at least for:

* The socket "bind" property.
* The socket "connect" property.
* The option "subscribe" property.

For example:

```
    "frontend": {
        "option": {
            "subscribe": [ "10001", "10002" ]
        },
        "bind": [ "tcp://eth0:5555", "inproc://device" ]
    }
```

## Handling Specification Versions

As an author of a ZDCF file, you must include the version of the specification that you wrote the ZDCF file against. The version of the specification is a floating point number and must be compared as such. Any change in the fractional part is considered a backwards-compatible change. Any change in the integer part of the specification version is understood to be backwards-incompatible.

A software that consumes ZDCF content must adhere to the following rules:

* It shall accept any spec-conformant ZDCF content that was written against a specification version that it claims to support.
* It shall not accept any ZDCF content that was written against a specification with a greater integer version than what the ZDCF-consuming software was written to accept.

In other words, you are free to implement backwards-compatibility with older versions of the specification, but please don't silently try to accept content that was written against a specification that you did not implement. Your users will thank you for it. If at the time of implementation, the specification version that you had access to is 5.3, then you may accept content written against 5.4 assuming it's a specification change that is backwards compatible. ZDCF 6.0, however, you should reject until you've updated your implementation to handle it.
