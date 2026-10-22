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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int check_denom_not_zero__intmax_t(intmax_t denom, const char *denom_expr, const char *file, int line) {
  if (denom == 0) {
    fprintf(stderr, "ERROR; for expression %s got value 0; want a value that is not zero; at %s:%d\n", denom_expr, file, line);
    abort();
  }

  return denom;
}

#define DENOM_CHECK(denom, denomType) (check_denom_not_zero__##denomType(denom, #denom, __FILE__, __LINE__))

int main() {
  int a = 10;
  int b = 0;

  int result = a;

  // CHECK-SOURCE: result /= DENOM_CHECK( (b), intmax_t);
  // CHECK-EXE: ERROR; for expression (b) got value 0; want a value that is not zero; at {{.*}}/divassign0.c.tmp.c:[[#@LINE + 1]]
  result /= b;
  return result;
}
