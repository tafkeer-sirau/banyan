/* user interface for cc: encrypt and decrypt files and streams */

/* cc is a tool for encrypting and decrypting files and streams. It can
   operate as a filter, or it can operate directly on files in the
   manner of gzip; it can overwrite files in-place on media that
   support read/write access. Encryption is based on the Rijndael
   block cipher, a version of which is also used in the Advanced
   Encryption Standard. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"
#include "readkey.h"
#include "ccrypt.h"
#include "traverse.h"
#include "xalloc.h"
#include "platform.h"

#define VERSION "1.11"

cmdline cmd;

/* print usage information, including version and license summary */

static void usage(FILE *fout) {
  fprintf(fout, "cc %s. Secure encryption and decryption of files and streams.\n", VERSION);
  fprintf(fout, "License: GNU General Public License.\n");
  fprintf(fout, "\n");
  fprintf(fout, "Usage: cc [mode] [file...]\n\n");
  fprintf(fout,
"Modes:\n"
"    -e            encrypt\n"
"    -d            decrypt\n"
"    -x            change key\n"
"\n"
"Options:\n"
"    -h            print this help message (including version) and exit\n"
"    --            end of options, filenames follow\n");
}

/* ---------------------------------------------------------------------- */
/* read the command line */

static const char *shortopts = "edxh-";

static cmdline read_commandline(int ac, char *av[]) {
  cmdline cmd;
  int c;

  /* defaults: */
  cmd.keyword = NULL;
  cmd.keyword2 = NULL;
  cmd.mode = ENCRYPT;
  cmd.filter = 1;
  cmd.infiles = NULL;
  cmd.count = 0;

  /* invocation name is always "cc" */
  cmd.name = "cc";

  while ((c = getopt(ac, av, shortopts)) != -1) {
    switch (c) {
    case 'h':
      usage(stdout);
      exit(0);
      break;
    case 'e':
      cmd.mode = ENCRYPT;
      break;
    case 'd':
      cmd.mode = DECRYPT;
      break;
    case 'x':
      cmd.mode = KEYCHANGE;
      break;
    case '?':
      fprintf(stderr, "Try -h for more information.\n");
      exit(1);
      break;
    default:
      fprintf(stderr, "%s: unimplemented option -- %c\n", cmd.name, c);
      exit(1);
    }
  }

  cmd.infiles = &av[optind];
  cmd.count = ac-optind;

  /* figure out if there are some filenames. Even an empty list of
     filenames is considered "some" filenames if "--" was used */

  if (cmd.count > 0 || (optind > 1 && strcmp(av[optind-1], "--") == 0)) {
    cmd.filter = 0;
  }

  /* if not in filter mode, and 0 filenames follow, don't bother continuing */
  if (!cmd.filter && cmd.count==0) {
    fprintf(stderr, "%s: warning: empty list of filenames given\n", cmd.name);
    exit(0);
  }

  /* check that we are not reading or writing encrypted data from/to a
     terminal */
  if (cmd.filter) {
    if ((cmd.mode==ENCRYPT || cmd.mode==KEYCHANGE)
	&& isatty(fileno(stdout))) {
      fprintf(stderr, "%s: encrypted data not written to a terminal.\n"
	      "Try -h for more information.\n", cmd.name);
      exit(1);
    }
    if ((cmd.mode==DECRYPT || cmd.mode==KEYCHANGE)
	&& isatty(fileno(stdin))) {
      fprintf(stderr, "%s: encrypted data not read from a terminal.\n"
	      "Try -h for more information.\n", cmd.name);
      exit(1);
    }
  }

  return cmd;
}

/* ---------------------------------------------------------------------- */

int main(int ac, char *av[]) {
  int r;

  /* read command line */
  cmd = read_commandline(ac, av);

  /* read keyword from terminal */
  if (cmd.keyword==NULL) {
    const char *prompt;

    switch (cmd.mode) {
    case ENCRYPT: default:
      prompt = "Enter encryption key: ";
      break;
    case DECRYPT:
      prompt = "Enter decryption key: ";
      break;
    case KEYCHANGE:
      prompt = "Enter old key: ";
      break;
    }
    cmd.keyword = readkey(prompt, "", cmd.name, 0);
    if (cmd.keyword==NULL) {  /* end of file: exit gracefully */
      fprintf(stderr, "%s: no key given\n", cmd.name);
      exit(9);
    }
    /* always prompt for the encryption key a second time, to catch typos */
    if (cmd.mode==ENCRYPT) {
      char *repeat;

      repeat = readkey(prompt, "(repeat) ", cmd.name, 0);
      if (repeat==NULL || strcmp(repeat, cmd.keyword)!=0) {
	fprintf(stderr, "Sorry, the keys you entered did not match.\n");
	exit(7);
      }
    }
  }

  /* read keyword2 from terminal if necessary */
  if (cmd.mode==KEYCHANGE && cmd.keyword2==NULL) {
    const char *prompt2 = "Enter new key: ";
    char *repeat;

    cmd.keyword2 = readkey(prompt2, "", cmd.name, 0);
    if (cmd.keyword2==NULL) {  /* end of file: exit gracefully */
      fprintf(stderr, "%s: no key given\n", cmd.name);
      exit(9);
    }
    /* always prompt a second time, to catch typos */
    repeat = readkey(prompt2, "(repeat) ", cmd.name, 0);
    if (repeat==NULL || strcmp(repeat, cmd.keyword2)!=0) {
      fprintf(stderr, "Sorry, the keys you entered did not match.\n");
      exit(7);
    }
  }

  /* reset stdin/stdout to binary mode under Windows */
  setmode(0,O_BINARY);
  setmode(1,O_BINARY);

  /* filter mode */

  if (cmd.filter) {
    switch (cmd.mode) {

    case ENCRYPT: default:
      r = ccencrypt_streams(stdin, stdout, cmd.keyword);
      break;

    case DECRYPT:
      r = ccdecrypt_streams(stdin, stdout, cmd.keyword);
      break;

    case KEYCHANGE:
      r = cckeychange_streams(stdin, stdout, cmd.keyword, cmd.keyword2);
      break;
    }

    free(cmd.keyword);
    free(cmd.keyword2);

    if (r) {
      fprintf(stderr, "%s: %s\n", cmd.name, ccrypt_error(r));
      if (r==-2 && (ccrypt_errno==CCRYPT_EFORMAT || ccrypt_errno==CCRYPT_EMISMATCH)) {
	return 4;
      } else if (r == -3) {
	return 3;
      } else {
	return 2;
      }
    }
    r = fflush(stdout);
    if (r == EOF) {
      fprintf(stderr, "%s: %s\n", cmd.name, strerror(errno));
      return 3;
    }
    return 0;
  }

  /* non-filter mode: traverse files */
  r = traverse_toplevel(cmd.infiles, cmd.count);

  free(cmd.keyword);
  free(cmd.keyword2);

  if (r==1) {
    return 4;
  } else if (r) {
    return 8;
  } else {
    return 0;
  }
}
