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
  int an_int;
} a_struct_t;

int main() {
  a_struct_t a_struct;
  a_struct.an_int = 42;

  a_struct_t *a_struct_ptr = &a_struct;

  assert(a_struct_ptr->an_int == 42);

  a_struct_ptr = (a_struct_t *)0x123;
  // CHECK-SOURCE: NULL_CHECK( (a_struct_ptr), a_struct_t *)->an_int)
  // CHECK-EXE: ERROR; Dereferencing NULLish pointer 0x123 in expression (a_struct_ptr); at {{.*}}.c:[[#@LINE + 1]]
  printf("FAIL; a_struct->an_int = %d\n", a_struct_ptr->an_int);

  return 0;
}
