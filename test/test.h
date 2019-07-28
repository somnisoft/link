/**
 * @file
 * @brief test suite
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */
#ifndef LINK_TEST_H
#define LINK_TEST_H

#include <stdbool.h>

int
link_main(int argc,
          char *const argv[]);

int
ln_main(int argc,
        char *argv[]);

int
unlink_main(int argc,
            char *const argv[]);

bool
si_add_size_t(const size_t a,
              const size_t b,
              size_t *const result);

/**
 * Decrement an error counter until it reaches -1.
 *
 * After a counter reaches -1, it will return a true response. This gets
 * used by the test suite to denote when to cause a function to fail. For
 * example, the unit test might need to cause the malloc() function to fail
 * after calling it a third time. In that case, the counter should initially
 * get set to 2 and will get decremented every time this function gets called.
 *
 * @param[in,out] err_ctr Error counter to decrement.
 * @retval        true    The counter has reached -1.
 * @retval        false   The counter has been decremented, but did not reach
 *                        -1 yet.
 */
bool
test_seam_dec_err_ctr(int *const err_ctr);

/**
 * Control when malloc() fails.
 *
 * @param[in] size  Number of bytes to allocate.
 * @retval    void* Allocated memory.
 * @retval    NULL  Failed to allocate memory.
 */
void *
test_seam_malloc(size_t size);

/**
 * Control when strdup() fails.
 *
 * @param[in] s     String to duplicate.
 * @retval    char* Duplicated string.
 * @retval    NULL  Failed to allocate memory.
 */
char *
test_seam_strdup(const char *s);

/**
 * Error counter for @ref test_seam_malloc.
 */
extern int g_test_seam_err_ctr_malloc;

/**
 * Error counter for @ref si_add_size_t.
 */
extern int g_test_seam_err_ctr_si_add_size_t;

/**
 * Error counter for @ref test_seam_strdup.
 */
extern int g_test_seam_err_ctr_strdup;

#endif /* LINK_TEST_H */

