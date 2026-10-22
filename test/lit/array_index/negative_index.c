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

// RUN: %rewrite_c_file %s -Wno-array-bounds | tee %t.c | %FileCheckSrc %s --check-prefix=CHECK-SOURCE
// RUN: %clang -o %t.exe %t.c -Wno-array-bounds
// RUN: ! %t.exe 2>&1 | tee %t.exe.out | FileCheck %s --check-prefix=CHECK-EXE

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LENGTH 10

int check_idx_ge_zero(int idx, const char *arr_expr, const char *idx_expr, const char *file, int line) {
  if (idx < 0) {
    fprintf(stderr, "ERROR; for expression %s[%s] got index value %d; want a value that is not negative; at %s:%d", arr_expr, idx_expr, idx, file, line);
    abort();
  }

  return idx;
}

#define ARRAY_ACCESS(arr, idx) (arr[check_idx_ge_zero(idx, #arr, #idx, __FILE__, __LINE__)])

int main() {
  int array[ARRAY_LENGTH];

  // CHECK-SOURCE: int result = ARRAY_ACCESS(array, -1);
  // CHECK-EXE: ERROR; for expression array[-1] got index value -1; want a value that is not negative; at {{.*}}/negative_index.c.tmp.c:[[#@LINE + 1]]
  int result = array[-1];

  return result;
}
