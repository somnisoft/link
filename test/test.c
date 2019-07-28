/**
 * @file
 * @brief test suite
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

/**
 * Test source file to link.
 */
#define PATH_SOURCE_1           "test-ln-source-1.txt"

/**
 * Another test source file to link.
 */
#define PATH_SOURCE_2           "test-ln-source-2.txt"

/**
 * Create the target link in the final parameter of the first synopsis.
 */
#define PATH_TARGET_FILE        "test-ln-target-file.txt"

/**
 * Create a symbolic link using this path.
 */
#define PATH_SYM                "test-ln-sym.txt"

/**
 * Create test links to the system hosts file.
 */
#define PATH_HOSTS              "/etc/hosts"

/**
 * Path to a file that has a different device ID than the new link.
 */
#define PATH_XDEV_DEST          "build/test-ln.txt"

/**
 * Create test links to the project COPYING file.
 */
#define PATH_COPYING            "COPYING"

/**
 * Create test links to the project README file.
 */
#define PATH_README             "README.md"

/**
 * Directory to add the new list of links.
 */
#define PATH_TARGET_DIR         "test-target-dir"

/**
 * Combine @ref PATH_TARGET_DIR with @ref PATH_COPYING.
 */
#define PATH_TARGET_DIR_COPYING (PATH_TARGET_DIR "/" PATH_COPYING)

/**
 * Combine @ref PATH_TARGET_DIR with @ref PATH_README.
 */
#define PATH_TARGET_DIR_README  (PATH_TARGET_DIR "/" PATH_README)

/**
 * Number of arguments in @ref g_argv.
 */
static int
g_argc;

/**
 * Argument list to pass to utility.
 */
static char **
g_argv;

/**
 * Ensure two files both hard linked.
 *
 * @param[in] file_1 Compare this file with @p file_2.
 * @param[in] file_2 Compare this file with @p file_1.
 */
static void
test_ln_hard_check(const char *const file_1,
                   const char *const file_2){
  struct stat sb_1;
  struct stat sb_2;

  assert(stat(file_1, &sb_1) == 0);
  assert(stat(file_2, &sb_2) == 0);
  assert(sb_1.st_dev == sb_2.st_dev);
  assert(sb_1.st_ino == sb_2.st_ino);
  assert(sb_1.st_nlink == sb_2.st_nlink);
  assert(sb_1.st_nlink == 2);
}

/**
 * Check if symlink points to another file.
 *
 * @param[in] file_1 @p file_2 should symlink to this file.
 * @param[in] file_2 Should symlink to @p file_1.
 */
static void
test_ln_soft_check(const char *const file_1,
                   const char *const file_2){
  struct stat sb_1;
  struct stat sb_rl;
  char path_link[1000];
  ssize_t nbytes_copy;

  nbytes_copy = readlink(file_2, path_link, sizeof(path_link));
  assert(nbytes_copy > 0);
  path_link[nbytes_copy] = '\0';
  assert(stat(file_1, &sb_1) == 0);
  assert(stat(path_link, &sb_rl) == 0);
  assert(sb_1.st_dev == sb_rl.st_dev);
  assert(sb_1.st_ino == sb_rl.st_ino);
}

/**
 * Call @ref link_main with the given arguments.
 *
 * @param[in] file1              See @ref link_main.
 * @param[in] file2              See @ref link_main.
 * @param[in] file3              Test error condition with extra operand.
 * @param[in] expect_exit_status Expected exit status code.
 */
static void
test_link_main(const char *const file1,
               const char *const file2,
               const char *const file3,
               const int expect_exit_status){
  int exit_status;

  g_argc = 0;
  strcpy(g_argv[g_argc++], "link");
  if(file1){
    strcpy(g_argv[g_argc++], file1);
  }
  if(file2){
    strcpy(g_argv[g_argc++], file2);
  }
  if(file3){
    strcpy(g_argv[g_argc++], file3);
  }
  optind = 0;
  exit_status = link_main(g_argc, g_argv);
  assert(exit_status == expect_exit_status);
  if(exit_status == EXIT_SUCCESS){
    test_ln_hard_check(file1, file2);
  }
}

/**
 * Call @ref ln_main with the given arguments.
 *
 * @param[in] f                  See @ref ln_main.
 * @param[in] L                  See @ref ln_main.
 * @param[in] P                  See @ref ln_main.
 * @param[in] s                  See @ref ln_main.
 * @param[in] extra_arg          Add another argument.
 * @param[in] expect_exit_status Expected exit status code.
 * @param[in] file_list          List of files to send to ln. Terminate list
 *                               with NULL.
 */
