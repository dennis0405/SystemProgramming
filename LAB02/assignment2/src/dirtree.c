//--------------------------------------------------------------------------------------------------
// System Programming                         I/O Lab                                    Spring 2025
//
/// @file
/// @brief recursively traverse directory tree and list all entries
/// @author ...
/// @studid ...
//--------------------------------------------------------------------------------------------------

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <grp.h>
#include <pwd.h>

#define MAX_DIR 64            ///< maximum number of supported directories

/// @brief output control flags
#define F_TREE      0x1       ///< enable tree view
#define F_SUMMARY   0x2       ///< enable summary
#define F_VERBOSE   0x4       ///< turn on verbose mode

/// @brief struct holding the summary
struct summary {
  unsigned int dirs;          ///< number of directories encountered
  unsigned int files;         ///< number of files
  unsigned int links;         ///< number of links
  unsigned int fifos;         ///< number of pipes
  unsigned int socks;         ///< number of sockets

  unsigned long long size;    ///< total size (in bytes)
  unsigned long long blocks;  ///< total number of blocks (512 byte blocks)
};

struct fileInfo {
  char name[256];             /// name of the file
  char *path;                 /// full path of the file
  char user[9];               /// user name
  char group[9];              /// group name
  off_t size;                 /// size of the file 
  blkcnt_t blocks;            /// number of blocks (512 byte blocks)
  char type;                  /// type of the file 
};

