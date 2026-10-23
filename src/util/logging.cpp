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

#include "logging.h"

#include <chrono>
#include <climits>
#include <sstream>

namespace logging {

static int getIntFromEnvVar(const char *EnvVar, int DefaultIfUnset = 0,
                            int DefaultIfSet = 1) {
  const char *Value = getenv(EnvVar);
  if (Value == nullptr) {
    return DefaultIfUnset;
  }
  if (*Value == '\0') {
    return DefaultIfSet;
  }
  char *Endptr;
  long Result = strtol(Value, &Endptr, 10);
  if (*Endptr != '\0') {
    return DefaultIfSet; // String is not a number.
  }
  if (Result < INT_MIN || Result > INT_MAX) {
    return DefaultIfSet;
  }
  return static_cast<int>(Result);
}

static int deduceDebugLevel() {
  // Env var to override all output.
  auto X = getIntFromEnvVar("THNODE_SILENCE");
  if (X > 0) {
    return 0;
  }

  // Look for THNODE_DEBUG_LEVEL environment variable.
  char *Value = getenv("THNODE_DEBUG_LEVEL");
  if (Value == nullptr) {
    return 0;
  }

  // Check if value is empty.
  if (*Value == '\0') {
    FATAL << "THNODE_DEBUG_LEVEL is set but has no value.";
  }

  // Parse as integer
  char *Endptr;
  long Result = strtol(Value, &Endptr, 10);
  // Check if parsing failed or there are extra characters.
  if (*Endptr != '\0') {
    FATAL << "THNODE_DEBUG_LEVEL value '" << Value
          << "' is not a valid integer.";
  }

  // Check for overflow/underflow
  if (Result < 0 || Result > INT_MAX) {
    FATAL << "THNODE_DEBUG_LEVEL='" << Value << "' is out of range [0-"
          << INT_MAX << "]";
  }

  return static_cast<int>(Result);
}

int GDebugLevel = 0;

// Ensure init() is called before main() using a static initializer.
struct LoggingInitializer {
  LoggingInitializer() {
    GDebugLevel = deduceDebugLevel();
    LOG(1) << "debug level: " << logging::GDebugLevel;
  }
};
static LoggingInitializer LI;

std::string rfcTimestamp() {
  using namespace std::chrono;
  auto Now = system_clock::now();
  std::time_t NowTime = system_clock::to_time_t(Now);

  // Format time as RFC 3339: YYYY-MM-DDTHH:MM:SSZ
  std::tm TmUtc;
#if defined(_WIN32) || defined(_WIN64)
  gmtime_s(&tm_utc, &now_time);
#else
  gmtime_r(&NowTime, &TmUtc);
#endif

  char Buf[32];
  std::strftime(Buf, sizeof(Buf), "%Y-%m-%dT%H:%M:%SZ", &TmUtc);

  std::ostringstream Oss;
  Oss << "\n[" << Buf << "]";
  return Oss.str();
}

} // namespace logging