static void
test_ln_main(const bool f,
             const bool L,
             const bool P,
             const bool s,
             const bool extra_arg,
             const int expect_exit_status,
             const char *const file_list, ...){
  int exit_status;
  const char *file;
  va_list ap;

  g_argc = 0;
  strcpy(g_argv[g_argc++], "ln");
  if(f){
    strcpy(g_argv[g_argc++], "-f");
  }
  if(L){
    strcpy(g_argv[g_argc++], "-L");
  }
  if(P){
    strcpy(g_argv[g_argc++], "-P");
  }
  if(s){
    strcpy(g_argv[g_argc++], "-s");
  }
  if(extra_arg){
    strcpy(g_argv[g_argc++], "-a");
  }
  va_start(ap, file_list);
  for(file = file_list; file; file = va_arg(ap, const char *const)){
    strcpy(g_argv[g_argc++], file);
  }
  va_end(ap);
  optind = 0;
  exit_status = ln_main(g_argc, g_argv);
  assert(exit_status == expect_exit_status);
}

/**
 * Create a blank test file.
 *
 * @param[in] path Path to create new file.
 */
static void
test_ln_create_file(const char *const path){
  char cmd[1000];

  sprintf(cmd, "touch \'%s\'", path);
  assert(system(cmd) == 0);
}

/**
 * Call @ref unlink_main with the given arguments.
 *
 * @param[in] file1              See @ref unlink_main.
 * @param[in] file2              Test error condition with extra operand.
 * @param[in] expect_exit_status Expected exit status code.
 */
static void
test_unlink_main(const char *const file1,
                 const char *const file2,
                 const int expect_exit_status){
  int exit_status;

  g_argc = 0;
  strcpy(g_argv[g_argc++], "unlink");
  if(file1){
    strcpy(g_argv[g_argc++], file1);
  }
  if(file2){
    strcpy(g_argv[g_argc++], file2);
  }
  optind = 0;
  exit_status = unlink_main(g_argc, g_argv);
  assert(exit_status == expect_exit_status);
  if(exit_status == EXIT_SUCCESS){
    assert(access(file1, F_OK) != 0);
  }
}

/**
 * Test harness for @ref si_add_size_t.
 *
 * @param[in] a             Add this with @p b.
 * @param[in] b             Add this with @p a.
 * @param[in] expect_result Expected result of calculation.
 * @param[in] expect_wrap   Expected wrap return code.
 */
static void
test_unit_si_add_size_t(const size_t a,
                        const size_t b,
                        const size_t expect_result,
                        const bool expect_wrap){
  bool wraps;
  size_t result;

  wraps = !si_add_size_t(a, b, &result);
  assert(wraps == expect_wrap);
  if(wraps == false){
    assert(result == expect_result);
  }
}

/**
 * Run all miscellaneous unit tests.
 */
static void
test_all_unit(void){
  test_unit_si_add_size_t(0, 1, 1, false);
  test_unit_si_add_size_t(SIZE_MAX, 1, 0, true);
}

/**
 * Run all tests for link utility.
 */
static void
test_all_link(void){
  /* Successfully create a hard link. */
  test_link_main(PATH_README, PATH_SOURCE_1, NULL, EXIT_SUCCESS);
  assert(remove(PATH_SOURCE_1) == 0);

  /* File does not exist. */
  test_link_main(PATH_SOURCE_1, PATH_SOURCE_2, NULL, EXIT_FAILURE);

  /* Failed to link directory. */
  test_link_main("build/", PATH_SOURCE_1, NULL, EXIT_FAILURE);

  /* Only one operand. */
  test_link_main(PATH_SOURCE_1, NULL, NULL, EXIT_FAILURE);

  /* Too many operands. */
  test_link_main(PATH_SOURCE_1, PATH_SOURCE_2, PATH_README, EXIT_FAILURE);
}

/**
 * Run all tests for ln utility.
 */
