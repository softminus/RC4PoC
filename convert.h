/* this file handles conversion between raw bytes and base64 and raw bytes and
   hex encoding */

/* this file is copyright 2014 Matilda <heinousbutch@gmail.com> and licensed
   under the terms of the ISC license. */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>


#define ENCODED_SIZE(foo) (4 * ((foo + 2) / 3))
#define DECODED_SIZE(foo) ((foo * 3) / 4)


void encode_memory(unsigned char *in, size_t inbytes, char *out);
size_t decode_string(char *in, size_t len, unsigned char *out);
void hex_a_buffer(uint8_t *bytes, size_t inlen, uint8_t *output);
void dehex_a_buffer(uint8_t *input, size_t inlen, uint8_t *output);