/// @brief abort the program with EXIT_FAILURE and an optional error message
/// @param msg optional error message or NULL
void panic(const char *msg)
{
  if (msg) fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/// @brief read next directory entry from open directory 'dir'. Ignores '.' and '..'
/// @param dir open DIR* stream
/// @retval entry on success
/// @retval NULL on error or if there are no more entries
struct dirent *getNext(DIR *dir)
{
  struct dirent *next;
  int ignore;

  do {
    errno = 0;
    next = readdir(dir);
    if (errno != 0) perror(NULL);
    ignore = next && ((strcmp(next->d_name, ".") == 0) || (strcmp(next->d_name, "..") == 0));
  } while (next && ignore);

  return next;
}

/// @brief qsort comparator to sort directory entries. Directories come first.
/// @param a pointer to first entry pointer
/// @param b pointer to second entry pointer
static int dirent_compare(const void *a, const void *b)
{
  const struct dirent *e1 = *(const struct dirent **)a;
  const struct dirent *e2 = *(const struct dirent **)b;

  // if one of the entries is a directory, it comes first
  if (e1->d_type != e2->d_type) {
    if (e1->d_type == DT_DIR) return -1;
    if (e2->d_type == DT_DIR) return 1;
  }
  return strcmp(e1->d_name, e2->d_name);
}

/// @brief recursively process directory @a dn and print its tree
/// @param dn absolute or relative path string
/// @param pstr prefix string printed in front of each entry
/// @param stats pointer to statistics
/// @param flags output control flags (F_*)
void processDir(const char *dn, const char *pstr, struct summary *stats, unsigned int flags)
{
  DIR *dir = opendir(dn);
  if (!dir) {
    if (errno == ENOENT) {
      fprintf(stderr, "%s%s%s\n", pstr, (flags & F_TREE) ? "`-" : "  ", "ERROR: No such file or directory");
      return;
    } 
    else if (errno == EACCES) {
      fprintf(stderr, "%s%s%s\n", pstr, (flags & F_TREE) ? "`-" : "  ", "ERROR: Permission denied");
      return;
    } 
    else {
      perror("opendir failed");
      return;
    }
  }

  int capacity = 64;
  struct dirent **entries = malloc(capacity * sizeof(struct dirent *));
  if (entries == NULL) {
    panic("malloc failed");
  }

  int nentries = 0;
  struct dirent *entry;
  while ((entry = getNext(dir)) != NULL) {
    struct dirent *entry_copy = malloc(entry->d_reclen);
    if (entry_copy == NULL) {
        panic("malloc failed for entry_copy");
    }
    memcpy(entry_copy, entry, entry->d_reclen);
    
    if (nentries == capacity) {
      capacity *= 2;
      struct dirent **tmp = realloc(entries, capacity * sizeof(struct dirent *));
      if (tmp == NULL) {
        panic("realloc failed");
      }
      entries = tmp;
    }
  
    entries[nentries++] = entry_copy;
  }
  closedir(dir);

  qsort(entries, nentries, sizeof(struct dirent*), dirent_compare);

  for (int i = 0; i < nentries; i++) {
    int isLast = (i == nentries - 1);
    
    const char *branch = "  ";
    if (flags & F_TREE) {
      branch = isLast ? "`-" : "|-";
    }

    entry = entries[i];

    size_t pathlen = strlen(dn) + strlen(entry->d_name) + 2;
    char *filepath = calloc(pathlen, 1);
    if (!filepath) {
      panic("calloc failed for filepath");
    }
    snprintf(filepath, pathlen, "%s/%s", dn, entry->d_name);

    struct stat st;
    if (lstat(filepath, &st) < 0) {
      perror("lstat");
      free(filepath);
      free(entries[i]);
      continue;
    }

    struct fileInfo f;
    memset(&f, 0, sizeof(f));
    f.path = filepath;
    strncpy(f.name, entry->d_name, sizeof(f.name)-1);
    f.name[sizeof(f.name)-1] = '\0';
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    if (pw) {
      strncpy(f.user, pw->pw_name, sizeof(f.user)-1);
      f.user[sizeof(f.user)-1] = '\0';
    }
    if (gr) {
      strncpy(f.group, gr->gr_name, sizeof(f.group)-1);
      f.group[sizeof(f.group)-1] = '\0';
    }
    f.size = st.st_size;
    f.blocks = st.st_blocks;

    switch (entry->d_type) {
      case DT_DIR:  f.type = 'd'; break;
      case DT_LNK:  f.type = 'l'; break;
      case DT_REG:  f.type = ' '; break;
      case DT_FIFO: f.type = 'f'; break;
      case DT_SOCK: f.type = 's'; break;
      case DT_CHR:  f.type = 'c'; break;
      case DT_BLK:  f.type = 'b'; break;
      default:      f.type = '?'; break;
    }

    size_t path_and_name_len = strlen(pstr) + strlen(branch) + strlen(f.name) + 1;
    char *path_and_name = calloc(path_and_name_len, 1);
    if (!path_and_name){
      panic("calloc failed for path_and_name");
    }
    snprintf(path_and_name, path_and_name_len, "%s%s%s", pstr, branch, f.name);

    if (path_and_name_len > 55) {
      path_and_name[51] = '\0';
      snprintf(path_and_name + strlen(path_and_name), path_and_name_len - strlen(path_and_name), "%s", "...");
    }  

    if (flags & F_VERBOSE) {
      printf("%-54s  %8s:%-8s  %10ld  %8ld  %c\n", path_and_name, f.user, f.group, f.size, f.blocks, f.type);
    } 
    else {
      printf("%s\n", path_and_name);
    }
    free(path_and_name);

    if (flags & F_SUMMARY) {
      stats->size += f.size;
      stats->blocks += f.blocks;
      switch (f.type) {
        case 'd': stats->dirs++; break;
        case 'l': stats->links++; break;
        case 'f': stats->fifos++; break;
        case 's': stats->socks++; break;
        case ' ': stats->files++; break;
        default: break;
      }
    }

    if (f.type == 'd') {
      size_t new_prefix_len = strlen(pstr) + 3;
      char *new_pstr = calloc(new_prefix_len, 1);
      if (!new_pstr) {
        panic("calloc failed for new_pstr");
      }

      if (flags & F_TREE) {
        isLast ? snprintf(new_pstr, new_prefix_len, "%s  ", pstr) : snprintf(new_pstr, new_prefix_len, "%s| ", pstr);
      } 
      else {
        snprintf(new_pstr, new_prefix_len, "%s  ", pstr);
      }

      processDir(f.path, new_pstr, stats, flags);
      free(new_pstr);
    }

    free(filepath);
  }

  for (int i = 0; i < nentries; i++) {
    free(entries[i]);
  }
  free(entries);
}

/// @brief print program syntax and an optional error message. Aborts with EXIT_FAILURE
/// @param argv0 command line argument 0 (executable)
/// @param error optional error (format) string or NULL
/// @param ... parameters for the error format string
void syntax(const char *argv0, const char *error, ...)
{
  if (error) {
    va_list ap;
    va_start(ap, error);
    vfprintf(stderr, error, ap);
    va_end(ap);
    printf("\n\n");
  }

  assert(argv0 != NULL);

  fprintf(stderr, "Usage %s [-t] [-s] [-v] [-h] [path...]\n"
                  "Gather information about directory trees. If no path is given, the current directory\n"
                  "is analyzed.\n"
                  "\n"
                  "Options:\n"
                  " -t        print the directory tree (default if no other option specified)\n"
                  " -s        print summary of directories (total number of files, total file size, etc)\n"
                  " -v        print detailed information for each file. Turns on tree view.\n"
                  " -h        print this help\n"
                  " path...   list of space-separated paths (max %d). Default is the current directory.\n",
                  basename((char *)argv0), MAX_DIR);

  exit(EXIT_FAILURE);
}

/// @brief program entry point
int main(int argc, char *argv[])
{
  const char CURDIR[] = ".";
  const char *directories[MAX_DIR];
  int ndir = 0;
  struct summary tstat;
  unsigned int flags = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (!strcmp(argv[i], "-t"))
        flags |= F_TREE;
      else if (!strcmp(argv[i], "-s"))
        flags |= F_SUMMARY;
      else if (!strcmp(argv[i], "-v"))
        flags |= F_VERBOSE;
      else if (!strcmp(argv[i], "-h"))
        syntax(argv[0], NULL);
      else
        syntax(argv[0], "Unrecognized option '%s'.", argv[i]);
    } else {
      if (ndir < MAX_DIR) {
        directories[ndir++] = argv[i];
      } else {
        printf("Warning: maximum number of directories exceeded, ignoring '%s'.\n", argv[i]);
      }
    }
  }

  if (ndir == 0)
    directories[ndir++] = CURDIR;

  setbuf(stdout, NULL);
  memset(&tstat, 0, sizeof(tstat));

  for (int i = 0; i < ndir; i++) {
    struct summary dstat;
    memset(&dstat, 0, sizeof(dstat));
    const char *dir = directories[i];

    if (flags & F_SUMMARY) {
      if (flags & F_VERBOSE)
        printf("%-60s%-21s%-8s%-7s%4s\n", "Name", "User:Group", "Size", "Blocks", "Type"); 
      else 
        printf("%-4s\n", "Name");
      
      for (int j = 0; j < 100; j++) 
        putchar('-');
      putchar('\n');
    }

    printf("%s\n", dir);
    
    processDir(dir, "", &dstat, flags);

    if (flags & F_SUMMARY) {
      for (int j = 0; j < 100; j++)
        putchar('-');
      putchar('\n');

      char summary[256];
      snprintf(summary, sizeof(summary), "%d %s, %d %s, %d %s, %d %s, and %d %s",
        dstat.files,  (dstat.files  == 1 ? "file"       : "files"),
        dstat.dirs,   (dstat.dirs   == 1 ? "directory"  : "directories"),
        dstat.links,  (dstat.links  == 1 ? "link"       : "links"),
        dstat.fifos,  (dstat.fifos  == 1 ? "pipe"       : "pipes"),
        dstat.socks,  (dstat.socks  == 1 ? "socket"     : "sockets")
      );
      
      if (strlen(summary) > 68) {
        summary[65] = '\0';
        snprintf(summary + strlen(summary), sizeof(summary) - strlen(summary), "%s", "...");
      }
      
      if (flags & F_VERBOSE)
        printf("%-68s   %14llu %9llu\n\n", summary, dstat.size, dstat.blocks); 
      else
        printf("%-68s\n\n", summary);
    }

    tstat.dirs   += dstat.dirs;
    tstat.files  += dstat.files;
    tstat.links  += dstat.links;
    tstat.fifos  += dstat.fifos;
    tstat.socks  += dstat.socks;
    tstat.size   += dstat.size;
    tstat.blocks += dstat.blocks;
  }

  if ((flags & F_SUMMARY) && (ndir > 1)) {
    printf("Analyzed %d directories:\n"
           "  total # of files:        %16d\n"
           "  total # of directories:  %16d\n"
           "  total # of links:        %16d\n"
           "  total # of pipes:        %16d\n"
           "  total # of sockets:      %16d\n",
           ndir, tstat.files, tstat.dirs, tstat.links, tstat.fifos, tstat.socks);

    if (flags & F_VERBOSE) {
      printf("  total file size:         %16llu\n"
             "  total # of blocks:       %16llu\n",
             tstat.size, tstat.blocks);
    }
  }

  return EXIT_SUCCESS;
}