static void
test_all_ln(void){
  int i;

  /* Too few arguments. */
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               NULL);

  /* Invalid argument. */
  test_ln_main(false,
               false,
               false,
               false,
               true,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               NULL);

  /* > 3 arguments target_file. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_SOURCE_1,
               PATH_SOURCE_1,
               NULL);
  assert(remove(PATH_SOURCE_1) == 0);

  /* More than 2 operands, but final operand not a directory. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_create_file(PATH_SOURCE_2);
  test_ln_main(false,
               false,
               false,
               true,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               "noexist",
               NULL);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /* Fail to stat the source file. */
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               NULL);

  /* Destination already exists. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_create_file(PATH_SOURCE_2);
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               NULL);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /* Fail to remove file when destination same as source. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_main(true,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_SOURCE_1,
               NULL);
  assert(remove(PATH_SOURCE_1) == 0);

  /* Fail to unlink destination file. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_main(true,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               PATH_HOSTS,
               NULL);
  assert(remove(PATH_SOURCE_1) == 0);

  /* Failed to create symlink. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_SOURCE_1,
               "/noperm",
               NULL);
  assert(remove(PATH_SOURCE_1) == 0);

  /* Successfully create a hard link. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_SUCCESS,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               NULL);
  test_ln_hard_check(PATH_SOURCE_1, PATH_SOURCE_2);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /* Remove destination file and successfully create the link. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_create_file(PATH_SOURCE_2);
  test_ln_main(true,
               false,
               false,
               false,
               false,
               EXIT_SUCCESS,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               NULL);
  test_ln_hard_check(PATH_SOURCE_1, PATH_SOURCE_2);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /*
   * Remove destination file on a different device and create the link.
   *
   * The file path pointed to by @ref PATH_XDEV_DEST must point to a
   * different device ID for this test to work.
   */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_create_file(PATH_XDEV_DEST);
  test_ln_main(true,
               false,
               false,
               true,
               false,
               EXIT_SUCCESS,
               PATH_SOURCE_1,
               PATH_XDEV_DEST,
               NULL);
  test_ln_soft_check(PATH_SOURCE_1, PATH_XDEV_DEST);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_XDEV_DEST) == 0);

  /* Successfully create a soft link. */
  test_ln_create_file(PATH_SOURCE_1);
  test_ln_main(false,
               false,
               false,
               true,
               false,
               EXIT_SUCCESS,
               PATH_SOURCE_1,
               PATH_SOURCE_2,
               NULL);
  test_ln_soft_check(PATH_SOURCE_1, PATH_SOURCE_2);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /* Create a hard link to the reference of a sym link (-L). */
  test_ln_create_file(PATH_SOURCE_2);
  assert(symlink("test-ln-source-2.txt", PATH_SYM) == 0);
  test_ln_main(false,
               true,
               false,
               false,
               false,
               EXIT_SUCCESS,
               PATH_SYM,
               PATH_SOURCE_1,
               NULL);
  test_ln_hard_check(PATH_SYM, PATH_SOURCE_2);
  assert(remove(PATH_SYM) == 0);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /* Create a hard link to the sym link (-P). */
  test_ln_create_file(PATH_SOURCE_2);
  assert(symlink("test-ln-source-2.txt", PATH_SYM) == 0);
  test_ln_main(false,
               false,
               true,
               false,
               false,
               EXIT_SUCCESS,
               PATH_SYM,
               PATH_SOURCE_1,
               NULL);
  test_ln_soft_check("test-ln-source-2.txt", "test-ln-sym.txt");
  assert(remove(PATH_SYM) == 0);
  assert(remove(PATH_SOURCE_1) == 0);
  assert(remove(PATH_SOURCE_2) == 0);

  /* Create a link inside a target_dir. */
  assert(mkdir(PATH_TARGET_DIR, 0777) == 0);
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_SUCCESS,
               PATH_README,
               PATH_TARGET_DIR,
               NULL);
  test_ln_hard_check(PATH_README, PATH_TARGET_DIR_README);
  assert(remove(PATH_TARGET_DIR_README) == 0);
  assert(rmdir(PATH_TARGET_DIR) == 0);

  /* Create multiple links inside a target_dir (with a '/'). */
  assert(mkdir(PATH_TARGET_DIR, 0777) == 0);
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_SUCCESS,
               PATH_README,
               PATH_COPYING,
               PATH_TARGET_DIR "/",
               NULL);
  test_ln_hard_check(PATH_COPYING, PATH_TARGET_DIR_COPYING);
  test_ln_hard_check(PATH_README, PATH_TARGET_DIR_README);
  assert(remove(PATH_TARGET_DIR_COPYING) == 0);
  assert(remove(PATH_TARGET_DIR_README) == 0);
  assert(rmdir(PATH_TARGET_DIR) == 0);

  /* Create multiple symlinks inside a target directory. */
  assert(mkdir(PATH_TARGET_DIR, 0777) == 0);
  test_ln_main(false,
               false,
               false,
               true,
               false,
               EXIT_SUCCESS,
               PATH_README,
               PATH_COPYING,
               PATH_HOSTS,
               PATH_TARGET_DIR,
               NULL);
  test_ln_soft_check(PATH_COPYING, PATH_TARGET_DIR_COPYING);
  test_ln_soft_check(PATH_README, PATH_TARGET_DIR_README);
  test_ln_soft_check(PATH_HOSTS, PATH_TARGET_DIR "/hosts");
  assert(remove(PATH_TARGET_DIR_COPYING) == 0);
  assert(remove(PATH_TARGET_DIR_README) == 0);
  assert(remove(PATH_TARGET_DIR "/hosts") == 0);
  assert(rmdir(PATH_TARGET_DIR) == 0);

  /* Failed to strdup source file. */
  assert(mkdir(PATH_TARGET_DIR, 0777) == 0);
  g_test_seam_err_ctr_malloc = 0;
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_README,
               PATH_TARGET_DIR,
               NULL);
  g_test_seam_err_ctr_malloc = -1;
  assert(rmdir(PATH_TARGET_DIR) == 0);

  /* Failed to malloc path. */
  assert(mkdir(PATH_TARGET_DIR, 0777) == 0);
  g_test_seam_err_ctr_strdup = 0;
  test_ln_main(false,
               false,
               false,
               false,
               false,
               EXIT_FAILURE,
               PATH_README,
               PATH_TARGET_DIR,
               NULL);
  g_test_seam_err_ctr_strdup = -1;
  assert(rmdir(PATH_TARGET_DIR) == 0);

  /* Wrap while adding size_t. */
  for(i = 0; i < 2; i++){
    g_test_seam_err_ctr_si_add_size_t = i;
    assert(mkdir(PATH_TARGET_DIR, 0777) == 0);
    test_ln_main(false,
                 false,
                 false,
                 false,
                 false,
                 EXIT_FAILURE,
                 PATH_README,
                 PATH_TARGET_DIR,
                 NULL);
    assert(rmdir(PATH_TARGET_DIR) == 0);
    g_test_seam_err_ctr_si_add_size_t = -1;
  }
}

