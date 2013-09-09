//  --------------------------------------------------------------------------
//  Reference implementation for rfc.zeromq.org/spec:27/ZAP
//
//  This implementation demonstrates a server talking to a proxy handler
//  over inproc, talking to an external terminal handler over TCP. The
//  terminal handler implements a PLAIN authentication mechanism.

//  --------------------------------------------------------------------------
//  Copyright (c) 2010-2013 iMatix Corporation and Contributors
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  --------------------------------------------------------------------------

#include <czmq.h>

//  This is a null server task that executes several ZAP requests

static void
server_task (void *args, zctx_t *ctx, void *pipe)
{
    //  We'll use a REQ socket for our request
    void *requestor = zsocket_new (ctx, ZMQ_REQ);
    int rc = zsocket_connect (requestor, "inproc://zeromq.zap.01");
    assert (rc != -1);

    //  Create a valid ZAP request and send it
    zmsg_t *request = zmsg_new ();
    zmsg_addstr (request, "1.0");       //  ZAP version 1.0
    zmsg_addstr (request, "0001");      //  Sequence number
    zmsg_addstr (request, "test");      //  Domain
    zmsg_addstr (request, "192.168.55.1");  //  Address
    zmsg_addstr (request, "BOB");     //  Identity
    zmsg_addstr (request, "PLAIN");     //  Mechanism
    zmsg_addstr (request, "admin");     //  Username
    zmsg_addstr (request, "secret");    //  Password
    rc = zmsg_send (&request, requestor);
    assert (rc == 0);

    //  Get reply and print it out
    zmsg_t *reply = zmsg_recv (requestor);
    zmsg_dump (reply);
    zmsg_destroy (&reply);

    //  Create an invalid (Mechanism) ZAP request and send it
    request = zmsg_new ();
    zmsg_addstr (request, "1.0");       //  ZAP version 1.0
    zmsg_addstr (request, "2");         //  Sequence number
    zmsg_addstr (request, "test");      //  Domain
    zmsg_addstr (request, "192.168.55.1");  //  Address
    zmsg_addstr (request, "BOB");     //  Identity
    zmsg_addstr (request, "BOGUS");     //  Mechanism
    zmsg_send (&request, requestor);

    //  Get reply and print it out
    reply = zmsg_recv (requestor);
    zmsg_dump (reply);
    zmsg_destroy (&reply);

    //  Create an invalid (Identity) ZAP request and send it
    request = zmsg_new ();
    zmsg_addstr (request, "1.0");       //  ZAP version 1.0
    zmsg_addstr (request, "2");         //  Sequence number
    zmsg_addstr (request, "test");      //  Domain
    zmsg_addstr (request, "192.168.55.1");  //  Address
    zmsg_addstr (request, "ALICE");          //  Identity
    zmsg_addstr (request, "PLAIN");     //  Mechanism
    zmsg_addstr (request, "admin");     //  Username
    zmsg_addstr (request, "secret");    //  Password
    zmsg_send (&request, requestor);

    //  Get reply and print it out
    reply = zmsg_recv (requestor);
    zmsg_dump (reply);
    zmsg_destroy (&reply);

    zstr_send (pipe, "finished");
}

//  The internal handler does not do authentication itself but acts as
//  a proxy for external handlers. It receives requests on a frontend
//  socket, and forwards them to a backend socket

static void
internal_handler (void *args, zctx_t *ctx, void *pipe)
{
    //  The frontend implements the ZAP inproc endpoint
    void *frontend = zsocket_new (ctx, ZMQ_ROUTER);
    int rc = zsocket_bind (frontend, "inproc://zeromq.zap.01");
    assert (rc != -1);

    //  The handler binds to a fixed endpoint; in practice you would
    //  use something more flexible for discovery
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    rc = zsocket_bind (backend, "tcp://*:9999");
    assert (rc != -1);

    zstr_send (pipe, "ready");

    //  Use the built-in proxy to do the actual work
    zmq_proxy (frontend, backend, NULL);
}

//  The external handler does the real work; in reality it might
//  access a database. Here we always reply "200 OK".

static void *
external_handler (void *args)
{
    zctx_t *ctx = zctx_new ();
    void *handler = zsocket_new (ctx, ZMQ_REP);
    int rc = zsocket_connect (handler, "tcp://localhost:9999");
    assert (rc != -1);

    char *status_code = "400";
    char *status_text = "Default Deny";

    while (true) {
        //  Get request, print it for fun
        zmsg_t *request = zmsg_recv (handler);
        if (!handler)
            break;              //  Interrupted
        zmsg_dump (request);

        //  Check version number
        char *version = zmsg_popstr (request);
        if (strneq (version, "1.0")) {
            status_code = "500";
            status_text = "Version number not valid";
        }
        free (version);

        //  Get sequence number for use in reply
        char *sequence = zmsg_popstr (request);

        //  Get domain, and discard it
        char *domain = zmsg_popstr (request);
        free (domain);

        //  Get IP address, and discard it
        char *address = zmsg_popstr (request);
        assert (streq (address, "192.168.55.1"));
        free (address);

        // Get originating socket identity
        char *identity = zmsg_popstr (request);
        if (streq (identity, "BOB")) {
            //  Get and validate mechanism
            char *mechanism = zmsg_popstr (request);
            if (streq (mechanism, "NULL")) {
                status_code = "200";
                status_text = "OK";
            }
            else
            if (streq (mechanism, "PLAIN")) {
                char *username = zmsg_popstr (request);
                char *password = zmsg_popstr (request);
                if (streq (username, "admin")
                &&  streq (password, "secret")) {
                    status_code = "200";
                    status_text = "OK";
                }
                else {
                    status_code = "400";
                    status_text = "Invalid username or password";
                }
                free (username);
                free (password);
            }
            else {
                status_code = "400";
                status_text = "Security mechanism not supported";
            }
            free (mechanism);
        }
        else {
            status_code = "400";
            status_text = "Identity is not known";
        }
        free (identity);
        zmsg_destroy (&request);

        zmsg_t *reply = zmsg_new ();
        zmsg_addstr (reply, "1.0");       //  ZAP version 1.0
        zmsg_addstr (reply, sequence);    //  Sequence number
        zmsg_addstr (reply, status_code);
        zmsg_addstr (reply, status_text);
        zmsg_addstr (reply, streq (status_code, "200")? "joe": "");
        zmsg_send (&reply, handler);
        free (sequence);
    }
    zctx_destroy (&ctx);
    return NULL;
}


//  Main thread starts the other threads
int main (void) {
    zctx_t *ctx = zctx_new ();

    //  Start external handler (terminal)
    zthread_new (external_handler, NULL);

    //  Start internal handler (proxy) and wait for ready message
    void *handler = zthread_fork (ctx, internal_handler, NULL);
    char *ready = zstr_recv (handler);
    free (ready);

    //  Start server and wait for finished message
    void *server = zthread_fork (ctx, server_task, NULL);
    char *finished = zstr_recv (server);
    free (finished);

    zctx_destroy (&ctx);
    return 0;
}
