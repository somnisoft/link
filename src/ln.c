/**
 * @file
 * @brief ln utility
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TEST
/**
 * Declare some functions with extern linkage, allowing the test suite to call
 * those functions.
 */
# define LINKAGE extern
# include "../test/seams.h"
#else /* !(TEST) */
/**
 * Define all functions as static when not testing.
 */
# define LINKAGE static
#endif /* TEST */

/**
 * @defgroup ln_flag ln flags
 *
 * Option flags when running ln.
 */

/**
 * Remove the destination path if it already exists.
 *
 * Corresponds to argument (-f).
 *
 * @ingroup ln_flag
 */
#define LN_FLAG_REMOVE_DEST ((unsigned int)(1 << 0))

/**
 * Specifies whether ln creates a hard link to the symbolic link or to the
 * file pointed to by the symbolic link.
 *
 * Corresponds to argument (-L) if set.
 *
 * Corresponds to argument (-P) if unset.
 *
 * @ingroup ln_flag
 */
#define LN_FLAG_FOLLOW_SYMBOLIC ((unsigned int)(1 << 1))

/**
 * Create symbolic links (ln creates hard links by default).
 *
 * Corresponds to argument (-s).
 *
 * @ingroup ln_flag
 */
#define LN_FLAG_SYMBOLIC ((unsigned int)(1 << 2))

/**
 * ln utility context.
 */
struct ln_ctx{
  /**
   * Exit status set to one of the following values.
   *   - EXIT_SUCCESS
   *   - EXIT_FAILURE
   */
  int status_code;

  /**
   * See @ref ln_flag.
   */
  unsigned int flags;
};

/**
 * Print an error message to STDERR and set an error status code.
 *
 * @param[in,out] ln_ctx    See @ref ln_ctx.
 * @param[in]     errno_msg Include a standard message describing errno.
 * @param[in]     fmt       Format string used by vwarn.
 */
static void
ln_warn(struct ln_ctx *const ln_ctx,
        const bool errno_msg,
        const char *const fmt, ...){
  va_list ap;

  ln_ctx->status_code = EXIT_FAILURE;
  va_start(ap, fmt);
  if(errno_msg){
    vwarn(fmt, ap);
  }
  else{
    vwarnx(fmt, ap);
  }
  va_end(ap);
}

/**
 * Add two size_t values and check for wrap.
 *
 * @param[in]  a      Add to @p b.
 * @param[in]  b      Add to @p a.
 * @param[out] result Store the result of the addition here.
 * @retval     true   Addition without wrap.
 * @retval     false  Value wrapped.
 */
LINKAGE bool
si_add_size_t(const size_t a,
              const size_t b,
              size_t *const result){
  int wrap;

#ifdef TEST
  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_si_add_size_t)){
    return false;
  }
#endif /* TEST */

  *result = a + b;
  if(*result < a){
    wrap = true;
  }
  else{
    wrap = false;
  }
  return !(wrap);
}

/**
 * Get the concatenation of the target_dir and source_file.
 *
 * [target_dir]/basename(source_file)
 *
 * @param[in] target_dir  First part of new path.
 * @param[in] source_file Append the basename of this to @p target_dir.
 * @retval    char*       New target path. Caller must free this memory after
 *                        use.
 * @retval    NULL        Failed to allocate memory for new path.
 */
static char *
ln_path_target_concat(const char *const target_dir,
                      const char *const source_file){
  char *source_file_copy;
  char *bname_source_file;
  size_t slen_target_dir;
  size_t slen_bname_source_file;
  size_t concat_len;
  char *path_concat;
  char *path_cpy;

  path_concat = NULL;
  source_file_copy = strdup(source_file);
  if(source_file_copy){
    bname_source_file = basename(source_file_copy);
    slen_target_dir = strlen(target_dir);
    slen_bname_source_file = strlen(bname_source_file);
    /*
     * concat_len = strlen(target_dir) + '/' + strlen(bname_source_file) + \0
     * concat_len = slen_target_dir    +  1  + slen_bname_source_file    + 1
     */
    if(si_add_size_t(slen_target_dir, slen_bname_source_file, &concat_len) &&
       si_add_size_t(concat_len, 2, &concat_len)){
      path_concat = malloc(concat_len);
      if(path_concat){
        path_cpy = stpcpy(path_concat, target_dir);
        /* Append a slash if target_dir does not already end with a slash. */
        if(target_dir[slen_target_dir - 1] != '/'){
          path_cpy = stpcpy(path_cpy, "/");
        }
        stpcpy(path_cpy, bname_source_file);
      }
    }
    free(source_file_copy);
  }
  return path_concat;
}

/**
 * Check if two files have the same directory entry.
 *
 * Files considered identical if st_dev and st_ino match.
 *
 * @param[in] sb1   Compare with @p sb2.
 * @param[in] sb2   Compare with @p sb1.
 * @retval    true  @p sb1 and @p sb2 identical.
 * @retval    false @p sb1 and @p sb2 different.
 */
static bool
ln_same_file(const struct stat *const sb1,
             const struct stat *const sb2){
  bool same;

  if(sb1->st_dev == sb2->st_dev &&
     sb1->st_ino == sb2->st_ino){
    same = true;
  }
  else{
    same = false;
  }
  return same;
}

/**
 * Try to remove the destination file if (-f) argument set.
 *
 * @param[in,out] ln_ctx    See @ref ln_ctx.
 * @param[in]     source_sb Source file info.
 * @param[in]     path_dest Destination file to remove.
 * @retval        true      Successfully removed destination file or file
 *                          does not exit.
 * @retval        false     Error occurred while removing destination file.
 */
