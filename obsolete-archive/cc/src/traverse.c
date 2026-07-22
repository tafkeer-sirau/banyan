/* traverse.c: functions for acting on a list of files, encrypting/
   decrypting/keychanging each one in place */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <utime.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "xalloc.h"
#include "main.h"
#include "traverse.h"
#include "ccrypt.h"
#include "platform.h"

#define IGNORE_RESULT(x) if ((int)(x)) {;}

/* ---------------------------------------------------------------------- */
/* an "object" for keeping track of a list of inodes that we have seen */

struct inode_dev_s {
  ino_t inode;  /* an inode */
  dev_t dev;    /* a device */
  int success;  /* was encryption/decryption successful for this inode? */
};
typedef struct inode_dev_s inode_dev_t;

/* inode_list: a list of inode/device pairs. inode_num is number of
   nodes in the list, and inode_size is its allocated size */

static inode_dev_t *inode_list = NULL;
static int inode_num = 0;
static int inode_size = 0;

/* error_flags: collect statistics on all error and warning messages
   that occur. */

static int key_errors = 0;
static int io_errors = 0;

/* add an inode/device pair to the list and record success or failure */
static void add_inode(ino_t ino, dev_t dev, int success) {
  if (inode_list==NULL) {
    inode_size = 100;
    inode_list = (inode_dev_t *)xalloc(inode_size*sizeof(inode_dev_t), cmd.name);
  }
  if (inode_num >= inode_size) {
    inode_size += 100;
    inode_list = (inode_dev_t *)xrealloc(inode_list, inode_size*sizeof(inode_dev_t), cmd.name);
  }
  inode_list[inode_num].inode = ino;
  inode_list[inode_num].dev = dev;
  inode_list[inode_num].success = success;
  inode_num++;
}

/* look up ino/dev pair in list. Return -1 if not found, else 0 if
   success=0, else 1 */
static int known_inode(ino_t ino, dev_t dev) {
  int i;

  /* have we already seen this inode/device pair? */
  for (i=0; i<inode_num; i++) {
    if (inode_list[i].inode == ino && inode_list[i].dev == dev) {
      return inode_list[i].success ? 1 : 0;
    }
  }
  return -1;
}

/* ---------------------------------------------------------------------- */
/* suffix handling (suffix is always the fixed SUF, ".cpt") */

/* return 1 if filename ends in, but is not equal to, suffix. */
static int has_suffix(const char *filename, const char *suffix) {
  int flen = strlen(filename);
  int slen = strlen(suffix);
  return flen>slen && strcmp(filename+flen-slen, suffix)==0;
}

/* add suffix to filename. Returns an allocated string, or NULL on
   error with errno set. */
static char *add_suffix(const char *filename, const char *suffix) {
  char *outfile;
  int flen = strlen(filename);
  int slen = strlen(suffix);

  outfile = (char *)malloc(flen+slen+1);
  if (!outfile) {
    return NULL;
  }
  strncpy (outfile, filename, flen);
  strncpy (outfile+flen, suffix, slen+1);
  return outfile;
}

/* remove suffix from filename. Returns an allocated string, or NULL
   on error with errno set. */
static char *remove_suffix(const char *filename, const char *suffix) {
  char *outfile;
  int flen = strlen(filename);
  int slen = strlen(suffix);

  if (!has_suffix(filename, suffix)) {
    return strdup(filename);
  }
  outfile = (char *)malloc(flen-slen+1);
  if (!outfile) {
    return NULL;
  }
  strncpy (outfile, filename, flen-slen);
  outfile[flen-slen] = 0;
  return outfile;
}

/* ---------------------------------------------------------------------- */
/* some helper functions */

/* read a yes/no response from the user */
static int prompt(void) {
  char *line;
  FILE *fin;
  int r;

  fin = fopen("/dev/tty", "r");
  if (fin==NULL) {
    fin = stdin;
  }

  line = xreadline(fin, cmd.name);
  r = line && (!strcmp(line, "y") || !strcmp(line, "yes"));
  free(line);
  return r;
}

/* check whether named file exists */
static int file_exists(char *filename) {
  struct stat buf;
  int st;

  st = lstat(filename, &buf);

  if (st) {
    return 0;
  } else {
    return 1;
  }
}

/* ---------------------------------------------------------------------- */
/* file actions for the individual modes. */

/* local signal handler - catch interrupt signal */
static int sigint_flag = 0;

