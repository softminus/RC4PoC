/* breaks RC4 given hella ciphertexts of the same plaintext */

#include <assert.h>
#include <openssl/rc4.h>
#include <string.h>
#include <openssl/rand.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include "convert.h"

struct state {
    uint64_t bigN[256][256]; 
    /* the 1st index selects the *byte index* within the plaintext 
       we desire. the 2nd index is the index is the index described in 
       the paper (the byte value);

       basically, we compute the statistics for *all plaintext bytes* 
       at once */
       
};

struct probs {
    uint64_t p[256][256];
    /* the 1st index is the position within the keystream.
       the 2nd index is the byte value */
    uint64_t total; /* total number of samples */
};



/* reads a file containing probabilities formatted like the one in 
   RC4_keystream_dist_2_45.txt .
   the format is:
   [position] [byte-value] [count] 
   where [count] is the number of occurrences of [byte-value] at [position] 
   in the RC4 keystream.
*/
void read_probs(FILE *fp, struct probs *probs)
{
    assert (fp != NULL);
    char *line = NULL, *saveptr, *token;
    size_t len = 0;
    ssize_t result;

    int line_n = 0, i, j;

    while ((result = getline(&line, &len, fp)) != -1)
    {
        if (line[0] == '\n')
        {
            line_n++;
            continue;
        }
        if (line[0] == '%')
        {
            line_n++;
            continue;
        }
        
        token = strtok_r(line, " ", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr, "no field # 1 at line %d", line_n);
            exit(-1);
        }
        
        i = atoi(token);

        token = strtok_r(NULL, " ", &saveptr);

        if(token == NULL)
        {
            fprintf(stderr, "no field # 2 at line %d", line_n);
            exit(-1);
        }
        j = atoi(token);

        token = strtok_r(NULL, " ", &saveptr);

        if(token == NULL)
        {
            fprintf(stderr, "no field # 3 at line %d", line_n);
            exit(-1);
        }
        probs->p[i][j] = atoll(token);

        line_n++;
    }

    probs->total = 0;

    for(i = 0; i <= 255; i++)
    {
        probs->total += probs->p[0][i];
    }


}





void ciphertext_freqstat(uint8_t *ciphertext, size_t len, struct state *st)
{
    int i;
    assert (len < 256);
    for(i = 0; i < len; i++)
    {
        st->bigN[i][ciphertext[i]]++;
    }
}


uint8_t guess_byte(uint8_t which, struct state *st, struct probs *probs)
{
    uint8_t mu, k, maxmu;
    uint64_t bigNprime[256][256];
    double lambda[256], maxlambda;

    for (mu = 0; mu <= 255; mu++)
    {
        for (k = 0; k <= 255; k++)
        {
            bigNprime[mu][k] = st->bigN[which][mu ^ k];
        }

    lambda [mu] = 0.0;
    
    for (k = 0; k <= 255; k++)
    {
        lambda[mu] += (double) bigNprime[mu][k] * \
            log( (double) probs->p[which][k] / (double) probs->total );
    }

    } /* closes off the for (mu = 0; mu <= 255; mu++) loop */

    maxlambda = -INFINITY;
    maxmu = 0;
    for (mu = 0; mu <= 255; mu ++)
    {
        if(lambda[mu] > maxlambda)
        {
            maxmu = mu;
            maxlambda = lambda[mu];
        }
    }

    return maxmu;
}

int main(int argc, char **argv)
{
    FILE *fp;
    struct probs probs;
    fp = fopen("RC4_keystream_dist_2_45.txt","r");
    if (fp == NULL)
    {
        perror("error opening RC4_keystream_dist_2_45.txt");
        exit(-1);
    }

    read_probs(fp, &probs);
    printf("%d\n",probs.p[0][0]);
    return 0;
}
