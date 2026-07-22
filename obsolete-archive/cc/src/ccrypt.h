/* ccrypt.c: high-level functions for accessing ccryptlib */

/* cc implements a stream cipher based on the block cipher
   Rijndael, the candidate for the AES standard. */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "ccryptlib.h"

const char *ccrypt_error(int st);

int ccencrypt_streams(FILE *fin, FILE *fout, const char *key);
int ccdecrypt_streams(FILE *fin, FILE *fout, const char *key);
int cckeychange_streams(FILE *fin, FILE *fout, const char *key1, const char *key2);

int ccencrypt_file(int fd, const char *key);
int ccdecrypt_file(int fd, const char *key);
int cckeychange_file(int fd, const char *key1, const char *key2);