static void sigint_overwrite(int dummy) {
  static time_t sigint_time = 0;
  int save_errno = errno;

  /* exit if two SIGINTS are received in one second */
  if ((time(NULL)-sigint_time) <= 1) {
    fprintf(stderr, "%s: interrupted.\n", cmd.name);
    exit(6);
  }

  /* otherwise, schedule to exit at the end of the current file. Note:
     this signal handler is only in use if we're not in filter mode */
  sigint_time = time(NULL);
  sigint_flag = 1;
  fprintf(stderr,
  "Interrupt - will exit after current file.\n"
    "Press CTRL-C twice to exit now (warning: this can lead to loss of data).\n");
  errno = save_errno;
}

/* this function is called to act on a file: encrypts/decrypts/
   keychanges it in place. */
static void action_overwrite(char *infile, char *outfile) {
  int st;
  struct stat buf;
  int do_chmod = 0;
  int r;
  int fd;
  int save_errno;
  int s;

  /* read file attributes */
  st = stat(infile, &buf);
  if (st) {
    fprintf(stderr, "%s: %s: %s\n", cmd.name, infile, strerror(errno));
    io_errors++;
    return;
  }

  /* check whether this file is write protected */
  if ((buf.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
    /* file is write-protected. In this case, we prompt the user to
       see if they want to operate on it anyway. */
    switch (cmd.mode) {
    case ENCRYPT:
      fprintf(stderr, "%s: encrypt write-protected file %s (y or n)? ", cmd.name, infile);
      break;
    case DECRYPT: default:
      fprintf(stderr, "%s: decrypt write-protected file %s (y or n)? ", cmd.name, infile);
      break;
    case KEYCHANGE:
      fprintf(stderr, "%s: perform keychange on write-protected file %s (y or n)? ", cmd.name, infile);
      break;
    }
    fflush(stderr);
    if (prompt()==0) {
      fprintf(stderr, "Not changed.\n");
      add_inode(buf.st_ino, buf.st_dev, 0);
      return;
    }
    /* we will attempt to change the mode just before encrypting it. */
    do_chmod = 1;
  }

  /* check whether this inode was already handled under another filename */
  r = known_inode(buf.st_ino, buf.st_dev);
  if (r == 0) {
    /* previous action on this inode failed - do nothing */
    return;
  } else if (r == 1) {
    /* previous action on this inode succeeded - rename only */
    goto rename;
  }

  /* act on this inode now */
  if (do_chmod) {
    chmod(infile, buf.st_mode | S_IWUSR);
  }

  /* open file */
  fd = open(infile, O_RDWR | O_BINARY);
  if (fd == -1) {
    /* could not open file. */
    fprintf(stderr, "%s: %s: %s\n", cmd.name, infile, strerror(errno));
    io_errors++;
    add_inode(buf.st_ino, buf.st_dev, 0);
    return;
  }

  /* set local signal handler for SIGINT */
  signal(SIGINT, sigint_overwrite);

  /* crypt */
  switch (cmd.mode) {   /* only ENCRYPT, DECRYPT, or KEYCHANGE */

  case ENCRYPT: default:
    r = ccencrypt_file(fd, cmd.keyword);
    break;

  case DECRYPT:
    r = ccdecrypt_file(fd, cmd.keyword);
    break;

  case KEYCHANGE:
    r = cckeychange_file(fd, cmd.keyword, cmd.keyword2);
    break;

  }
  save_errno = errno;

  /* restore the original file attributes for this file descriptor.
     Ignore failures silently */
  IGNORE_RESULT(fchown(fd, buf.st_uid, buf.st_gid));
  fchmod(fd, buf.st_mode);

  /* close file */
  s = close(fd);  /* i/o errors from previous writes may appear here */
  if (!r && s) {
    r = -3;
    save_errno = errno;
  }

  /* now restore original modtime */
  {
    struct utimbuf ut;
    ut.actime = buf.st_atime;
    ut.modtime = buf.st_mtime;

    utime(infile, &ut);
  }

  /* restore default signal handler */
  signal(SIGINT, SIG_DFL);

  errno = save_errno;
  if (r==-2 && (ccrypt_errno == CCRYPT_EFORMAT || ccrypt_errno == CCRYPT_EMISMATCH)) {
    fprintf(stderr, "%s: %s: %s -- unchanged\n", cmd.name, infile, ccrypt_error(r));
    key_errors++;
    add_inode(buf.st_ino, buf.st_dev, 0);
    return;
  } else if (r) {
    fprintf(stderr, "%s: %s: %s\n", cmd.name, infile, ccrypt_error(r));
    if (r == -3) { /* i/o error */
      exit(3);
    } else {
      exit(2);
    }
  } else {
    add_inode(buf.st_ino, buf.st_dev, 1);
  }

 rename:
  /* rename file if necessary */
  if (strcmp(infile, outfile)) {
    r = rename(infile, outfile);
    if (r) {
      fprintf(stderr, "%s: could not rename %s to %s: %s\n", cmd.name,
	      infile, outfile, strerror(errno));
      io_errors++;
    }
  }

  if (sigint_flag) {  /* SIGINT received while crypting - delayed exit */
    exit(6);
  }
  return;
}

/* ---------------------------------------------------------------------- */
/* file_action(): this procedure is called once for each file given on
   the command line. It is only called on files, not directories or
   symbolic links. The name of the input and output filename is
   determined here - this involves manipulating suffixes as needed.
   Furthermore, the decision of whether to overwrite an existing
   outfile is also made here. Note that file_action does not itself
   write or modify anything in the filesystem; that is delegated to
   action_overwrite. */

static void file_action(char *filename) {
  struct stat buf;
  int st;
  char *outfile = NULL;
  char *infile;
  char *buffer = NULL;

  infile = filename;  /* but it may be changed below */

  st = lstat(infile, &buf);

  if (st) {
    int save_errno = errno;

    /* if file didn't exist and decrypting, try if suffixed file exists */
    if (errno==ENOENT
	&& (cmd.mode==DECRYPT || cmd.mode==KEYCHANGE)) {
      buffer = (char *)xalloc(strlen(filename)+strlen(SUF)+1, cmd.name);

      strcpy(buffer, infile);
      strcat(buffer, SUF);
      infile=buffer;
      st = lstat(infile, &buf);
    }
    if (st) {
      fprintf(stderr, "%s: %s: %s\n", cmd.name, filename, strerror(save_errno));
      io_errors++;
      goto done;
    }
  }

  /* if file is a symbolic link or not a regular file, skip */
  if (S_ISLNK(buf.st_mode)) {
    fprintf(stderr, "%s: %s: is a symbolic link -- ignored\n", cmd.name, infile);
    goto done;
  }
  if (!S_ISREG(buf.st_mode)) {
    fprintf(stderr, "%s: %s: is not a regular file -- ignored\n", cmd.name,
	    infile);
    goto done;
  }

  /* determine outfile name */
  switch (cmd.mode) {
  case ENCRYPT: default:
    outfile = add_suffix(infile, SUF);
    break;
  case DECRYPT:
    outfile = remove_suffix(infile, SUF);
    break;
  case KEYCHANGE:
    outfile = strdup(infile);
    break;
  }
  if (!outfile) {
    fprintf(stderr, "%s: %s\n", cmd.name, strerror(errno));
    exit(2);
  }

  /* if outfile exists, prompt whether to overwrite */
  if (strcmp(infile, outfile) &&
      file_exists(outfile)) {
    fprintf(stderr, "%s: %s already exists; overwrite (y or n)? ", cmd.name,
	    outfile);
    fflush(stderr);
    if (prompt()==0) {
      fprintf(stderr, "Not overwritten.\n");
      goto done;
    }
  }
  action_overwrite(infile, outfile);
 done:
  free(outfile);
  free(buffer);
  return;
}

/* ---------------------------------------------------------------------- */
/* if filename is a directory or a symbolic link, issue a warning and
   do nothing; otherwise call file_action. Directories are never
   traversed. */

static void traverse_file(char *filename) {
  struct stat buf;
  int st;

  st = lstat(filename, &buf);
  if (!st && S_ISLNK(buf.st_mode)) {
    fprintf(stderr, "%s: %s: is a symbolic link -- ignored\n", cmd.name, filename);
    return;
  }
  if (!st && S_ISDIR(buf.st_mode)) {
    fprintf(stderr, "%s: %s: is a directory -- ignored\n", cmd.name, filename);
    return;
  }
  file_action(filename);
}

/* traverse a list of files. Return 1 if there were some non-matching
   keys and/or bad file formats, 2 if there were some (non-fatal) i/o
   errors, or 3 if there were both kinds of errors. */
int traverse_toplevel(char **filelist, int count) {

  /* reset inode list (redundant) */
  free(inode_list);
  inode_list = NULL;
  inode_num = 0;
  inode_size = 0;

  /* reset error stats */
  key_errors = 0;
  io_errors = 0;

  while (count > 0) {
    traverse_file(*filelist);
    ++filelist, --count;
  }

  free(inode_list);

  return (key_errors ? 1 : 0) | (io_errors ? 2 : 0);
}
