
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

/* Directory where symlinks to rehome live, named after the program to rehome.
 */
char *REHOME_BIN = "/.local/rehome/bin";

/* Directory in XDG_DATA_HOME used to hold the fake HOME directories for the
 * rehomed programs.
 */
char *REHOME_DIR = "/rehome/";

/* Default value of XDG_DATA_HOME if environment variable not set */
char *XDG_DATA_HOME = "/.local/share";

#define DEBUG 0

void error(int code, char *msg) {
  fprintf(stderr, msg);
  exit(code);
}

/* Remove targ from str path. Must not be last element of path. (It never
 * should be, for this application.)
 */
void path_remove(char *str, char *targ) {
  char *p = strstr(str, targ);
  if (p == NULL) {
    error(1, "didn't find rehome dir in path\n");
  }
  char *colon = strchr(p, ':');
  if (colon == NULL) {
    error(2, "rehome dir was last in path\n");
  }
  strcpy(p, colon+1);
}

/* Get XDG_DATA_HOME or default based on home */
char *xdg_data_home(char *home) {
  char *xdh = getenv("XDG_DATA_HOME");
  if (xdh != NULL) {
    return xdh;
  }
  xdh = malloc(strlen(home) + strlen(XDG_DATA_HOME) + 1);
  strcat(xdh, home);
  strcat(xdh, XDG_DATA_HOME);
  return xdh;
}

/* Filter a program name to just its alphanumeric characters.
 */
char *clean(char *progname) {
  char *clean = malloc(strlen(progname)+1);
  int j = 0;
  for (int i = 0; i < strlen(progname); i++) {
    char c = progname[i];
    if (isalnum(c)) {
      clean[j++] = c;
    }
  }
  clean[j] = '\0';
  return clean;
}

/* Make directory if doesn't exist. */
void mkifne(char *dirname) {
  DIR* dir = opendir(dirname);
  if (dir) {
    closedir(dir);
    return;
  }
  mkdir(dirname, 0755);
}

int main(int argc, char *argv[]) {
  /* Get target, PATH and HOME */
  char *target = clean(argv[0]);
  char *path = getenv("PATH");
  char *home = getenv("HOME");
  if (home == NULL) {
    error(3, "can't find HOME\n");
  }
  /* Remove rehome symlink dir from path */
  int bufsiz = strlen(REHOME_BIN) +  strlen(home);
  char *rehome = malloc(bufsiz+1);
  strcat(rehome, home);
  strcat(rehome, REHOME_BIN);
  path_remove(path, rehome);
  setenv("PATH", path, 1);
  /* Now compute a fake home directory */
  char *xdh = xdg_data_home(home);
  int fhl = strlen(xdh) + strlen(REHOME_DIR) + strlen(target) + 1;
  char *fakehome = malloc(fhl);
  strcat(fakehome, xdh);
  strcat(fakehome, REHOME_DIR);
  strcat(fakehome, target);
  setenv("HOME", fakehome, 1);
  mkifne(fakehome);
  /* Call the target with whatever arguments we were given */
  return execvp(target, argv);
}

