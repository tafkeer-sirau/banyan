/* user interface for cc: casual encryption and decryption for files */

#ifndef __MAIN_H
#define __MAIN_H

/* modes */
#define ENCRYPT   0
#define DECRYPT   1
#define KEYCHANGE 2

/* fixed suffix for encrypted files (no longer user-configurable) */
#define SUF ".cpt"

/* structure to hold command-line */
typedef struct {
  const char *name;  /* invocation name: always "cc" */
  char *keyword;
  char *keyword2;    /* when changing keys: new key */
  int mode;          /* ENCRYPT, DECRYPT, KEYCHANGE */
  int filter;        /* running as a filter? */
  char **infiles;    /* list of filenames */
  int count;         /* number filenames */
} cmdline;

extern cmdline cmd;

#endif /* __MAIN_H */
