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

#pragma once

#include <iostream>
#include <string>

namespace logging {

// These symbols are exposed because it is used by the LOG() macro:
extern int GDebugLevel;
std::string rfcTimestamp();

} // namespace logging

#define LOG(level)                                                             \
  if (logging::GDebugLevel >= level)                                           \
  logging::LogHelper() << logging::rfcTimestamp() << " " __FILE__ << ":"       \
                       << __LINE__ << " "

#define FATAL                                                                  \
  logging::FatalAndExit() = logging::LogHelper()                               \
                            << logging::rfcTimestamp() << " " __FILE__ << ":"  \
                            << __LINE__ << " "                                 \
                            << " FATAL: "

namespace logging {

class LogHelper {
public:
  LogHelper() {}
  template <typename T> LogHelper &operator<<(const T &Val) {
    std::cerr << Val;
    return *this;
  }

  // Specialization for std::endl.
  LogHelper &operator<<(std::ostream &(*Manip)(std::ostream &)) {
    Manip(std::cerr);
    return *this;
  }

  ~LogHelper() { std::cerr << std::endl; }
};

// Used by the macro FATAL to end the fatal log line and terminate the program.
// Older versions of GCC don't honor [[noreturn]] on a destructor.
// Use operator= as a workaround.
struct FatalAndExit {
  [[noreturn]] FatalAndExit &operator=(const LogHelper &) {
    std::cerr << std::endl;
    std::exit(1);
  }
};

} // namespace logging
