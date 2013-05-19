//  Reference implementation for rfc.zeromq.org/spec:27/ZAP
//  This implementation demonstrates a server talking to a proxy handler 
//  over inproc, talking to an external terminal handler over TCP. The 
//  terminal handler implements a PLAIN authentication mechanism.

#include <czmq.h>

//  This is a null server task that executes several ZAP requests

static void 
server_task (void *args, zctx_t *ctx, void *pipe)
{
    //  We'll use a REQ socket for our request
    void *requestor = zsocket_new (ctx, ZMQ_REQ);
    int rc = zsocket_connect (requestor, "inproc://zeromq.zap.01");
    assert (rc != -1);

    byte credentials [] = { 5, 'a','d','m','i','n', 8, 'p','a','s','s','w','o','r','d' };

    //  Create a valid ZAP request and send it
    zmsg_t *request = zmsg_new ();
    zmsg_addstr (request, "1.0");       //  ZAP version 1.0
    zmsg_addstr (request, "1");         //  Sequence number
    zmsg_addstr (request, "test");      //  Domain
    zmsg_addstr (request, "PLAIN");     //  Mechanism
    zmsg_addmem (request, credentials, sizeof (credentials));
    zmsg_send (&request, requestor);
    
    //  Get reply and print it out
    zmsg_t *reply = zmsg_recv (requestor);
    zmsg_dump (reply);
    zmsg_destroy (&reply);
    
    //  Create an invalid ZAP request and send it
    request = zmsg_new ();
    zmsg_addstr (request, "1.0");       //  ZAP version 1.0
    zmsg_addstr (request, "2");         //  Sequence number
    zmsg_addstr (request, "test");      //  Domain
    zmsg_addstr (request, "BOGUS");     //  Mechanism
    zmsg_addmem (request, credentials, sizeof (credentials));
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
    zstr_send (pipe, "ready");
    
    //  The handler binds to a fixed endpoint; in practice you would
    //  use something more flexible for discovery
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    rc = zsocket_bind (backend, "tcp://*:9999");
    assert (rc != -1);

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
    
    char *status_code = "200";
    char *status_text = "OK";
    
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

        //  Get domain, but discard it
        char *domain = zmsg_popstr (request);
        free (domain);
        
        //  Get and validate mechanism
        char *mechanism = zmsg_popstr (request);
        if (strneq (mechanism, "PLAIN")) {
            status_code = "400";
            status_text = "Security mechanism not supported";
        }
        free (mechanism);
        
        //  We don't check the credentials in this example
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