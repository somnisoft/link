/**
 * @file
 * @brief link utility
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <err.h>
#include <stdlib.h>
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
 * Main entry point for link utility.
 *
 * Usage:
 * link file1 file2
 *
 * @param[in] argc         Number of arguments in @p argv.
 * @param[in] argv         Argument list.
 * @retval    EXIT_SUCCESS Successful.
 * @retval    EXIT_FAILURE Error occurred.
 */
LINKAGE int
link_main(const int argc,
          char *const argv[]){
  int status_code;

  status_code = EXIT_SUCCESS;
  if(argc != 3){
    status_code = EXIT_FAILURE;
    warnx("must have exactly two file operands");
  }
  else{
    if(link(argv[1], argv[2]) != 0){
      status_code = EXIT_FAILURE;
      warn("failed to create link: \'%s\' - \'%s\'", argv[1], argv[2]);
    }
  }
  return status_code;
}

#ifndef TEST
/**
 * Main program entry point.
 *
 * @param[in] argc See @ref link_main.
 * @param[in] argv See @ref link_main.
 * @return         See @ref link_main.
 */
int
main(int argc,
     char *argv[]){
  return link_main(argc, argv);
}
#endif /* TEST */

