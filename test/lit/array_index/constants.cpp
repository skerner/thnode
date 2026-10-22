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

// RUN: %rewrite_cpp_file %s | tee %t.cpp | %FileCheckSrc %s --check-prefix=CHECK-SOURCE
// RUN: %clang++ %t.cpp -o %t.exe
// RUN: %t.exe 2>&1 | tee %t.exe.out | FileCheck %s --check-prefix=CHECK-EXE

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static const int StaticConstIntArray[] = {0, 1, 2, 3, 4, 5, 6};

// CHECK-SOURCE: SizeOfStaticConstIntArray {{.*}} // ARRAY-SKIPPED:IsConstInitializer // DENOM-SKIPPED:IsConstInitializer
static const size_t SizeOfStaticConstIntArray = sizeof(StaticConstIntArray) / sizeof(StaticConstIntArray[0]);

constexpr int ConstexprIntArray[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
constexpr int ConstExprInt = ConstexprIntArray[3];

int main() {
  // CHECK-SOURCE: FunctionScopedStaticConstant {{.*}} // ARRAY-SKIPPED:IsConstInitializer
  static const char FunctionScopedStaticConstant = "abcdefg"[2];

  // CHECK-SOURCE: FunctionScopedConstexpr {{.*}} // ARRAY-SKIPPED:IsConstExprExpression
  constexpr int FunctionScopedConstexpr = ConstexprIntArray[1];

  // CHECK-EXE: PASS
  printf("PASS\n");
  return 0;
}
