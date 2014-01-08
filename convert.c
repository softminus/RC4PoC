/* this file handles conversion between raw bytes and base64 and raw bytes and
   hex encoding */

/* this file is copyright 2014 Matilda <heinousbutch@gmail.com> and licensed
   under the terms of the ISC license. */

#include "convert.h"
const char e_table [] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char hex_table [] = "0123456789abcdef";

/* base64 encodes up to three raw bytes
 *
 * *in          pointer to where we begin to encode
 * inlen        how many bytes do we have available to encode (needs to be 1, 2, or 3)
 * *out         pointer to where we output
 *
 * this writes up to four bytes to the location pointed to by out and does
 * not null-terminate its output.
 */

void encode_quantum(unsigned char *in, size_t inlen, char *out)
{
    
    assert ((inlen < 4) & (inlen > 0));
    
    /* ok, we have at least one byte in in[] that we need to encode */

    out[0] = e_table [in[0] >> 2]; /* ...so we can just cut off the lower two 
                                      bits of it w/o any conditionals */

    /* if we don't have any in[1] we need to zerofill the right bits of the
       expression */

    out[1] = e_table [ ((in[0] & 0x03) << 4) | (( (inlen > 1 ? in[1] : 0x00) & 0xf0) >> 4)];

    /* If we have no more in[]s to encode, we add output padding and return */
    if (inlen == 1)
    {
        out[2] = '=';
        out[3] = '=';
        return;
    }

    /* just like for out[1], we just select the bits we want, shift 'em in 
       the right position, OR the whole thing together. if there's no in[1] 
       available, we just replace it with 0x00 in that expression */

    out[2] = e_table [ ((in[1] & 0x0f) << 2) | (( (inlen > 2 ? in[2] : 0x00) & 0xc0) >> 6)];
    
    /* we have no more in[]s to encode, just pad the output and return */

    if(inlen == 2)
    {
        out[3] = '=';
        return;
    }

    /* we have one final byte to process, this one's easy */
    out[3] = e_table [ (in[2] & 0x3f) ]; 
}



/* base64 encode a block of memory 
 * 
 * *in      pointer to memory that we will encode
 * inbytes  how many bytes we will encode
 * *out     where we will output the encoded data
 *
 * this assumes that there is a buffer of memory long enough to store all
 * the encoded data at out. this *does* null-terminate the output.
 */


void encode_memory(unsigned char *in, size_t inbytes, char *out)
{
    size_t i = 0;

    while(i < inbytes)
    {
        encode_quantum(in, (inbytes - i < 3 ? inbytes - i : 3), out);
        in  += 3;
        i   += 3;
        out += 4;

    }
    out = 0;
}



int charval(char c)
{
    if(strchr(e_table,c) == NULL)
    {
        return -1;
    }
    return strchr(e_table,c) - e_table;
}

/* base64 decodes 
 *
 * *in          pointer to a string exactly 4 bytes long.
 * *out         pointer to where we output
 *
 * return value is how many bytes were written to *out
 * this writes up to 3 bytes to the location pointed to by out and does
 * not null-terminate its output.
 */

size_t decode_quantum(char *in, unsigned char *out)
{
    uint8_t buf0, buf1, buf2, buf3;
    uint32_t buf;
    int numout = 3; /* number of bytes we'll output */

    assert (charval(in[0]) != -1 || in[0] == '=');
    assert (charval(in[1]) != -1 || in[1] == '=');
    assert (charval(in[2]) != -1 || in[2] == '=');
    assert (charval(in[3]) != -1 || in[3] == '=');
    if(in[3] == '=')
    {
        numout--;
    }
    if(in[2] == '=')
    {
        numout--;
    }
    
    buf0 = (in[0] == '=') ? 0 : charval(in[0]) ;
    buf1 = (in[1] == '=') ? 0 : charval(in[1]) ;
    buf2 = (in[2] == '=') ? 0 : charval(in[2]) ;
    buf3 = (in[3] == '=') ? 0 : charval(in[3]) ;

    buf = (buf0 << (3 * 6)) + 
          (buf1 << (2 * 6)) +
          (buf2 << (1 * 6)) +
          (buf3 << (0 * 6));

    out[0] = (buf >> 2 * 8 ) & 0xFF;
    if (numout == 1)
    {
        return numout;
    }
    out[1] = (buf >> 1 * 8) & 0xFF;

    if (numout == 2)
    {
        return numout;
    }
    out[2] = (buf >> 0 * 8 ) & 0xFF;
    return numout;
}


/* base64 decode a string and put the output into a block of memory
 * in -- pointer to string we will decode
 * len -- length of that string
 * out -- pointer to where we will output the data
 *
 * return value -- how many bytes were written to *out
 */

size_t decode_string(char *in, size_t len, unsigned char *out)
{
    assert(len % 4 == 0);
    size_t written = 0, total = 0;
    int i = 0;
    while(i < len)
    {
        written = decode_quantum(in, out);
        in += 4;
        i += 4;
        out += written;
        total += written;
    }
    return written;
}

/* reads one byte, outputs two bytes */
void hex_a_byte(uint8_t *byte, uint8_t *output)
{
    output[0] = hex_table[(*byte & 0xf0) >> 4];
    output[1] = hex_table[(*byte & 0x0f)];
}

int charval_hex(char c)
{
    if(strchr(hex_table,c) == NULL)
    {
        return -1;
    }
    return strchr(hex_table,c) - hex_table;
}

/* reads 2 bytes, outputs 1 */
void dehex_2_bytes(uint8_t *text, uint8_t *byte)
{
    uint8_t result;
    assert(charval_hex(text[0]) != -1);
    assert(charval_hex(text[1]) != -1);
    *byte = (charval_hex(text[0]) << 4) + charval_hex(text[1]);
}

/* buffer at *output needs to be length inlen * 2 + 1. the +1 is for the NULL
 termination */
void hex_a_buffer(uint8_t *bytes, size_t inlen, uint8_t *output)
{
    int i;
    for(i = 0; i < inlen; i++)
    {
        hex_a_byte(bytes + i, output + 2 * i);
    }
    output[2 * inlen] = 0;
}

/* buffer at *output needs to be length inlen / 2 */
void dehex_a_buffer(uint8_t *input, size_t inlen, uint8_t *output)
{
    int i;
    assert(inlen % 2 == 0);
    for(i = 0; i < inlen / 2; i ++)
    {
        dehex_2_bytes(input + 2 * i, output + i);
    }

}


