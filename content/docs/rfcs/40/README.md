---
slug: 40
title: 40/XRAP
aliases: [/spec:40/XRAP]
name:  Extensible Resource Access Protocol
status: stable
editor: Pieter Hintjens <ph@imatix.com>
---

This document describes the Extensible Resource Access Protocol (XRAP), a RESTful protocol built over ZeroMQ. XRAP provides a single, extensible answer to the problem of how to work with remote resources. We designed XRAP to avert the development of fragile domain-specific RPC protocols.

## Preamble

Copyright (c) 2015 the Contributors.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

This Specification is a [free and open standard](http://www.digistan.org/open-standard:definition) and is governed by the Digital Standards Organization's [Consensus-Oriented Specification System](http://www.digistan.org/spec:1/COSS).

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](http://tools.ietf.org/html/rfc2119), "Key words for use in RFCs to Indicate Requirement Levels".

## Overview

### Use Case

Our generic use case is a server that holds a set of resources belonging to, and managed by, a set of remote clients. We wish to build APIs, based on network protocols that allow clients and servers to be distributed across both local and wide area networks. We hold that such protocols are essential for the clean separation of components of a distributed architecture.

Protocols define agreed and testable contracts. Our resource access protocol has two levels of contract:

* Agreement on the resources that the server provides; the names and properties, and semantics of such resources; and their relationship to one another. This consists of any number of independent "resource schemas".

* Agreement on how to represent resources on the wire, how to navigate the resource schema, create new resources, retrieve resources, return errors, and so on. This is the "access protocol", which XRAP provides.

The main goal of XRAP is to make it extremely cheap to develop, test, improve, and discard resource schemas. That is, to allow specialists in this domain to develop good schemas with minimal marginal cost. XRAP achieves this economy is by splitting off, and independently solving the access protocol.

* In technical terms, it allows the provision of 100% reusable solutions (libraries and internal APIs in arbitrary languages) for the access and transport layers.

* In specification terms, it allows the formal specification of schemas as small independent RFCs.

### XRAP vs. RPC

A primary use of network serialization tools like protobufs and zproto is to build domain-specific RPC protocols. These are easy to make and use, yet create significant upstream costs:

* They are typically structured into methods + arguments, with little or no organization.
* They are typically mapped closely to the software domain design, i.e. they are not abstracted.
* As a result, they will usually change with every release of the software.
* This makes them useless as interoperable contracts, and extremely fragile for large distributed systems.

In effect, an RPC protocol attempts to fit all semantics into a single contract, and this demands that all components in the system are updated in parallel. This is expensive even inside a single organization. It is pathological for wider distribution.

XRAP (like REST, which it is based on) provides only four methods (POST, GET, PUT, DELETE) and moves all extensibility into resources, as defined by schemas. This is a proven and simple way to get future extensibility. Each peer can choose to implement the document semantics it chooses. Each document type can evolve independently, as its own contract.

### Design of the Protocol

XRAP defines a consistent protocol for creating, retrieving, updating, and deleting remote resources. To reduce the learning curve and element of surprise, we have used [RESTful design principles](http://www.ics.uci.edu/~fielding/pubs/dissertation/rest_arch_style.htm), and we have kept HTTP/1.1 compatibility.

REST is a design pattern rather than a formal specification. It is not formal about e.g. one updates a resource using PUT, or POST, nor does it explain how to represent resources. It is also incomplete in areas such as asynchronous event delivery. XRAP formalizes these missing areas. It is a reusable specification. However, it does not claim to be universal or complete. It provides sufficient abstractions to implement some basic schemas, and no more.

### Network Transports

In this document we use ZMTP as the reference transport. However XRAP may use other transports, such as HTTP. The mapping of XRAP over other transports SHALL be defined by separate RFCs.

### Performance Tradeoffs

XRAP is mostly a verbose, request-reply protocol that uses self-describing documents (JSON or XML) for content encoding. This is the easiest form of encoding to work with and extend. It is trivial for implementors to add new properties, for experimentation or specialization. It is trivial to ignore unknown properties.

## Technical Specifications

### Formal Grammar

The following ABNF grammar defines the XRAP serialization over ZMTP:

```
xrap_msg        = *( POST | POST-OK
                   | GET | GET-OK | GET-EMPTY
                   | PUT | PUT-OK
                   | DELETE | DELETE-OK
                   | ERROR )

;  Create a new, dynamically named resource in some parent.

POST            = signature %d1 tracker parent content_type content_body
signature       = %xAA %xA5             ; two octets
tracker         = number-4              ; request tracker
parent          = string                ; Schema/type/name
content_type    = string                ; Content type
content_body    = longstr               ; New resource specification

;  Success response for POST.

POST-OK         = signature %d2 tracker status_code location etag date_modified
                  content_type content_body metadata
status_code     = number-2              ; Response status code 2xx
location        = string                ; Schema/type/name
etag            = string                ; Opaque hash tag
date_modified   = number-8              ; Date and time modified
content_type    = string                ; Content type
content_body    = longstr               ; Resource contents
metadata        = hash                  ; Collection total size/version/hypermedia

;  Retrieve a known resource.

GET             = signature %d3 tracker resource parameters if_modified_since
                  if_none_match content_type
resource        = string                ; Schema/type/name
parameters      = hash                  ; Filtering/sorting/selecting/paging
if_modified_since = number-8            ; GET if more recent
if_none_match   = string                ; GET if changed
content_type    = string                ; Desired content type

;  Success response for GET.

GET-OK          = signature %d4 tracker status_code etag date_modified content_type
                  content_body metadata
status_code     = number-2              ; Response status code 2xx
etag            = string                ; Opaque hash tag
date_modified   = number-8              ; Date and time modified
content_type    = string                ; Actual content type
content_body    = longstr               ; Resource specification
metadata        = hash                  ; Collection total size/version/hypermedia

;  Conditional GET returned 304 Not Modified.

GET-EMPTY       = signature %d5 tracker status_code
status_code     = number-2              ; Response status code 3xx

;  Update a known resource.

PUT             = signature %d6 tracker resource if_unmodified_since if_match
                  content_type content_body
resource        = string                ; Schema/type/name
if_unmodified_since = number-8          ; Update if same date
if_match        = string                ; Update if same ETag
content_type    = string                ; Content type
content_body    = longstr               ; New resource specification

;  Success response for PUT.

PUT-OK          = signature %d7 tracker status_code location etag date_modified metadata
status_code     = number-2              ; Response status code 2xx
location        = string                ; Schema/type/name
etag            = string                ; Opaque hash tag
date_modified   = number-8              ; Date and time modified
metadata        = hash                  ; Collection total size/version/hypermedia

;  Remove a known resource.

DELETE          = signature %d8 tracker resource if_unmodified_since if_match
resource        = string                ; schema/type/name
if_unmodified_since = number-8          ; DELETE if same date
if_match        = string                ; DELETE if same ETag

;  Success response for DELETE.

DELETE-OK       = signature %d9 tracker status_code metadata
status_code     = number-2              ; Response status code 2xx
metadata        = hash                  ; Collection total size/version/hypermedia

;  Error response for any request.

ERROR           = signature %d10 tracker status_code status_text
status_code     = number-2              ; Response status code, 4xx or 5xx
status_text     = string                ; Response status text

; A list of name/value pairs
hash            = hash-count *( hash-name hash-value )
hash-count      = number-4
hash-value      = longstr
hash-name       = string

; Strings are always length + text contents
string          = number-1 *VCHAR
longstr         = number-4 *VCHAR

; Numbers are unsigned integers in network byte order
number-1        = 1OCTET
number-2        = 2OCTET
number-4        = 4OCTET
number-8        = 8OCTET
```

### XRAP Methods and Arguments

A basic XRAP interaction consists of one request and one reply. The request consists of a method, a set of headers, and a content body. An XRAP client uses these methods to work with server-side resources:

* POST - create a new, dynamically named resource.
* GET - retrieve the representation of a known resource.
* PUT - update a known resource.
* DELETE - remove a known resource.

All methods work orthogonally on all types of resources, whether those resources contain or do not contain other resources. Not all combinations of methods and resources are meaningful, permitted, or necessarily implemented in any eventual API based on XRAP.

We use these headers, with their HTTP/1.1 meanings, implemented as individual fields within each method that uses them:

* Location - a URN that identifies newly created resource.
* ETag - an opaque hash tag that identifies a resource instance.
* Date-Modified - the date and time that resource was modified.
* If-Modified-Since - a conditional GET using resource date.
* If-None-Match - a conditional GET using resource ETag.
* If-Unmodified-Since - a conditional PUT or DELETE using resource date.
* If-Match - a conditional PUT or DELETE using resource ETag.

Requests MAY carry a tracker, which is a non-zero integer. If the client request has a tracker, the server MUST include this in the corresponding reply.

Additionally we can pass arguments to GET as we usually would do with a HTTP query.

* Parameters - A map of key/value pairs for filtering, sorting, selecting and paging. Parameter key names are case-insensitive.

Further all _OK messages take a map of key/value pairs to supply further metadata like the total size of a collection, hypermedia e.g. links for paging through a collection or the returned version of a document.

* Metadata - A map of key/value pair containing custom data to further describe the response. Metadata key names are case-insensitive.

All replies contain a 3-digit status code that conforms to the [HTTP/1.1 specifications](http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html) for status codes.

### XRAP Resources

Server-side resources are either **public** (shared by many clients) or **private** (to a single client). Public resources are named by formal or informal agreement, while the server alone is responsible for naming private resources. The URN of a resource is based on the resource name, so clients can know the URNs of public resources in advance, while the server provides the URNs of private resources at runtime.

Resources are either **structured** or **opaque**. A structured resource has properties that both client and server can access and modify. Resources can contain references (URNs) to child resources. An opaque resource has a MIME type and a binary content that intermediate layers never examine nor modify.

The resource types form a static **schema** attached to a root type. The set of actual resources that a server holds form a dynamic **resource tree**, attached to a public root resource. To navigate the resource tree, clients retrieve the root resource and inspect it. Not all resources are discoverable: to work with a private resource an client must have created it, and thus know its URN.

We can hold structured resources as interchangeable XML or JSON **resource documents**, at the choice of the client. XRAP specifies the grammar for representing resources and their properties.

#### Resource Names (URNs)

Resources can be created by various parties at different times: by the server at startup, by clients at runtime, or by the server as the consequence of other events, at runtime. Only the party that created a resource can delete it, and it may place restrictions on other parties' right to work with the resource.

The URN for a public resource is derived from the resource type and name as follows:

```
/{schema}/{resource type}/{resource name}
```

The corresponding URI is prefixed by a protocol and endpoint (e.g. the hostname and port):

```
{transport}://{endpoint}/{schema}/{resource type}/{resource name}
```

The fields have this meaning:

* transport - the name of the transport, e.g. "http" or "zmtp".
* endpoint - a network endpoint, which depends on the transport.
* schema - the name of the resource schema, which may not contain '/' characters.
* resource type - the name of the resource type, which may not contain '/' characters.
* resource name - the name of the resource itself.

The URN for a private resource follows an identical format, where resource type is "resource" and resource name is a hash generated by the server:

```
{transport}://{endpoint}/{schema}/resource/{resource hash}
```

Note that "resource" is a reserved word and SHALL NOT be used as a resource type name.

### Creating a Resource

Clients create new resources as follows:

* The client must know the URN of the parent for the new resource it wants to create.
* The client POSTs a specification for the new resource to the parent URN.
* The client creates a public resource by specifying a name. Otherwise the server names the resource, which is then private.
* The server either returns a status code 2xx with a resource document, or 4xx or 5xx with an error text.
* The server, after creating the resource, returns "201 Created" with the new URN in the Location: header.

Here is the walkthrough from client to server:

```
Client                                     Server
|                                           |
|  1.) POST to parent URN                   |
|      Resource specifications              |
|------------------------------------------>|
|                                           |
|  2.) 201 Created                          |
|      Location: Resource URN               |
|<------------------------------------------|
|                                           |
```

The content body of the created resource may be different than the content body provided by the client. While the client provides specifications for the resource, the server returns the actual resulting resource, which may have additional properties and/or children.

This is the general form of a client request to create a dynamic resource, with the server response. We assume we're using XML rather than JSON:

```
Client:
-------------------------------------------------
POST /{parent urn}
Content-Type: application/{schema}+xml

<?xml version="1.0"?>
<{schema} xmlns="http://digistan.org/schema/{schema}">
<{resource type} [name = "{name for public resource}"]>
    {resource specifications}
</{resource type}>
</{schema}>

Server:
-------------------------------------------------
201 Created
Content-Type: application/{schema}+xml
Date-Modified: {resource date and time}
ETag: {resource entity tag}
Location: {resource URN}

<?xml version="1.0"?>
<{schema} xmlns="http://digistan.org/schema/{schema}">
<{resource type} name="{resource name}"...>
    {resource contents}
</{resource type}>
</{schema}>
```

The server may return these response codes specifically for a POST request:

* 200 OK - the resource already existed as specified (only possible for public resources).
* 201 Created - the server created the resource, and the Location: header provides the URN.
* 400 Bad Request - the resource specification was incomplete or badly formatted.
* 403 Forbidden - the POST method is not allowed on the parent URN.

In case of a 4xx or 5xx error response the server will return a textual error message in the content body of the response.

Creating public resources is **idempotent**, i.e. repeated requests to create the same public resource are allowed, and safe. Clients should treat "200 OK" and "201 Created" as equally successful.

### Retrieving a Resource

Clients retrieve resources as follows:

* The client must know the URN of the resource that it wants to retrieve.
* The client sends a GET method to the resource URN.
* The client optionally specifies headers to do a conditional retrieval.
* The server either returns "200 OK" with a resource document, "304 Not Modified" with no content, or 4xx or 5xx with an error text.

Here is the walkthrough from client to server:

```
Client                                     Server
|                                           |
|  1.) GET to Resource URN                  |
|------------------------------------------>|
|                                           |
|  2.) 200 OK                               |
|      Resource representation              |
|<------------------------------------------|
|                                           |
```

This is the general form of an unconditional client request to retrieve a resource, with the server response. We assume we're using XML rather than JSON:

```
Client:
-------------------------------------------------
GET /{resource urn}

Server:
-------------------------------------------------
200 OK
Content-Type: application/{schema}+xml
Date-Modified: {resource date and time}
ETag: {resource entity tag}

<?xml version="1.0"?>
<{schema} xmlns="http://digistan.org/schema/{schema}">
<{resource type} ...>
    {resource contents}
</{resource type}>
</{schema}>
```

The server may return these response codes specifically for a GET request:

* 200 OK - the resource already existed as specified (only for public resources).
* 304 Not Modified - the client already has the latest copy of the resource.

In case of a 4xx or 5xx error response the server will return a textual error message in the content body of the response.

**Conditional retrieval** is used to cache resources and fetch them only if they have changed. Clients retrieve resources conditionally as follows:

* The client must previously have retrieved the resource.
* The client does a GET method with If-None-Match: and If-Modified-Since: headers.
* The server returns "200 OK" and the resource document if the client's copy is out of date.
* The server returns "304 Not Modified" with no content if the client's copy is up-to-date.

This is the general form of a client request to conditionally retrieve a resource:

```
Client:
-------------------------------------------------
GET /{resource urn}
If-None-Match: {resource entity tag}
If-Modified-Since: {resource date and time}
```

A conditional get only takes effect if there were no other errors, i.e. if the result would otherwise be "200 OK". It is valid to send either of the If-None-Match: or If-Modified-Since: headers but for best results, use both.

Retrieving a resource has **no side effects**, i.e. repeated requests to retrieve the same resource will provoke the same responses, unless a third party deletes or modifies the resource.

### Updating a Resource

Clients update resources as follows:

* The client must know the URN of the resource that it wants to update.
* The client should have retrieved the resource before updating it.
* The client sends a PUT method to the resource URN with the modified resource document.
* The client optionally specifies headers to do a conditional update.
* The server either returns 2xx with a resource document, or 4xx or 5xx with an error text.

Here is the walkthrough from client to server:

```
Client                                     Server
|                                           |
|  1.) GET to Resource URN                  |
|------------------------------------------>|
|                                           |
|  2.) 200 OK                               |
|      Resource representation              |
|<------------------------------------------|
|                                           |
|  3.) PUT to Resource URN                  |
|      Modified resource representation     |
|------------------------------------------>|
|                                           |
|  4.) 200 OK                               |
|<------------------------------------------|
```

This is the general form of a client request to unconditionally modify a resource, with the server response. We ignore standard headers and we assume we're using XML rather than JSON:

```
Client:
-------------------------------------------------
PUT /{resource urn}
Content-Type: application/{schema}+xml

<?xml version="1.0"?>
<{schema} xmlns="http://digistan.org/schema/{schema}">
<{resource type} ...>
    {resource contents}
</{resource type}>
</{schema}>

Server:
-------------------------------------------------
200 OK
Date-Modified: {resource date and time}
ETag: {resource entity tag}
```

The server may return these response codes specifically for a PUT request:

* 200 OK - the resource was successfully updated.
* 204 No Content - the client provided no resource document and the update request had no effect.
* 400 Bad Request - the resource document was incomplete or badly formatted.
* 403 Forbidden - the PUT method is not allowed on the resource.
* 412 Precondition Failed - the client does not have the latest copy of the resource.

In case of a 4xx or 5xx error response the server will return a textual error message in the content body of the response.

**Conditional update** is used to detect and prevent update conflicts (when multiple clients try to update the same resource at the same time). Clients conditionally update resources conditionally as follows:

* The client must previously have retrieved the resource.
* The client does a PUT method with If-Match: and If-Unmodified-Since: headers.
* The server returns "412 Precondition Failed" if the client's copy is out of date.
* The server returns "200 OK" and the new resource document if the client's copy was up-to-date.

This is the general form of a client request to conditionally update a resource:

```
Client:
-------------------------------------------------
PUT /{resource urn}
If-Match: {resource entity tag}
If-Unmodified-Since: {resource date and time}
```

A conditional PUT only takes effect if there were no other errors, i.e. if the result would otherwise be "200 OK". It is valid to send either of the If-Match: or If-Unmodified-Since: headers but for best results, use both.

Modifying resources is **idempotent**, i.e. repeated requests to modify the same resource are allowed, and safe, unless a third party modifies or deletes the resource.

### Deleting a Resource

Clients delete resources as follows:

* The client must know the URN of the resource that it wants to delete.
* The client should have retrieved the resource before trying to delete it.
* The client sends a DELETE method, specifying the resource URN.
* The client optionally specifies headers to do a conditional delete.
* The server either returns "200 OK", or 4xx or 5xx with an error text.

Here is the walkthrough from client to server:

```
Client                                     Server
|                                           |
|  1.) GET to Resource URN                  |
|------------------------------------------>|
|                                           |
|  2.) 200 OK                               |
|      Resource representation              |
|<------------------------------------------|
|                                           |
|  3.) DELETE to resource URN               |
|------------------------------------------>|
|                                           |
|  4.) 200 OK                               |
|<------------------------------------------|
|                                           |
```

This is the general form of a client request to unconditionally delete a resource, with the server response. We ignore standard headers:

```
Client:
-------------------------------------------------
DELETE /{resource urn}

Server:
-------------------------------------------------
200 OK
```

The server may return these response codes specifically for a DELETE request:

* 200 OK - the resource was successfully updated.
* 403 Forbidden - the DELETE method is not allowed on the resource.
* 412 Precondition Failed - the client does not have the latest copy of the resource.

In case of a 4xx or 5xx error response the server will return a textual error message in the content body of the response.

**Conditional delete** is used to detect and prevent delete conflicts (when multiple clients try to delete the same resource at the same time). Clients conditionally delete resources conditionally as follows:

* The client must previously have retrieved the resource.
* The client does a DELETE method with If-Match: and If-Unmodified-Since: headers.
* The server returns "412 Precondition Failed" if the client's copy is out of date.
* The server returns "200 OK" if the client's copy was up-to-date.

This is the general form of a client request to conditionally delete a resource:

```
Client:
-------------------------------------------------
DELETE /{resource urn}
If-Match: {resource entity tag}
If-Unmodified-Since: {resource date and time}
```

A conditional DELETE only takes effect if there were no other errors, i.e. if the result would otherwise be "200 OK". It is valid to send either of the If-Match: or If-Unmodified-Since: headers but for best results, use both.

Deleting resources is **idempotent**, i.e. repeated requests to delete the same resource are allowed, and safe. Implementations may cache the URNs of deleted resources in order to differentiate between deletes on already-deleted resources, and deletes on resources that never existed.

If the resource to be deleted contains other resources, these are implicitly and silently deleted along with the resource.

### MIME Type Selection

When the client retrieves a resource it MAY specify which MIME type it desires using the content-type field. XRAP allows these content types:

* "application/{schema}+xml"
* "application/{schema}+json"
* "text/xml", or empty, which are equivalent.

The server will respond with a resource document in the requested format and a Content-Type: header of the appropriate type. Note that most browsers will display "text/xml" documents as XML, while they will not show "application/{schema}+xml". Thus it is possible to embed XRAP resource URNs in, for example, HTML documents, with usable results.

When a client creates a new resource or modifies an existing resource, it SHOULD specify the MIME type of the resource document that it is sending to the browser, using the Content-Type: header. XRAP allows the same three values as for the content-type field:

* "application/{schema}+xml"
* "application/{schema}+json"
* "text/xml", or empty, which are equivalent.

If a server does not support the content type requested or provided by the client, it SHOULD return "501 Not Implemented."

### Asynchronous Operation

XRAP emulates REST, which is a synchronous request-reply model. This is simple and robust. However it is excessively expensive for asynchronous event delivery (mainly from server to client), as each event requires a full round trip.

XRAP has one mechanism for asynchronous non-polled resource delivery, namely *asynclets*. An asynclet is a resource that is given a URN identifier before it exists. When the client retrieves the asynclet, the server will respond only when the resource has come into existence.

Asynclets are used in a specific case: when resources sit in some kind of a queue that is retrieved and emptied by the client (using GET and DELETE in a loop). The queue would be the parent resource (the "container"). The queue then contains zero or more existing resources plus a single asynclet.

A normal resource is identified in a container resource by a href attribute, and other attributes:

```
<some-type href="some-URN" ... />
```

An asynclet has the href attribute and a second attribute 'async="1"', telling the client that this resource, though it has a URN, does not in fact yet exist:

```
<some-type href="some-URN" async="1" />
```

When the client retrieves an asynclet, the server waits until the resource is created, within that container, and it then responds to the client with the contents of that new resource.

When the client retrieves the asynclet container, the container resource holds all existing resources plus a single asynclet:

```
<some-type href="some-URN-1" ... />
<some-type href="some-URN-2" ... />
<some-type href="some-URN-3" ... />
<some-type href="some-URN-4" ... />
<some-type href="some-URN-5" async="1" />
```

At any time a new resource may 'arrive' and the URN held by a client for an asynclet will then refer to a real existing resource. This is transparent to the client except that there will be no wait when the client issues a GET on that URN.

### Idempotency and Side-effects

The GET, PUT, and DELETE methods are idempotent: the client can safely issue these more than once. POST is idempotent when creating public resources. POST to create private resources is not idempotent and will create one resource per successful execution.

Idempotency allows the client to safely recover from failures where it did not get a response, yet could not be certain the server did not receive the request. For example, any failure in the network, the client, or an intermediary that happens after the server receives the request but before the client receives the response. The client recovers by simply reissuing any GET, PUT, DELETE, and POST-public requests that were not completed.

The GET method does not modify the state of any resource on the server.

### Pipelining Requests for Performance

The client MAY send requests without waiting for responses. This can improve performance by reducing network round-tripping. The HTTP term for this is "pipelining", and we use HTTP semantics for pipelining. RFC 2616 section 8.1.2.2 says,

> Clients SHOULD NOT pipeline requests using non-idempotent methods or non-idempotent sequences of methods (see section 9.1.2). Otherwise, a premature termination of the transport connection could lead to indeterminate results. A client wishing to send a non-idempotent request SHOULD wait to send that request until it has received the response status for the previous request.

The client MAY pipeline GET, PUT, DELETE and POST-public methods, but SHOULD NOT pipeline POST-private methods. When pipelining requests, the client SHOULD set a non-zero tracker.

The server is permitted to return responses in a different order to that in which they are received.

### Error Responses

When the server returns an error response (4xx or 5xx), the content body MUST be in plain text, and the MIME type MUST be "text/plain". The client can print and log the content body as a text with no parsing or decoding.

## Resource Document Grammar

Most, though not all, resources managed through a XRAP API are represented as structured resource documents.

In generally, a client sends a resource document to the server when it wishes to create a new resource or update a resource. The server sends a resource document to the client in response to a create, retrieve, or update request. Certain resources may be represented as opaque MIME-type blobs, in which case they are posted and retrieved as such, not as structured resource documents.

XRAP resource documents have a schema-independent regular grammar designed to support all the requirements of a RESTful API. The grammar makes it possible for applications to navigate the API as follows:

* Resources are typed, and the type names form the main element names in the document.
* A resource type may be a container for other resource types.
* This hierarchy of resource types forms a static type tree attached to a single root type.
* At runtime, the actual resources form an actual resource tree attached to a single root resource.
* Clients navigate the actual resource tree by retrieving the root resource.
* Resources that are containers will contain URN references to accessible child resources.
* A private resource is accessible only to the client that created it, and knows its URN.

Our goals with this design are:

* To define a document syntax that is easily mapped to arbitrary structured languages including XML and JSON.
* To provide documents that are navigable and discoverable with minimal prior knowledge.
* To deliver the cheap parsing benefits of structured languages without the cost of formal validation.
* To allow unilateral extensibility of the resource hierarchy by server implementations.
* To keep things as simple as possible for XRAP application developers.
* To allow for future evolution in structured data representation.

### Basic syntax rules

XRAP resource documents obey these basic rules:

* All resource documents are part of a **schema** that defines the type tree.
* The resource document has a **document root** element with the name of the schema.
* The document root contains zero or more **resource roots**.
* The resource root elements may contain other elements.
* All elements except the document root correspond to XRAP resources, with the name of the element equal to the resource type.
* Resource properties are represented as element attributes, not as child elements.
* All elements except the document root may repeat 0 or more times.

Further, for XML documents:

* The Content-type MUST be "application/{schema-name}+xml".
* The document root has a single attribute, xmlns="http://digistan.org/schema/{schema-name}". Note that this URL does not need to exist.
* The double quote character MUST be escaped in attribute values, by writing &quot;.

And for JSON documents:

* The Content-type MUST be "application/{schema-name}+json".
* The double quote following character MUST be escaped in value strings, by writing \".

Here is an example of a XRAP document in XML, for a schema called "music":

```
<?xml version="1.0"?>
<music xmlns="http://digistan.org/schema/music">
<playlist name = "default">
    <album
    artist="Echobelly" title="On" released="1995-10-17"
    summary="Underrated, bittersweet guitar rock perfection">
    <track title="Car Fiction" length="2:31" />
    <track title="King of the Kerb" length="3:59" />
    <track title="Great Things" length="3:31" />
    <track title="Natural Animal" length="3:27" />
    <track title="Go Away" length="2:44" />
    <track title="Pantyhose and Roses" length="3:26" />
    <track title="Something Hot in a Cold Country" length="4:01" />
    <track title="Four Letter Word" length="2:51" />
    <track title="Nobody Like You" length="3:52" />
    <track title="In the Year" length="3:31" />
    <track title="Dark Therapy" length="5:30" />
    <track title="Worms and Angels" length="2:38" />
    </album>
</playlist>
</music>
```

The resource tree for the music schema is:

```
playlist
    |
    o- album
        |
        o- track
```

Here is the same document in JSON:

```
{
"music": {
"playlist": [ { "name":"default",
    "album": [ { "artist":"Echobelly", "title":"On", "released":"1995-10-17",
    "summary":"Underrated, bittersweet guitar rock perfection",
    "track": [
        { "title":"Car Fiction", "length":"2:31" },
        { "title":"King of the Kerb", "length":"3:59" },
        { "title":"Great Things", "length":"3:31" },
        { "title":"Natural Animal", "length":"3:27" },
        { "title":"Go Away", "length":"2:44" },
        { "title":"Pantyhose and Roses", "length":"3:26" },
        { "title":"Something Hot in a Cold Country", "length":"4:01" },
        { "title":"Four Letter Word", "length":"2:51" },
        { "title":"Nobody Like You", "length":"3:52" },
        { "title":"In the Year", "length":"3:31" },
        { "title":"Dark Therapy", "length":"5:30" },
        { "title":"Worms and Angels", "length":"2:38" }
    ] }
    ] }
] }
}
```

Note that it is possible to map between JSON and XML without loss. The key rule for XML documents is that properties are represented as element attributes, not as child elements.

### Navigation and discovery

XRAP documents use the following rule to allow navigation and discovery:

* The attribute "href", if present, holds the URN for the resource that the element represents.
* The URN for the root resource is known to both client and server by common agreement.
* The URNs for all non-root resources are generated by the server and may be stored by the client.
* All URNs provided by the server are absolute. The client should use URNs with no scheme or hostname.

Here is an example of a client retrieving a public playlist resource using a GET method, with the server's response. The server is at host.com:

```
Client:
-------------------------------------------------
GET /music/playlist/default

Server:
-------------------------------------------------
200 OK
Content-Type: application/music+xml

<?xml version="1.0"?>
<music xmlns="http://digistan.org/schema/music">
<playlist name="default">
    <album
        artist="Echobelly" title="On"
        href="/music/resource/A1023" />
    <album
        artist="Muse" title="Showbiz"
        href="/music/resource/A0911" />
    <album
        artist="Toumani Diabate" title="Djelika"
        href="/music/resource/A0023" />
</playlist>
</music>
```

To retrieve a specific album, the client uses the URN provided by the server, for example:

```
Client:
-------------------------------------------------
GET /music/resource/A1023

Server:
-------------------------------------------------
200 OK
Content-Type: application/music+xml

<?xml version="1.0"?>
<music xmlns="http://digistan.org/schema/music">
    <album
        artist="Echobelly" title="On" released="1995-10-17"
        summary="Underrated, bittersweet guitar rock perfection"
    <track title="Car Fiction" length="2:31"
        href="/music/resource/A1023/1" />
    <track title="King of the Kerb" length="3:59"
        href="/music/resource/A1023/2" />
    ...
    ...
    <track title="Worms and Angels" length="2:38"
        href="/music/resource/A1023/12" />
    </album>
</music>
```

To retrieve a specific track, the client once again uses the URN provided by the server. Note that in this case the server delivers a content of type "audio/mpeg-3", which the client should process accordingly (and not as XRAP XML or JSON):

```
Client:
-------------------------------------------------
GET http://host.com/music/resource/A1023/5

Server:
-------------------------------------------------
200 OK
Content-Length: 2870112
Content-Type: audio/mpeg-3

...opaque binary content...
```

## Response Status Codes

In this specification, these server reply codes have specific significance:

* 200 OK - the request completed normally.
* 201 Created - a POST request succeeded.
* 204 No Content - an empty PUT request completed, with no effect.
* 304 Not Modified - a conditional GET request completed, with no effect.
* 400 Bad Request - a resource document was invalid.
* 403 Forbidden - a requested method was not allowed on a resource.
* 404 Not Found - the resource did not exist.
* 412 Precondition Failed - a conditional PUT or DELETE was not carried out.

Clients should be capable of handling all errors defined by HTTP/1.1 (even over a ZMTP transport), including:

* 401 Unauthorized - authentication is required.
* 413 Too Large - the request was too large.
* 500 Internal Error - the server suffered an internal error.
* 501 Not Implemented - the requested functionality is not implemented.
* 503 Overloaded - the server was overloaded.

### Handling Unknown Elements and Attributes

Resource documents may contain elements, and unknown attributes on known elements. This is especially likely when the client and server implement different versions of the API, or if the API is extended by a particular client or server implementation.

Clients and servers should tolerate and ignore unknown elements. Neither the client nor the server should maintain unknown elements.

## Transport over ZeroMQ

### Protocol Model

This is the zproto model for the above grammar:

```
<class name = "xrap_msg"
    signature = "5"
    title = "XRAP serialization over ZMTP"
    script = "zproto_codec_c"
    >
This is the XRAP protocol (http://rfc.zeromq.org/spec:40/XRAP):

<include filename = "license.xml" />

<message name = "POST" id = "1">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "parent" type = "string">Schema/type/name</field>
    <field name = "content type" type = "string">Content type</field>
    <field name = "content body" type = "longstr">New resource specification</field>
Create a new, dynamically named resource in some parent.
</message>

<message name = "POST OK" id = "2">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "status code" type = "number" size = "2">Response status code 2xx</field>
    <field name = "location" type = "string">Schema/type/name</field>
    <field name = "etag" type = "string">Opaque hash tag</field>
    <field name = "date modified" type = "number" size = "8">Date and time modified</field>
    <field name = "content type" type = "string">Content type</field>
    <field name = "content body" type = "longstr">Resource contents</field>
    <field name = "metadata" type = "hash">Collection total size/version/hypermedia</field>
Success response for POST.
</message>

<message name = "GET" id = "3">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "resource" type = "string">Schema/type/name</field>
    <field name = "parameters" type = "hash">Filtering/sorting/selecting/paging</field>
    <field name = "if modified since" type = "number" size="8">GET if more recent</field>
    <field name = "if none match" type = "string">GET if changed</field>
    <field name = "content type" type = "string">Desired content type</field>
Retrieve a known resource.
</message>

<message name = "GET OK" id = "4">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "status code" type = "number" size = "2">Response status code 2xx</field>
    <field name = "etag" type = "string">Opaque hash tag</field>
    <field name = "date modified" type = "number" size = "8">Date and time modified</field>
    <field name = "content type" type = "string">Actual content type</field>
    <field name = "content body" type = "longstr">Resource specification</field>
    <field name = "metadata" type = "hash">Collection total size/version/hypermedia</field>
Success response for GET.
</message>

<message name = "GET EMPTY" id = "5">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "status code" type = "number" size = "2">Response status code 3xx</field>
Conditional GET returned 304 Not Modified.
</message>

<message name = "PUT" id = "6">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "resource" type = "string">Schema/type/name</field>
    <field name = "if unmodified since" type = "number" size="8">Update if same date</field>
    <field name = "if match" type = "string">Update if same ETag</field>
    <field name = "content type" type = "string">Content type</field>
    <field name = "content body" type = "longstr">New resource specification</field>
Update a known resource.
</message>

<message name = "PUT OK" id = "7">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "status code" type = "number" size = "2">Response status code 2xx</field>
    <field name = "location" type = "string">Schema/type/name</field>
    <field name = "etag" type = "string">Opaque hash tag</field>
    <field name = "date modified" type = "number" size = "8">Date and time modified</field>
    <field name = "metadata" type = "hash">Collection total size/version/hypermedia</field>
Success response for PUT.
</message>

<message name = "DELETE" id = "8">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "resource" type = "string">schema/type/name</field>
    <field name = "if unmodified since" type = "number" size="8">DELETE if same date</field>
    <field name = "if match" type = "string">DELETE if same ETag</field>
Remove a known resource.
</message>

<message name = "DELETE OK" id = "9">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "status code" type = "number" size = "2">Response status code 2xx</field>
    <field name = "metadata" type = "hash">Collection total size/version/hypermedia</field>
Success response for DELETE.
</message>

<message name = "ERROR" id = "10">
    <field name = "tracker" type = "number" size="4">Request tracker</field>
    <field name = "status code" type = "number" size = "2">Response status code, 4xx or 5xx</field>
    <field name = "status text" type = "string">Response status text</field>
Error response for any request.
</message>

</class>
```

### ZeroMQ Socket Types

The server SHALL create a ROUTER socket and SHOULD bind it to port <to be specified>, which is the registered Internet Assigned Numbers Authority (IANA) port for XRAP. The server MAY bind its ROUTER socket to other ports in the ephemeral port range (%C000x - %FFFFx). The client SHALL create a DEALER socket and connect it to the server ROUTER host and port.

Note that the ROUTER socket provides the caller with the connection identity of the sender for any message received on the socket, as an identity frame that precedes other frames in the message.

### Protocol Signature

Every ZeroMQ message SHALL start with the XRAP/ZMTP protocol signature, %xAA %xA5. The server and client SHALL silently discard any message received that does not start with these two octets.

This mechanism is designed particularly for servers that bind to ephemeral ports which may have been previously used by other protocols, and to which there are still peers attempting to connect. It is also a general fail-fast mechanism to detect ill-formed messages.

### Security Aspects

XRAP/ZMTP uses the ZMTP transport layer security mechanisms (NULL, PLAIN, CURVE, etc.) There are no specific security aspects in this protocol.

## Further Reading

XRAP was originally developed by Pieter Hintjens in 2014 for the EU UNIFY project. The design is heavily influenced by and derived from the [RESTful Transport Layer](http://www.restms.org/spec:1) (RestTL), part of [the RestMS protocol stack](http://www.restms.org). RestTL is itself inspired by the design of [AtomPub](http://www.ietf.org/rfc/rfc4287.txt) (IETF RFC 4287).
