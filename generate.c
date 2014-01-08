/* this file encrypts the same plaintext with RC4 with a bunch of 
   different and random keys */

/* copyright (c) 2014 Kia <. licensed under 
 the terms of the ISC license. */
#include <assert.h>
#include <openssl/rc4.h>
#include <string.h>
#include <openssl/rand.h>
#include <stdint.h>
#include <unistd.h>

#include "convert.h"

int main()
{
    unsigned char plaintext [] = "hello world!!!!!!!!!!!!";
    uint8_t key [16];
    RC4_KEY actual_key;
    uint8_t *encrypted, *hexed_encrypted;
    int result;
    uint64_t i, iters;
    iters = 67108864; /* 2^26 */
    /* iters = 4294967296; */
    
    size_t len, hexlen;

    len = strlen(plaintext);
    hexlen = len * 2;
    fprintf(stderr, "length of the ciphertext is %d\n",len);

    assert (len < 255);

    encrypted = malloc(len);
    assert (encrypted != NULL);

/*    hexed_encrypted = malloc(hexlen + 1); 
    assert (hexed_encrypted != NULL);
*/
    for (i = 0; i < iters; i++)
    {
        result = RAND_bytes(key, 16);
        assert(result == 1);

        RC4_set_key(&actual_key, 16, key);

        RC4(&actual_key, len, plaintext, encrypted);
        write(1, encrypted, len); 
/*        hex_a_buffer(encrypted, len, hexed_encrypted);

        write(1, hexed_encrypted, hexlen);
        write(1, "\n", 1);
  */

    }

    return 0; 

}

