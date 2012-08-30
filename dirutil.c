
#include "dirutil.h"

#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <malloc.h>

typedef struct stat Stat;

static int do_mkdir(const char *path, mode_t mode) {
  Stat st;
  int status = 0;

  if(stat(path, &st) != 0) {
    if(mkdir(path, mode) != 0 && errno != EEXIST) {
      status = -1;
    }
  } else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    status = -1;
  }
  return status;
}

int mkpath(const char *path, mode_t mode) {
  char *sp;
  int status = 0;
  char *copypath = strdup(path);
  char *pp = copypath;

  while(status == 0 && (sp = strchr(pp, '/')) != 0) {
    if(sp != pp) {
      /* Check root/ double slash */
      *sp = '\0';
      status = do_mkdir(copypath, mode);
      *sp = '/';
    }
    pp = sp+1;
  }
  if (status == 0) {
    status = do_mkdir(path, mode);
  }

  free(copypath);
  return status;
}
