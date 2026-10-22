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

// CHECK-SOURCE: FileScopedConstexpr {{.*}} // DENOM-SKIPPED:IsConstInitializer
constexpr int FileScopedConstexpr = 10 / 4;
// CHECK-SOURCE: FileScopedConst {{.*}} // DENOM-SKIPPED:IsConstInitializer
const int FileScopedConst = 10 / 4;
// CHECK-SOURCE: FileScopedStaticConst {{.*}} // DENOM-SKIPPED:IsConstInitializer
static const int FileScopedStaticConst = 10 / 4;

int main() {
  // CHECK-SOURCE: FunctionScopedStaticConstant {{.*}} // DENOM-SKIPPED:IsConstInitializer
  static const int FunctionScopedStaticConstant = 17 / 7;

  // CHECK-SOURCE: FunctionScopedConstexpr {{.*}} // DENOM-SKIPPED:IsConstExprExpression
  constexpr int FunctionScopedConstexpr = 17 / 7;

  // CHECK-EXE: PASS
  printf("PASS\n");
  return 0;
}
