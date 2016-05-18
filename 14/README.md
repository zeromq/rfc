Worker-Manager Protocol is a generalization of request-reply pattern, allowing many workers talk to many managers (servers) with intermediate devices and custom load-balancing. **This paper is a rather brief description of protocol, it lacks details and is not complete. I will do my best to finish it and to provide a reference implementation as soon as possible.**

* Name: http://rfc.zeromq.org/spec:14/WMP
* Editor: Brugeman Artur <brugeman.artur@gmail.com>
* Contributors: none yet

## License

Copyright (c) 2011 Brugeman Artur

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

## Change Process

This Specification is a free and open standard (see "[Definition of a Free and Open Standard](http://www.digistan.org/open-standard:definition)") and is governed by the Digital Standards Organization's Consensus-Oriented Specification System (COSS) (see "[Consensus Oriented Specification System](http://www.digistan.org/spec:1/COSS)").

## Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 (see "[Key words for use in RFCs to Indicate Requirement Levels](http://tools.ietf.org/html/rfc2119)").

## Goals

The purpose of the protocol is to allow:
1. Many-to-many communications btw workers and managers.
2. Many jobs done simultaneously by each worker.
3. Extensible load-balancing based on idleness of workers.
4. Support for intermediate devices.

## Architecture

### Protocol

The protocol is inspired by gearman (see "[Gearman project](http://gearman.org/)"), for those who are familiar. There are two types of peers: workers (receive requests and send responses), and managers (send requests and receive responses).

Peers exchange messages. Each message has a command, extensible supplementary information ("headers", which may include routing paths, manager's identity, roles, etc.), and may have a payload. There are 6 types of commands, 3 sent by managers and 3 sent by workers.

Managers don't know their workers, they start by binding a socket and waiting for incoming messages.  Workers know their managers (endpoints, etc.), they start by connecting to each manager and initiating message exchange.

++++ Scheme

```
 W               M
   --- READY -->   Worker is ready
   <-- WAKE  ---   Have job for you
   --- GIVE  -->   Give me some job
   <-- TAKE  ---   Take the job
       or
   <-- NO    ---   No job for you
   --- DONE  -->   Job is done
```

++++ Description
1. To get a job, worker sends a READY command to all his managers.
2. Manager receives READY, and remembers an available worker (routing path to worker) until a job is available. If another READY message is received with routing path that is already remembered, the message is ignored.
3. When a job is available, manager sends WAKE to all corresponding workers (correspondance is defined by custom load-balancing solutions, or it may be "any worker" in the simplest case).
4. Worker receives a WAKE command, and checks whether it can do the job. If it is busy (no free resources), WAKE message is ignored. Otherwise, a part of worker's resources needed to process the job is "locked" (marked as requested by a particular manager), and GIVE command is sent to the requesting manager.
5. Manager receives GIVE and checks whether there is a job for the worker. If there is a job, a message with TAKE command and a job payload is sent to requesting worker. If there is no job for the worker (already given to another one), a NO command is sent to requesting worker.
6. When worker receives GIVE command, it processes the job, "unlocks" resources taken by job's manager, and replies with DONE command with result as a payload.
7. If worker receives NO command, it simply "unlocks" resources taken by job's manager.

### Load-balancing

As noted in p.3 above, the protocol provides load-balancing based on availability (idleness) of workers. This basis may be extended by workers supplying additional information (e.g. "roles") to managers, for managers to use that information to determine the suitable workers for each job. Such functionality is application specific and is out of scope of this protocol.

### Multiple jobs

To allow for many jobs processed by a single worker, a READY message should be sent right after receiving a job (TAKE message), but before processing it. Managers will process each READY command as a separate request for a job, and will give another job to the worker.

### Exceptional cases

To prevent workers' resources from being locked for infinity, there is a timeout. If timeout expires, worker will unlock it's resources. This is a rare scenario, as manager will usually reply with either TAKE or NO.

Workers may retry READY commands after some period just in case a manager has forgotten the previous requests. It's a good idea to use some backoff strategy to prevent spoiling network with READY commands.

### Intermediate devices

Intermediate devices work as a worker on one side, and manager on the other. Device is given a list of managers it's worker side should connect to. Rules:
1. READY message (an initial message in conversation) received by manager-side is transferred by worker-side to all list of managers.
2. All other messages from both sides are transferred using routing path information to the next peer.

End nodes must know identities of the peers they are talking to. To allow that with intermediate devices in place, an end node identity is taken from the routing path (an origin of path), with one exception (see Routing, p.2).

### Routing

To allow routing, as message is passed from end to end, all intermediate peers add routing information to allow message to be returned back using the same path.

As an exception w/ respect to other commands, TAKE command is sent with manager's identity specified. This extension allows managers to forward job request, received from elsewhere, as defining manager's identity from routing path is impossible if manager is in the middle of path. Workers respect manager's identity specified and use that instead of routing path.

### Reliability

All reliability issues, such as jobs timing out, de-duplication of job requests and responses, retries, etc. are handled at application level. The protocol's purpose is to allow load-balancing in many-to-many request-reply with devices scenario, not to provide any sort of reliability.

### Extensions

To extend functionality offered by the protocol, two more commands could be introduced. STATUS - sent by workers to indicate the status of a job. And a CANCEL command, sent by managers, to inform worker that no job result is expected and job processing can be cancelled.

