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

// RUN: %rewrite_cpp_file -fcxx-exceptions -fexceptions %s > %t.cpp
// RUN: cat %t.cpp | %FileCheckSrc %s --check-prefix=CHECK-SOURCE
// RUN: %clang++ -o %t.exe %t.cpp
// RUN: %t.exe 2>&1 | tee %t.exe.out | FileCheck %s --check-prefix=CHECK-EXE

// Run manually:
//   clear; do/build && do/rewrite_diff_exec test/lit/interactions/null_div0_negidx.cpp

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>

const char failure_type_none[] = "none";
const char failure_type_div_by_0[] = "div by 0";
const char failure_type_null_deref[] = "null deref";
const char failure_type_array_index[] = "array_index";

const char outcome_pass[] = "PASS";
const char outcome_fail[] = "FAIL";
const char *outcome_str(bool outcome) {
  return outcome ? outcome_pass : outcome_fail;
}

constexpr int ARRAY_LENGTH = 10;

void *my_null_check(void *ptr, const char *expr, const char *file, int line) {
  if (ptr < (void *)1024) {
    throw failure_type_null_deref;
  }
  return ptr;
}

#define NULL_CHECK(ptrExpr, ptrType) ((ptrType)my_null_check((void *)ptrExpr, #ptrExpr, __FILE__, __LINE__))

template <class T>
int check_denom_not_zero(T denom, const char *denom_expr, const char *file, int line) {
  if (denom == static_cast<T>(0)) {
    throw failure_type_div_by_0;
  }

  return denom;
}

#define DENOM_CHECK(denom, denomType) (check_denom_not_zero(denom, #denom, __FILE__, __LINE__))

int check_idx_ge_zero(int idx, const char *arr_expr, const char *idx_expr, const char *file, int line) {
  if (idx < 0) {
    throw failure_type_array_index;
  }

  return idx;
}

#define ARRAY_ACCESS(arr, idx) (arr[check_idx_ge_zero(idx, #arr, #idx, __FILE__, __LINE__)])

int lots_of_trouble_in_one_tiny_function(int *arr[], int idx_num, int idx_denom) {
  // CHECK-SOURCE: int x = idx_num / DENOM_CHECK( (idx_denom), intmax_t);
  int x = idx_num / idx_denom;

  // CHECK-SOURCE: int *y = ARRAY_ACCESS(arr, idx_num / DENOM_CHECK( (idx_denom), intmax_t));
  int *y = arr[idx_num / idx_denom];

  // CHECK-SOURCE: return *(NULL_CHECK( ((ARRAY_ACCESS(arr, idx_num / DENOM_CHECK( (idx_denom), intmax_t)))), int *) );
  return *(arr[idx_num / idx_denom]);
}

int g_fails = 0;

template <class T>
void expect_eq(T got, T want, const char *msg) {
  if (got != want) {
    g_fails++;
  }
  std::cout << outcome_str(got == want) << "; " << msg << "; got " << got << "; want " << want << "\n";
}

void fail(const char *msg) {
  g_fails++;
  std::cout << outcome_str(false) << "; " << msg << "\n";
}

int main() {
  // Set up an array of pointers to ints. Element 0 is NULL, the rest are not:
  int an_int = 123;
  int *an_array_of_int_ptrs[ARRAY_LENGTH];
  for (int i = 0; i < ARRAY_LENGTH; i++) {
    an_array_of_int_ptrs[i] = (i == 0 ? NULL : &an_int);
  }

  try {
    int got = lots_of_trouble_in_one_tiny_function(an_array_of_int_ptrs, 1, 1);
    expect_eq(got, an_int, "arr, 1, 1");
  } catch (const char *ex) {
    printf("FAIL: Got unexpected exception: %s\n", ex);
  }

  try {
    int got = lots_of_trouble_in_one_tiny_function(an_array_of_int_ptrs, 1, 0);
    fail("Expected div0 exception");

  } catch (const char *ex) {
    expect_eq(ex, failure_type_div_by_0, "1/0 should throw");
  }

  try {
    int got = lots_of_trouble_in_one_tiny_function(an_array_of_int_ptrs, 0, 1);
    fail("Expected null deref exception");

  } catch (const char *ex) {
    expect_eq(ex, failure_type_null_deref, "0/1 should throw");
  }
  try {
    int got = lots_of_trouble_in_one_tiny_function(an_array_of_int_ptrs, -1, 1);
    fail("Expected array index exception");

  } catch (const char *ex) {
    expect_eq(ex, failure_type_array_index, "arr[-1] should throw");
  }

  if (g_fails > 0) {
    printf("FAIL: %d tests failed\n", g_fails);
    return 1;
  }

  // CHECK-EXE: PASS all tests
  printf("PASS all tests\n");
  return 0;
}
