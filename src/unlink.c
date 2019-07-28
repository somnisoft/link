/**
 * @file
 * @brief unlink utility
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
 * Main entry point for unlink utility.
 *
 * @param[in] argc         Number of arguments in @p argv.
 * @param[in] argv         Argument list.
 * @retval    EXIT_SUCCESS Successful.
 * @retval    EXIT_FAILURE Error occurred.
 */
LINKAGE int
unlink_main(const int argc,
            char *const argv[]){
  int status_code;

  status_code = EXIT_SUCCESS;
  if(argc != 2){
    status_code = EXIT_FAILURE;
    warnx("must have exactly one file operand");
  }
  else{
    if(unlink(argv[1]) != 0){
      status_code = EXIT_FAILURE;
      warn("failed to unlink: %s", argv[1]);
    }
  }
  return status_code;
}

#ifndef TEST
/**
 * Main program entry point.
 *
 * @param[in] argc See @ref unlink_main.
 * @param[in] argv See @ref unlink_main.
 * @return         See @ref unlink_main.
 */
int
main(int argc,
     char *argv[]){
  return unlink_main(argc, argv);
}
#endif /* TEST */

