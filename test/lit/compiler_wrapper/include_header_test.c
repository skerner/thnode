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

// RUN: ! env THNODE_CC=gcc THNODE_INCLUDE_HEADER="include/include_this_header.h" %compiler_driver %s -o %t_gcc.exe 2>&1 | %FileCheckSrc %s --check-prefix=CHECK-ERROR-RELATIVE-PATH
// RUN: ! env THNODE_CC=gcc THNODE_INCLUDE_HEADER="/this/file/does/not/exist.h" %compiler_driver %s -o %t_gcc.exe 2>&1 | %FileCheckSrc %s --check-prefix=CHECK-ERROR-INCLUDE-NOT-FOUND
// RUN: ! env THNODE_CC=gcc THNODE_INCLUDE_HEADER="/" %compiler_driver %s -o %t_gcc.exe 2>&1 | %FileCheckSrc %s --check-prefix=CHECK-ERROR-INCLUDE-IS-NOT-A-FILE
// RUN: env THNODE_DEBUG_LEVEL=10 THNODE_CC=gcc THNODE_INCLUDE_HEADER=%test_root_dir/compiler_wrapper/include/include_this_header.h %compiler_driver %s -o %t_gcc.exe
// RUN: %t_gcc.exe | FileCheck %s --check-prefix=CHECK-INCLUDE-ADDED

#include <stdio.h>

// CHECK-ERROR-RELATIVE-PATH: FATAL: THNODE_INCLUDE_HEADER should be set to an absolute path; got include/include_this_header.h

// CHECK-ERROR-INCLUDE-NOT-FOUND: FATAL: THNODE_INCLUDE_HEADER path does not exist: "/this/file/does/not/exist.h"

// CHECK-ERROR-INCLUDE-IS-NOT-A-FILE: FATAL: THNODE_INCLUDE_HEADER is not a file: "/"

// Tricky: When the clang plugin parses this file, the include has not been added.
// The macro is not defined, so the file can't be parsed.
// Work around this by defining it if it is not defined.
#ifndef A_MACRO
#define A_MACRO "A_MACRO was not defined in any #include."
#endif // A_MACRO

int main() {
  // CHECK-INCLUDE-ADDED: The macro A_MACRO has this value: this string comes from header 'include_this_header.h'
  printf("The macro A_MACRO has this value: %s\n", A_MACRO);

  return 0;
}
