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

// RUN: ! env THNODE_CC=gcc THNODE_CONFIG='this is not valid!' %compiler_driver %s -o %t_inst.exe 2>&1 | %FileCheckSrc %s

// CHECK: ERROR: safty-check plugin: Failed to parse config string: "this is not valid!"
