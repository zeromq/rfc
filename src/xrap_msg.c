/*  =========================================================================
    xrap_msg - XRAP serialization over ZMTP

    Codec class for xrap_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: spec_40.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    =========================================================================
*/

/*
@header
    xrap_msg - XRAP serialization over ZMTP
@discuss
@end
*/

#include "./xrap_msg.h"

//  Structure of our class

struct _xrap_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  xrap_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    // Schema/type/name
    char parent [256];
    // Content type
    char content_type [256];
    // New resource specification
    char *content_body;
    // Response status code 2xx
    uint16_t status_code;
    // Schema/type/name
    char location [256];
    // Opaque hash tag
    char etag [256];
    // Date and time modified
    uint64_t date_modified;
    // Schema/type/name
    char resource [256];
    // GET if more recent
    uint64_t if_modified_since;
    // GET if changed
    char if_none_match [256];
    // Update if same date
    uint64_t if_unmodified_since;
    // Update if same ETag
    char if_match [256];
    // Response status text
    char status_text [256];
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) { \
        zsys_warning ("xrap_msg: GET_OCTETS failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) { \
        zsys_warning ("xrap_msg: GET_NUMBER1 failed"); \
        goto malformed; \
    } \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) { \
        zsys_warning ("xrap_msg: GET_NUMBER2 failed"); \
        goto malformed; \
    } \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) { \
        zsys_warning ("xrap_msg: GET_NUMBER4 failed"); \
        goto malformed; \
    } \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) { \
        zsys_warning ("xrap_msg: GET_NUMBER8 failed"); \
        goto malformed; \
    } \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("xrap_msg: GET_STRING failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("xrap_msg: GET_LONGSTR failed"); \
        goto malformed; \
    } \
    free ((host)); \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new xrap_msg

xrap_msg_t *
xrap_msg_new (void)
{
    xrap_msg_t *self = (xrap_msg_t *) zmalloc (sizeof (xrap_msg_t));
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the xrap_msg

void
xrap_msg_destroy (xrap_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        xrap_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->content_body);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Receive a xrap_msg from the socket. Returns 0 if OK, -1 if
//  there was an error. Blocks if there is no message waiting.

int
xrap_msg_recv (xrap_msg_t *self, zsock_t *input)
{
    assert (input);

    if (zsock_type (input) == ZMQ_ROUTER) {
        zframe_destroy (&self->routing_id);
        self->routing_id = zframe_recv (input);
        if (!self->routing_id || !zsock_rcvmore (input)) {
            zsys_warning ("xrap_msg: no routing ID");
            return -1;          //  Interrupted or malformed
        }
    }
    zmq_msg_t frame;
    zmq_msg_init (&frame);
    int size = zmq_msg_recv (&frame, zsock_resolve (input), 0);
    if (size == -1) {
        zsys_warning ("xrap_msg: interrupted");
        goto malformed;         //  Interrupted
    }
    //  Get and check protocol signature
    self->needle = (byte *) zmq_msg_data (&frame);
    self->ceiling = self->needle + zmq_msg_size (&frame);

    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 5)) {
        zsys_warning ("xrap_msg: invalid signature");
        //  TODO: discard invalid messages and loop, and return
        //  -1 only on interrupt
        goto malformed;         //  Interrupted
    }
    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case XRAP_MSG_POST:
            GET_STRING (self->parent);
            GET_STRING (self->content_type);
            GET_LONGSTR (self->content_body);
            break;

        case XRAP_MSG_POST_OK:
            GET_NUMBER2 (self->status_code);
            GET_STRING (self->location);
            GET_STRING (self->etag);
            GET_NUMBER8 (self->date_modified);
            GET_STRING (self->content_type);
            GET_LONGSTR (self->content_body);
            break;

        case XRAP_MSG_GET:
            GET_STRING (self->resource);
            GET_NUMBER8 (self->if_modified_since);
            GET_STRING (self->if_none_match);
            GET_STRING (self->content_type);
            break;

        case XRAP_MSG_GET_OK:
            GET_NUMBER2 (self->status_code);
            GET_STRING (self->content_type);
            GET_LONGSTR (self->content_body);
            break;

        case XRAP_MSG_GET_EMPTY:
            GET_NUMBER2 (self->status_code);
            break;

        case XRAP_MSG_PUT:
            GET_STRING (self->resource);
            GET_NUMBER8 (self->if_unmodified_since);
            GET_STRING (self->if_match);
            GET_STRING (self->content_type);
            GET_LONGSTR (self->content_body);
            break;

        case XRAP_MSG_PUT_OK:
            GET_NUMBER2 (self->status_code);
            GET_STRING (self->location);
            GET_STRING (self->etag);
            GET_NUMBER8 (self->date_modified);
            break;

        case XRAP_MSG_DELETE:
            GET_STRING (self->resource);
            GET_NUMBER8 (self->if_unmodified_since);
            GET_STRING (self->if_match);
            break;

        case XRAP_MSG_DELETE_OK:
            GET_NUMBER2 (self->status_code);
            break;

        case XRAP_MSG_ERROR:
            GET_NUMBER2 (self->status_code);
            GET_STRING (self->status_text);
            break;

        default:
            zsys_warning ("xrap_msg: bad message ID");
            goto malformed;
    }
    //  Successful return
    zmq_msg_close (&frame);
    return 0;

    //  Error returns
    malformed:
        zsys_warning ("xrap_msg: xrap_msg malformed message, fail");
        zmq_msg_close (&frame);
        return -1;              //  Invalid message
}


