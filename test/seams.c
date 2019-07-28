/**
 * @file
 * @brief test seams
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

int g_test_seam_err_ctr_malloc = -1;

int g_test_seam_err_ctr_si_add_size_t = -1;

int g_test_seam_err_ctr_strdup = -1;

bool
test_seam_dec_err_ctr(int *const err_ctr){
  bool reached_end;

  reached_end = false;
  if(*err_ctr >= 0){
    *err_ctr -= 1;
    if(*err_ctr < 0){
      reached_end = true;
    }
  }
  return reached_end;
}

void *
test_seam_malloc(size_t size){
  void *alloc;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_malloc)){
    alloc = NULL;
    errno = ENOMEM;
  }
  else{
    alloc = malloc(size);
  }
  return alloc;
}

char *
test_seam_strdup(const char *s){
  void *alloc;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_strdup)){
    alloc = NULL;
  }
  else{
    alloc = strdup(s);
  }
  return alloc;
}

