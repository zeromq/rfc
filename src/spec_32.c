//  Base85 encode / decode reference implementation
    //  Goals
    //  Fixed length output
    //  4-octet based input
    //  25% expansion for encoding
    //  Printable and quotable
    //  Remove similar punctuation

#include <czmq.h>

//  ---------------------------------------------------------------------
//  Structure of a base85 class

typedef struct _base85_t base85_t;

struct _base85_t {
    char encoder [85 + 1];      //  Encoding table bin->asc
    byte decoder [256];         //  Decoding table asc->bin
};


//  --------------------------------------------------------------------------
//  Create a new base85 instance

base85_t *
base85_new (void)
{
    base85_t *self = (base85_t *) zmalloc (sizeof (base85_t));
    assert (self);
    strcpy (self->encoder, 
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        ".-:+=^!/*?&<>()[]{}@%$#");

    assert (strlen (self->encoder) == 85);
    int index;
    for (index = 0; index < 85; index++)
        self->decoder [(byte) self->encoder [index]] = index;
    
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the base85 instance

void
base85_destroy (base85_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        base85_t *self = *self_p;
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Encode a byte array as a string

char *
base85_encode (base85_t *self, byte *data, size_t size)
{
    //  Accepts only byte arrays bounded to 4 bytes
    assert (size % 4 == 0);
    
    size_t encoded_size = size * 5 / 4;
    char *encoded = malloc (encoded_size + 1);

    uint char_nbr = 0;
    uint byte_nbr;
    for (byte_nbr = 0; byte_nbr < size; byte_nbr += 4) {
        uint32_t value = (data [byte_nbr    ] << 24) 
                       + (data [byte_nbr + 1] << 16) 
                       + (data [byte_nbr + 2] << 8) 
                       + (data [byte_nbr + 3]);
        int step_nbr;
        for (step_nbr = 0; step_nbr < 5; step_nbr++) {
            encoded [char_nbr++] = self->encoder [value % 85];
            value /= 85;
        }
    }
    assert (char_nbr == encoded_size);
    encoded [char_nbr] = 0;
    return encoded;
}

    
//  --------------------------------------------------------------------------
//  Decode an encoded string into a byte array; size of array will be
//  strlen (string) * 4 / 5.

byte *
base85_decode (base85_t *self, char *string)
{
    //  Accepts only strings bounded to 5 bytes
    assert (strlen (string) % 5 == 0);
    
    size_t decoded_size = strlen (string) * 4 / 5;
    byte *decoded = malloc (decoded_size);

    uint byte_nbr = 0;
    uint char_nbr;
    for (char_nbr = 0; char_nbr < strlen (string); char_nbr += 5) {
        uint32_t value = 0;
        int step_nbr;
        for (step_nbr = 5; step_nbr > 0; step_nbr--)
            value = value * 85 + self->decoder [(byte) string [char_nbr + step_nbr - 1]];
        decoded [byte_nbr++] = (value >> 24) & 0xFF;
        decoded [byte_nbr++] = (value >> 16) & 0xFF;
        decoded [byte_nbr++] = (value >>  8) & 0xFF;
        decoded [byte_nbr++] =  value        & 0xFF;
    }
    assert (byte_nbr == decoded_size);
    return decoded;
}

int main (void)
{
    byte key [32] = {
      0x8E, 0x0B, 0xDD, 0x69, 0x76, 0x28, 0xB9, 0x1D, 
      0x8F, 0x24, 0x55, 0x87, 0xEE, 0x95, 0xC5, 0xB0, 
      0x4D, 0x48, 0x96, 0x3F, 0x79, 0x25, 0x98, 0x77, 
      0xB4, 0x9C, 0xD9, 0x06, 0x3A, 0xEA, 0xD3, 0xB7  
    };
    base85_t *base85 = base85_new ();
    char *encoded = base85_encode (base85, key, 32);
    byte *decoded = base85_decode (base85, encoded);
    assert (streq (encoded, "SVKTJw)%%BX.E0K}+>V)mNp?o4&O{CN!b4W6hL{i"));
    assert (memcmp (key, decoded, 32) == 0);
    free (decoded);
    free (encoded);
    base85_destroy (&base85);

    exit (0);
}