//  --------------------------------------------------------------------------
//  Send the xrap_msg to the socket. Does not destroy it. Returns 0 if
//  OK, else -1.

int
xrap_msg_send (xrap_msg_t *self, zsock_t *output)
{
    assert (self);
    assert (output);

    if (zsock_type (output) == ZMQ_ROUTER)
        zframe_send (&self->routing_id, output, ZFRAME_MORE + ZFRAME_REUSE);

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case XRAP_MSG_POST:
            frame_size += 1 + strlen (self->parent);
            frame_size += 1 + strlen (self->content_type);
            frame_size += 4;
            if (self->content_body)
                frame_size += strlen (self->content_body);
            break;
        case XRAP_MSG_POST_OK:
            frame_size += 2;            //  status_code
            frame_size += 1 + strlen (self->location);
            frame_size += 1 + strlen (self->etag);
            frame_size += 8;            //  date_modified
            frame_size += 1 + strlen (self->content_type);
            frame_size += 4;
            if (self->content_body)
                frame_size += strlen (self->content_body);
            break;
        case XRAP_MSG_GET:
            frame_size += 1 + strlen (self->resource);
            frame_size += 8;            //  if_modified_since
            frame_size += 1 + strlen (self->if_none_match);
            frame_size += 1 + strlen (self->content_type);
            break;
        case XRAP_MSG_GET_OK:
            frame_size += 2;            //  status_code
            frame_size += 1 + strlen (self->content_type);
            frame_size += 4;
            if (self->content_body)
                frame_size += strlen (self->content_body);
            break;
        case XRAP_MSG_GET_EMPTY:
            frame_size += 2;            //  status_code
            break;
        case XRAP_MSG_PUT:
            frame_size += 1 + strlen (self->resource);
            frame_size += 8;            //  if_unmodified_since
            frame_size += 1 + strlen (self->if_match);
            frame_size += 1 + strlen (self->content_type);
            frame_size += 4;
            if (self->content_body)
                frame_size += strlen (self->content_body);
            break;
        case XRAP_MSG_PUT_OK:
            frame_size += 2;            //  status_code
            frame_size += 1 + strlen (self->location);
            frame_size += 1 + strlen (self->etag);
            frame_size += 8;            //  date_modified
            break;
        case XRAP_MSG_DELETE:
            frame_size += 1 + strlen (self->resource);
            frame_size += 8;            //  if_unmodified_since
            frame_size += 1 + strlen (self->if_match);
            break;
        case XRAP_MSG_DELETE_OK:
            frame_size += 2;            //  status_code
            break;
        case XRAP_MSG_ERROR:
            frame_size += 2;            //  status_code
            frame_size += 1 + strlen (self->status_text);
            break;
    }
    //  Now serialize message into the frame
    zmq_msg_t frame;
    zmq_msg_init_size (&frame, frame_size);
    self->needle = (byte *) zmq_msg_data (&frame);
    PUT_NUMBER2 (0xAAA0 | 5);
    PUT_NUMBER1 (self->id);
    size_t nbr_frames = 1;              //  Total number of frames to send

    switch (self->id) {
        case XRAP_MSG_POST:
            PUT_STRING (self->parent);
            PUT_STRING (self->content_type);
            if (self->content_body) {
                PUT_LONGSTR (self->content_body);
            }
            else
                PUT_NUMBER4 (0);    //  Empty string
            break;

        case XRAP_MSG_POST_OK:
            PUT_NUMBER2 (self->status_code);
            PUT_STRING (self->location);
            PUT_STRING (self->etag);
            PUT_NUMBER8 (self->date_modified);
            PUT_STRING (self->content_type);
            if (self->content_body) {
                PUT_LONGSTR (self->content_body);
            }
            else
                PUT_NUMBER4 (0);    //  Empty string
            break;

        case XRAP_MSG_GET:
            PUT_STRING (self->resource);
            PUT_NUMBER8 (self->if_modified_since);
            PUT_STRING (self->if_none_match);
            PUT_STRING (self->content_type);
            break;

        case XRAP_MSG_GET_OK:
            PUT_NUMBER2 (self->status_code);
            PUT_STRING (self->content_type);
            if (self->content_body) {
                PUT_LONGSTR (self->content_body);
            }
            else
                PUT_NUMBER4 (0);    //  Empty string
            break;

        case XRAP_MSG_GET_EMPTY:
            PUT_NUMBER2 (self->status_code);
            break;

        case XRAP_MSG_PUT:
            PUT_STRING (self->resource);
            PUT_NUMBER8 (self->if_unmodified_since);
            PUT_STRING (self->if_match);
            PUT_STRING (self->content_type);
            if (self->content_body) {
                PUT_LONGSTR (self->content_body);
            }
            else
                PUT_NUMBER4 (0);    //  Empty string
            break;

        case XRAP_MSG_PUT_OK:
            PUT_NUMBER2 (self->status_code);
            PUT_STRING (self->location);
            PUT_STRING (self->etag);
            PUT_NUMBER8 (self->date_modified);
            break;

        case XRAP_MSG_DELETE:
            PUT_STRING (self->resource);
            PUT_NUMBER8 (self->if_unmodified_since);
            PUT_STRING (self->if_match);
            break;

        case XRAP_MSG_DELETE_OK:
            PUT_NUMBER2 (self->status_code);
            break;

        case XRAP_MSG_ERROR:
            PUT_NUMBER2 (self->status_code);
            PUT_STRING (self->status_text);
            break;

    }
    //  Now send the data frame
    zmq_msg_send (&frame, zsock_resolve (output), --nbr_frames? ZMQ_SNDMORE: 0);

    return 0;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