/**
 * Run all tests for unlink utility.
 */
static void
test_all_unlink(void){
  FILE *fp;
  const char *const PATH_TMP_FILE = "build/test-unlink.txt";

  /* Successfully unlink a file. */
  fp = fopen(PATH_TMP_FILE, "w");
  assert(fp);
  assert(fclose(fp) == 0);
  test_unlink_main(PATH_TMP_FILE, NULL, EXIT_SUCCESS);

  /* Try to unlink a file that does not exist. */
  test_unlink_main(PATH_TMP_FILE, NULL, EXIT_FAILURE);

  /* Too few operands. */
  test_unlink_main(NULL, NULL, EXIT_FAILURE);

  /* Too many operands. */
  test_unlink_main(PATH_TMP_FILE, PATH_TMP_FILE, EXIT_FAILURE);
}

/**
 * Run all test cases for the link utilities.
 */
static void
test_all(void){
  remove(PATH_SOURCE_1);
  remove(PATH_SOURCE_2);
  remove(PATH_TARGET_FILE);
  remove(PATH_SYM);
  remove(PATH_XDEV_DEST);
  remove(PATH_TARGET_DIR_COPYING);
  remove(PATH_TARGET_DIR_README);
  remove(PATH_TARGET_DIR "/hosts");
  rmdir(PATH_TARGET_DIR);

  test_all_unit();
  test_all_link();
  test_all_ln();
  test_all_unlink();
}

/**
 * Test ln, link, and unlink utilities.
 *
 * Usage:
 * test
 *
 * @retval 0 All tests passed.
 */
int
main(void){
  const size_t MAX_ARGS = 20;
  const size_t MAX_ARG_LEN = 255;
  size_t i;

  g_argv = malloc(MAX_ARGS * sizeof(g_argv));
  assert(g_argv);
  for(i = 0; i < MAX_ARGS; i++){
    g_argv[i] = malloc(MAX_ARG_LEN * sizeof(*g_argv));
    assert(g_argv[i]);
  }
  test_all();
  for(i = 0; i < MAX_ARGS; i++){
    free(g_argv[i]);
  }
  free(g_argv);
  return 0;
}