static bool
ln_remove_dest(struct ln_ctx *const ln_ctx,
               const struct stat *const source_sb,
               const char *const path_dest){
  struct stat dest_sb;
  bool removed;

  removed = true;
  if(stat(path_dest, &dest_sb) == 0){
    if(ln_ctx->flags & LN_FLAG_REMOVE_DEST){
      if(ln_same_file(source_sb, &dest_sb)){
        ln_warn(ln_ctx, false, "source and destination same: %s", path_dest);
        removed = false;
      }
      else{
        if(unlink(path_dest) != 0){
          ln_warn(ln_ctx, true, "failed to unlink destination: %s", path_dest);
          removed = false;
        }
      }
    }
    else{
      ln_warn(ln_ctx, false, "destination already exists: %s", path_dest);
      removed = false;
    }
  }
  return removed;
}

/**
 * Create the requested link file type.
 *
 * The following link functions will get called.
 *   - link    - If source is not a symbolic link.
 *   - linkat  - If source is a symbolic link.
 *   - symlink - Corresponds to (-s) argument.
 *
 * @param[in,out] ln_ctx      See @ref ln_ctx.
 * @param[in]     path_source Path to point the new link to.
 * @param[in]     path_dest   New link to create, pointing to @p path_source.
 */
static void
ln_create_link(struct ln_ctx *const ln_ctx,
               const char *const path_source,
               const char *const path_dest){
  int rc;
  int linkat_flag;
  struct stat source_sb;

  if(lstat(path_source, &source_sb) != 0){
    ln_warn(ln_ctx, true, "lstat(%s)", path_source);
  }
  else{
    if(ln_remove_dest(ln_ctx, &source_sb, path_dest)){
      if(ln_ctx->flags & LN_FLAG_SYMBOLIC){
        rc = symlink(path_source, path_dest);
      }
      else{
        if(S_ISLNK(source_sb.st_mode)){
          if(ln_ctx->flags & LN_FLAG_FOLLOW_SYMBOLIC){
            linkat_flag = AT_SYMLINK_FOLLOW;
          }
          else{
            linkat_flag = 0;
          }
          rc = linkat(AT_FDCWD, path_source, AT_FDCWD, path_dest, linkat_flag);
        }
        else{
          rc = link(path_source, path_dest);
        }
      }
      if(rc != 0){
        ln_warn(ln_ctx,
                true,
                "failed to create link: %s - %s",
                path_source,
                path_dest);
      }
    }
  }
}

/**
 * Store a link of a file inside a directory.
 *
 * @param[in,out] ln_ctx      See @ref ln_ctx.
 * @param[in]     source_file Create a link of this file in @p target_dir.
 * @param[in]     target_dir  Store a new link of @p source_file in this
 *                            directory.
 */
static void
ln_target_dir(struct ln_ctx *const ln_ctx,
              const char *const source_file,
              const char *const target_dir){
  char *path_dest;

  path_dest = ln_path_target_concat(target_dir, source_file);
  if(path_dest == NULL){
    ln_warn(ln_ctx, true, "alloc");
  }
  else{
    ln_create_link(ln_ctx, source_file, path_dest);
    free(path_dest);
  }
}

/**
 * Main entry point for ln utility.
 *
 * Usage:
 *
 * ln [-fs] [-L|-P] source_file target_file
 *
 * ln [-fs] [-L|-P] source_file... target_dir
 *
 * @param[in]     argc         Number of arguments in @p argv.
 * @param[in,out] argv         Argument list.
 * @retval        EXIT_SUCCESS All links created.
 * @retval        EXIT_FAILURE Failed to create at least one link.
 */
LINKAGE int
ln_main(int argc,
        char *argv[]){
  int c;
  int i;
  bool is_target_dir;
  struct ln_ctx ln_ctx;
  struct stat target_sb;

  memset(&ln_ctx, 0, sizeof(ln_ctx));
  while((c = getopt(argc, argv, "fLPs")) != -1){
    switch(c){
      case 'f':
        ln_ctx.flags |= LN_FLAG_REMOVE_DEST;
        break;
      case 'L':
        ln_ctx.flags |= LN_FLAG_FOLLOW_SYMBOLIC;
        break;
      case 'P':
        ln_ctx.flags &= ~(LN_FLAG_FOLLOW_SYMBOLIC);
        break;
      case 's':
        ln_ctx.flags |= LN_FLAG_SYMBOLIC;
        break;
      default:
        ln_ctx.status_code = EXIT_FAILURE;
        break;
    }
  }
  argc -= optind;
  argv += optind;

  if(ln_ctx.status_code == EXIT_SUCCESS){
    if(argc < 2){
      ln_warn(&ln_ctx, false, "must have >=2 file arguments");
    }
    else{
      is_target_dir = false;
      if(stat(argv[argc - 1], &target_sb) == 0){
        if(S_ISDIR(target_sb.st_mode)){
          is_target_dir = true;
          for(i = 0; i < argc - 1; i++){
            ln_target_dir(&ln_ctx, argv[i], argv[argc - 1]);
          }
        }
        else if(argc > 2){
          ln_warn(&ln_ctx,
                  false,
                  "final operand must be directory if > 2 operands");
        }
      }
      if(is_target_dir == false && ln_ctx.status_code == EXIT_SUCCESS){
        if(argc != 2){
          ln_warn(&ln_ctx,
                  false,
                  "only 2 operands allowed if final operand not a directory");
        }
        else{
          ln_create_link(&ln_ctx, argv[0], argv[1]);
        }
      }
    }
  }
  return ln_ctx.status_code;
}

#ifndef TEST
/**
 * Main program entry point.
 *
 * @param[in]     argc See @ref ln_main.
 * @param[in,out] argv See @ref ln_main.
 * @return             See @ref ln_main.
 */
int
main(int argc,
     char *argv[]){
  return ln_main(argc, argv);
}
#endif /* TEST */