xrap_msg_print (xrap_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case XRAP_MSG_POST:
            zsys_debug ("XRAP_MSG_POST:");
            if (self->parent)
                zsys_debug ("    parent='%s'", self->parent);
            else
                zsys_debug ("    parent=");
            if (self->content_type)
                zsys_debug ("    content_type='%s'", self->content_type);
            else
                zsys_debug ("    content_type=");
            if (self->content_body)
                zsys_debug ("    content_body='%s'", self->content_body);
            else
                zsys_debug ("    content_body=");
            break;

        case XRAP_MSG_POST_OK:
            zsys_debug ("XRAP_MSG_POST_OK:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            if (self->location)
                zsys_debug ("    location='%s'", self->location);
            else
                zsys_debug ("    location=");
            if (self->etag)
                zsys_debug ("    etag='%s'", self->etag);
            else
                zsys_debug ("    etag=");
            zsys_debug ("    date_modified=%ld", (long) self->date_modified);
            if (self->content_type)
                zsys_debug ("    content_type='%s'", self->content_type);
            else
                zsys_debug ("    content_type=");
            if (self->content_body)
                zsys_debug ("    content_body='%s'", self->content_body);
            else
                zsys_debug ("    content_body=");
            break;

        case XRAP_MSG_GET:
            zsys_debug ("XRAP_MSG_GET:");
            if (self->resource)
                zsys_debug ("    resource='%s'", self->resource);
            else
                zsys_debug ("    resource=");
            zsys_debug ("    if_modified_since=%ld", (long) self->if_modified_since);
            if (self->if_none_match)
                zsys_debug ("    if_none_match='%s'", self->if_none_match);
            else
                zsys_debug ("    if_none_match=");
            if (self->content_type)
                zsys_debug ("    content_type='%s'", self->content_type);
            else
                zsys_debug ("    content_type=");
            break;

        case XRAP_MSG_GET_OK:
            zsys_debug ("XRAP_MSG_GET_OK:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            if (self->content_type)
                zsys_debug ("    content_type='%s'", self->content_type);
            else
                zsys_debug ("    content_type=");
            if (self->content_body)
                zsys_debug ("    content_body='%s'", self->content_body);
            else
                zsys_debug ("    content_body=");
            break;

        case XRAP_MSG_GET_EMPTY:
            zsys_debug ("XRAP_MSG_GET_EMPTY:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            break;

        case XRAP_MSG_PUT:
            zsys_debug ("XRAP_MSG_PUT:");
            if (self->resource)
                zsys_debug ("    resource='%s'", self->resource);
            else
                zsys_debug ("    resource=");
            zsys_debug ("    if_unmodified_since=%ld", (long) self->if_unmodified_since);
            if (self->if_match)
                zsys_debug ("    if_match='%s'", self->if_match);
            else
                zsys_debug ("    if_match=");
            if (self->content_type)
                zsys_debug ("    content_type='%s'", self->content_type);
            else
                zsys_debug ("    content_type=");
            if (self->content_body)
                zsys_debug ("    content_body='%s'", self->content_body);
            else
                zsys_debug ("    content_body=");
            break;

        case XRAP_MSG_PUT_OK:
            zsys_debug ("XRAP_MSG_PUT_OK:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            if (self->location)
                zsys_debug ("    location='%s'", self->location);
            else
                zsys_debug ("    location=");
            if (self->etag)
                zsys_debug ("    etag='%s'", self->etag);
            else
                zsys_debug ("    etag=");
            zsys_debug ("    date_modified=%ld", (long) self->date_modified);
            break;

        case XRAP_MSG_DELETE:
            zsys_debug ("XRAP_MSG_DELETE:");
            if (self->resource)
                zsys_debug ("    resource='%s'", self->resource);
            else
                zsys_debug ("    resource=");
            zsys_debug ("    if_unmodified_since=%ld", (long) self->if_unmodified_since);
            if (self->if_match)
                zsys_debug ("    if_match='%s'", self->if_match);
            else
                zsys_debug ("    if_match=");
            break;

        case XRAP_MSG_DELETE_OK:
            zsys_debug ("XRAP_MSG_DELETE_OK:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            break;

        case XRAP_MSG_ERROR:
            zsys_debug ("XRAP_MSG_ERROR:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            if (self->status_text)
                zsys_debug ("    status_text='%s'", self->status_text);
            else
                zsys_debug ("    status_text=");
            break;

    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
xrap_msg_routing_id (xrap_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
xrap_msg_set_routing_id (xrap_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the xrap_msg id

int
xrap_msg_id (xrap_msg_t *self)
{
    assert (self);
    return self->id;
}

void
xrap_msg_set_id (xrap_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
xrap_msg_command (xrap_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case XRAP_MSG_POST:
            return ("POST");
            break;
        case XRAP_MSG_POST_OK:
            return ("POST_OK");
            break;
        case XRAP_MSG_GET:
            return ("GET");
            break;
        case XRAP_MSG_GET_OK:
            return ("GET_OK");
            break;
        case XRAP_MSG_GET_EMPTY:
            return ("GET_EMPTY");
            break;
        case XRAP_MSG_PUT:
            return ("PUT");
            break;
        case XRAP_MSG_PUT_OK:
            return ("PUT_OK");
            break;
        case XRAP_MSG_DELETE:
            return ("DELETE");
            break;
        case XRAP_MSG_DELETE_OK:
            return ("DELETE_OK");
            break;
        case XRAP_MSG_ERROR:
            return ("ERROR");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the parent field

const char *
xrap_msg_parent (xrap_msg_t *self)
{
    assert (self);
    return self->parent;
}

void
xrap_msg_set_parent (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->parent)
        return;
    strncpy (self->parent, value, 255);
    self->parent [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the content_type field

const char *
xrap_msg_content_type (xrap_msg_t *self)
{
    assert (self);
    return self->content_type;
}

void
xrap_msg_set_content_type (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->content_type)
        return;
    strncpy (self->content_type, value, 255);
    self->content_type [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the content_body field

const char *
xrap_msg_content_body (xrap_msg_t *self)
{
    assert (self);
    return self->content_body;
}

void
xrap_msg_set_content_body (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    free (self->content_body);
    self->content_body = strdup (value);
}


//  --------------------------------------------------------------------------
//  Get/set the status_code field

uint16_t
xrap_msg_status_code (xrap_msg_t *self)
{
    assert (self);
    return self->status_code;
}

void
xrap_msg_set_status_code (xrap_msg_t *self, uint16_t status_code)
{
    assert (self);
    self->status_code = status_code;
}


//  --------------------------------------------------------------------------
//  Get/set the location field

const char *
xrap_msg_location (xrap_msg_t *self)
{
    assert (self);
    return self->location;
}

void
xrap_msg_set_location (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->location)
        return;
    strncpy (self->location, value, 255);
    self->location [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the etag field

const char *
xrap_msg_etag (xrap_msg_t *self)
{
    assert (self);
    return self->etag;
}

void
xrap_msg_set_etag (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->etag)
        return;
    strncpy (self->etag, value, 255);
    self->etag [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the date_modified field

uint64_t
xrap_msg_date_modified (xrap_msg_t *self)
{
    assert (self);
    return self->date_modified;
}

void
xrap_msg_set_date_modified (xrap_msg_t *self, uint64_t date_modified)
{
    assert (self);
    self->date_modified = date_modified;
}


//  --------------------------------------------------------------------------
//  Get/set the resource field

const char *
xrap_msg_resource (xrap_msg_t *self)
{
    assert (self);
    return self->resource;
}

void
xrap_msg_set_resource (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->resource)
        return;
    strncpy (self->resource, value, 255);
    self->resource [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the if_modified_since field

uint64_t
xrap_msg_if_modified_since (xrap_msg_t *self)
{
    assert (self);
    return self->if_modified_since;
}

void
xrap_msg_set_if_modified_since (xrap_msg_t *self, uint64_t if_modified_since)
{
    assert (self);
    self->if_modified_since = if_modified_since;
}


//  --------------------------------------------------------------------------
//  Get/set the if_none_match field

const char *
xrap_msg_if_none_match (xrap_msg_t *self)
{
    assert (self);
    return self->if_none_match;
}

void
xrap_msg_set_if_none_match (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->if_none_match)
        return;
    strncpy (self->if_none_match, value, 255);
    self->if_none_match [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the if_unmodified_since field

uint64_t
xrap_msg_if_unmodified_since (xrap_msg_t *self)
{
    assert (self);
    return self->if_unmodified_since;
}

void
xrap_msg_set_if_unmodified_since (xrap_msg_t *self, uint64_t if_unmodified_since)
{
    assert (self);
    self->if_unmodified_since = if_unmodified_since;
}


//  --------------------------------------------------------------------------
//  Get/set the if_match field

const char *
xrap_msg_if_match (xrap_msg_t *self)
{
    assert (self);
    return self->if_match;
}

void
xrap_msg_set_if_match (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->if_match)
        return;
    strncpy (self->if_match, value, 255);
    self->if_match [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the status_text field

const char *
xrap_msg_status_text (xrap_msg_t *self)
{
    assert (self);
    return self->status_text;
}

void
xrap_msg_set_status_text (xrap_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->status_text)
        return;
    strncpy (self->status_text, value, 255);
    self->status_text [255] = 0;
}



//  --------------------------------------------------------------------------
//  Selftest

int
xrap_msg_test (bool verbose)
{
    printf (" * xrap_msg:");

    //  Silence an "unused" warning by "using" the verbose variable
    if (verbose) {;}

    //  @selftest
    //  Simple create/destroy test
    xrap_msg_t *self = xrap_msg_new ();
    assert (self);
    xrap_msg_destroy (&self);

    //  Create pair of sockets we can send through
    //  We must bind before connect if we wish to remain compatible with ZeroMQ < v4
    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    int rc = zsock_bind (output, "inproc://selftest-xrap_msg");
    assert (rc == 0);

    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    rc = zsock_connect (input, "inproc://selftest-xrap_msg");
    assert (rc == 0);

    //  Encode/send/decode and verify each message type
    int instance;
    self = xrap_msg_new ();
    xrap_msg_set_id (self, XRAP_MSG_POST);

    xrap_msg_set_parent (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_type (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_body (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (streq (xrap_msg_parent (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_type (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_body (self), "Life is short but Now lasts for ever"));
    }
    xrap_msg_set_id (self, XRAP_MSG_POST_OK);

    xrap_msg_set_status_code (self, 123);
    xrap_msg_set_location (self, "Life is short but Now lasts for ever");
    xrap_msg_set_etag (self, "Life is short but Now lasts for ever");
    xrap_msg_set_date_modified (self, 123);
    xrap_msg_set_content_type (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_body (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (xrap_msg_status_code (self) == 123);
        assert (streq (xrap_msg_location (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_etag (self), "Life is short but Now lasts for ever"));
        assert (xrap_msg_date_modified (self) == 123);
        assert (streq (xrap_msg_content_type (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_body (self), "Life is short but Now lasts for ever"));
    }
    xrap_msg_set_id (self, XRAP_MSG_GET);

    xrap_msg_set_resource (self, "Life is short but Now lasts for ever");
    xrap_msg_set_if_modified_since (self, 123);
    xrap_msg_set_if_none_match (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_type (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (streq (xrap_msg_resource (self), "Life is short but Now lasts for ever"));
        assert (xrap_msg_if_modified_since (self) == 123);
        assert (streq (xrap_msg_if_none_match (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_type (self), "Life is short but Now lasts for ever"));
    }
    xrap_msg_set_id (self, XRAP_MSG_GET_OK);

    xrap_msg_set_status_code (self, 123);
    xrap_msg_set_content_type (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_body (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (xrap_msg_status_code (self) == 123);
        assert (streq (xrap_msg_content_type (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_body (self), "Life is short but Now lasts for ever"));
    }
    xrap_msg_set_id (self, XRAP_MSG_GET_EMPTY);

    xrap_msg_set_status_code (self, 123);
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (xrap_msg_status_code (self) == 123);
    }
    xrap_msg_set_id (self, XRAP_MSG_PUT);

    xrap_msg_set_resource (self, "Life is short but Now lasts for ever");
    xrap_msg_set_if_unmodified_since (self, 123);
    xrap_msg_set_if_match (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_type (self, "Life is short but Now lasts for ever");
    xrap_msg_set_content_body (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (streq (xrap_msg_resource (self), "Life is short but Now lasts for ever"));
        assert (xrap_msg_if_unmodified_since (self) == 123);
        assert (streq (xrap_msg_if_match (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_type (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_content_body (self), "Life is short but Now lasts for ever"));
    }
    xrap_msg_set_id (self, XRAP_MSG_PUT_OK);

    xrap_msg_set_status_code (self, 123);
    xrap_msg_set_location (self, "Life is short but Now lasts for ever");
    xrap_msg_set_etag (self, "Life is short but Now lasts for ever");
    xrap_msg_set_date_modified (self, 123);
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (xrap_msg_status_code (self) == 123);
        assert (streq (xrap_msg_location (self), "Life is short but Now lasts for ever"));
        assert (streq (xrap_msg_etag (self), "Life is short but Now lasts for ever"));
        assert (xrap_msg_date_modified (self) == 123);
    }
    xrap_msg_set_id (self, XRAP_MSG_DELETE);

    xrap_msg_set_resource (self, "Life is short but Now lasts for ever");
    xrap_msg_set_if_unmodified_since (self, 123);
    xrap_msg_set_if_match (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (streq (xrap_msg_resource (self), "Life is short but Now lasts for ever"));
        assert (xrap_msg_if_unmodified_since (self) == 123);
        assert (streq (xrap_msg_if_match (self), "Life is short but Now lasts for ever"));
    }
    xrap_msg_set_id (self, XRAP_MSG_DELETE_OK);

    xrap_msg_set_status_code (self, 123);
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (xrap_msg_status_code (self) == 123);
    }
    xrap_msg_set_id (self, XRAP_MSG_ERROR);

    xrap_msg_set_status_code (self, 123);
    xrap_msg_set_status_text (self, "Life is short but Now lasts for ever");
    //  Send twice
    xrap_msg_send (self, output);
    xrap_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        xrap_msg_recv (self, input);
        assert (xrap_msg_routing_id (self));
        assert (xrap_msg_status_code (self) == 123);
        assert (streq (xrap_msg_status_text (self), "Life is short but Now lasts for ever"));
    }

    xrap_msg_destroy (&self);
    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
