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

#include <stdlib.h>

void foo(unsigned char **ucpp) {
  // CHECK-SOURCE: *(NULL_CHECK( ((*(NULL_CHECK( (ucpp), unsigned char **) ))++), unsigned char *) ) = (unsigned char)123;
  *(*ucpp)++ = (unsigned char)123;

#define STAR_UCPP (*ucpp)

  // The macro makes this line impossible for a plugin to edit.
  // CHECK-SOURCE: *STAR_UCPP++ = (unsigned char)123;
  *STAR_UCPP++ = (unsigned char)123;
}
