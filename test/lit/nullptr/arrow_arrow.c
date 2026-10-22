/*
Copyright 2026 the Thnode Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// RUN: %rewrite_c_file %s | tee %t.c | %FileCheckSrc %s --check-prefix=CHECK-SOURCE
// RUN: %clang -o %t.exe %t.c
// RUN: ! %t.exe 2>&1 | tee %t.exe.out | FileCheck %s --check-prefix=CHECK-EXE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void *my_null_check(void *ptr, const char *expr, const char *file, int line) {
  if (ptr < (void *)1024) {
    fprintf(stderr, "ERROR; Dereferencing NULLish pointer %p in expression %s; at %s:%d\n", ptr, expr, file, line);
    abort();
  }
  return ptr;
}

#define NULL_CHECK(ptrExpr, ptrType) ((ptrType)my_null_check((void *)ptrExpr, #ptrExpr, __FILE__, __LINE__))

typedef struct {
  int *an_int_ptr;
} inner_struct_t;

typedef struct {
  inner_struct_t *inner_struct_ptr;
} outer_struct_t;

int main() {
  int an_int = 42;

  inner_struct_t inner_struct;
  inner_struct.an_int_ptr = &an_int;

  outer_struct_t outer_struct;
  outer_struct.inner_struct_ptr = &inner_struct;

  outer_struct_t *outer_struct_ptr = &outer_struct;

  assert(*(outer_struct_ptr->inner_struct_ptr->an_int_ptr) == 42);

  inner_struct.an_int_ptr = (int *)0x123;

  // CHECK-SOURCE: *(NULL_CHECK( ((NULL_CHECK( (NULL_CHECK( (outer_struct_ptr), outer_struct_t *)->inner_struct_ptr), inner_struct_t *)->an_int_ptr)), int *) )
  // CHECK-EXE: ERROR; Dereferencing NULLish pointer 0x123 in expression ((NULL_CHECK( (NULL_CHECK( (outer_struct_ptr), outer_struct_t *)->inner_struct_ptr), inner_struct_t *)->an_int_ptr)); at {{.*}}.c:[[#@LINE + 1]]
  printf("FAIL; a_struct->an_int = %d\n", *(outer_struct_ptr->inner_struct_ptr->an_int_ptr));

  return 0;
}
