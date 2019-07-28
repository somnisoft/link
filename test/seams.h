/**
 * @file
 * @brief test seams
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */
#ifndef LINK_TEST_SEAMS_H
#define LINK_TEST_SEAMS_H

#include "test.h"

/*
 * Redefine these functions to internal test seams.
 */
#undef malloc
#undef strdup

/**
 * Inject a test seam to replace malloc().
 */
#define malloc test_seam_malloc

/**
 * Inject a test seam to replace strdup().
 */
#define strdup test_seam_strdup

#endif /* LINK_TEST_SEAMS_H */

