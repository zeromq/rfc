/*  =========================================================================
    xrap_msg - XRAP serialization over ZMTP

    Codec header for xrap_msg.

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

#ifndef XRAP_MSG_H_INCLUDED
#define XRAP_MSG_H_INCLUDED

/*  These are the xrap_msg messages:

    POST - Create a new, dynamically named resource in some parent.
        parent              string      Schema/type/name
        content_type        string      Content type
        content_body        longstr     New resource specification

    POST_OK - Success response for POST.
        status_code         number 2    Response status code 2xx
        location            string      Schema/type/name
        etag                string      Opaque hash tag
        date_modified       number 8    Date and time modified
        content_type        string      Content type
        content_body        longstr     Resource contents

    GET - Retrieve a known resource.
        resource            string      Schema/type/name
        if_modified_since   number 8    GET if more recent
        if_none_match       string      GET if changed
        content_type        string      Desired content type

    GET_OK - Success response for GET.
        status_code         number 2    Response status code 2xx
        content_type        string      Actual content type
        content_body        longstr     Resource specification

    GET_EMPTY - Conditional GET returned 304 Not Modified.
        status_code         number 2    Response status code 3xx

    PUT - Update a known resource.
        resource            string      Schema/type/name
        if_unmodified_since  number 8   Update if same date
        if_match            string      Update if same ETag
        content_type        string      Content type
        content_body        longstr     New resource specification

    PUT_OK - Success response for PUT.
        status_code         number 2    Response status code 2xx
        location            string      Schema/type/name
        etag                string      Opaque hash tag
        date_modified       number 8    Date and time modified

    DELETE - Remove a known resource.
        resource            string      schema/type/name
        if_unmodified_since  number 8   DELETE if same date
        if_match            string      DELETE if same ETag

    DELETE_OK - Success response for DELETE.
        status_code         number 2    Response status code 2xx

    ERROR - Error response for any request.
        status_code         number 2    Response status code, 4xx or 5xx
        status_text         string      Response status text
*/


#define XRAP_MSG_POST                       1
#define XRAP_MSG_POST_OK                    2
#define XRAP_MSG_GET                        3
#define XRAP_MSG_GET_OK                     4
#define XRAP_MSG_GET_EMPTY                  5
#define XRAP_MSG_PUT                        6
#define XRAP_MSG_PUT_OK                     7
#define XRAP_MSG_DELETE                     8
#define XRAP_MSG_DELETE_OK                  9
#define XRAP_MSG_ERROR                      10

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef XRAP_MSG_T_DEFINED
typedef struct _xrap_msg_t xrap_msg_t;
#define XRAP_MSG_T_DEFINED
#endif

//  @interface
//  Create a new empty xrap_msg
xrap_msg_t *
    xrap_msg_new (void);

//  Destroy a xrap_msg instance
void
    xrap_msg_destroy (xrap_msg_t **self_p);

//  Receive a xrap_msg from the socket. Returns 0 if OK, -1 if
//  there was an error. Blocks if there is no message waiting.
int
    xrap_msg_recv (xrap_msg_t *self, zsock_t *input);

//  Send the xrap_msg to the output socket, does not destroy it
int
    xrap_msg_send (xrap_msg_t *self, zsock_t *output);

//  Print contents of message to stdout
void
    xrap_msg_print (xrap_msg_t *self);

//  Get/set the message routing id
zframe_t *
    xrap_msg_routing_id (xrap_msg_t *self);
void
    xrap_msg_set_routing_id (xrap_msg_t *self, zframe_t *routing_id);

//  Get the xrap_msg id and printable command
int
    xrap_msg_id (xrap_msg_t *self);
void
    xrap_msg_set_id (xrap_msg_t *self, int id);
const char *
    xrap_msg_command (xrap_msg_t *self);

//  Get/set the parent field
const char *
    xrap_msg_parent (xrap_msg_t *self);
void
    xrap_msg_set_parent (xrap_msg_t *self, const char *value);

//  Get/set the content_type field
const char *
    xrap_msg_content_type (xrap_msg_t *self);
void
    xrap_msg_set_content_type (xrap_msg_t *self, const char *value);

//  Get/set the content_body field
const char *
    xrap_msg_content_body (xrap_msg_t *self);
void
    xrap_msg_set_content_body (xrap_msg_t *self, const char *value);

//  Get/set the status_code field
uint16_t
    xrap_msg_status_code (xrap_msg_t *self);
void
    xrap_msg_set_status_code (xrap_msg_t *self, uint16_t status_code);

//  Get/set the location field
const char *
    xrap_msg_location (xrap_msg_t *self);
void
    xrap_msg_set_location (xrap_msg_t *self, const char *value);

//  Get/set the etag field
const char *
    xrap_msg_etag (xrap_msg_t *self);
void
    xrap_msg_set_etag (xrap_msg_t *self, const char *value);

//  Get/set the date_modified field
uint64_t
    xrap_msg_date_modified (xrap_msg_t *self);
void
    xrap_msg_set_date_modified (xrap_msg_t *self, uint64_t date_modified);

//  Get/set the resource field
const char *
    xrap_msg_resource (xrap_msg_t *self);
void
    xrap_msg_set_resource (xrap_msg_t *self, const char *value);

//  Get/set the if_modified_since field
uint64_t
    xrap_msg_if_modified_since (xrap_msg_t *self);
void
    xrap_msg_set_if_modified_since (xrap_msg_t *self, uint64_t if_modified_since);

//  Get/set the if_none_match field
const char *
    xrap_msg_if_none_match (xrap_msg_t *self);
void
    xrap_msg_set_if_none_match (xrap_msg_t *self, const char *value);

//  Get/set the if_unmodified_since field
uint64_t
    xrap_msg_if_unmodified_since (xrap_msg_t *self);
void
    xrap_msg_set_if_unmodified_since (xrap_msg_t *self, uint64_t if_unmodified_since);

//  Get/set the if_match field
const char *
    xrap_msg_if_match (xrap_msg_t *self);
void
    xrap_msg_set_if_match (xrap_msg_t *self, const char *value);

//  Get/set the status_text field
const char *
    xrap_msg_status_text (xrap_msg_t *self);
void
    xrap_msg_set_status_text (xrap_msg_t *self, const char *value);

//  Self test of this class
int
    xrap_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define xrap_msg_dump       xrap_msg_print

#ifdef __cplusplus
}
#endif

#endif
