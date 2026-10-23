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

// Drop-in replacement for gcc/clang that instruments the code being compiled.
//
// Usage:
//  $ CC=thnode-cc CXX=thnode-c++ ./configure
//
//  $ cmake -DCMAKE_C_COMPILERC=thnode-cc -DCMAKE_CXX_COMPILER=thnode-c++ .

#include "compiler_command.h"
#include "util/logging.h"
#include "util/path_util.h"

#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <optional>

using std::string;
using std::filesystem::path;

std::string parseExeName(const char *ArgvZero) {
  std::string ExePath(ArgvZero);

  size_t Pos = ExePath.find_last_of("/");
  if (Pos == std::string::npos) {
    return ExePath;
  }

  return ExePath.substr(Pos + 1);
}

std::string deduceCompilerToCall(const char *Argv0) {
  const char *ThnodeCc = std::getenv("THNODE_CC");
  if (ThnodeCc != nullptr && *ThnodeCc != '\0') {
    return std::string(ThnodeCc);
  }

  // Extract only the executable name. Strip dirs out of the path.
  std::string ExeName = parseExeName(Argv0);
  LOG(1) << "exe name is " << ExeName;
  // TODO: act on the name by returning the equivalent compiler.

  FATAL << "failed to determine the compiler to be called; set env var "
           "THONDE_CC or name the binary something compilerish.";
}

int exec(const string &Cmd) {
  std::string Cwd = std::filesystem::current_path().string();
  LOG(1) << "From CWD " << Cwd;
  LOG(1) << "\nExec command: " << Cmd;
  int Result = std::system(Cmd.c_str());
  LOG(1) << "\tCommand returned " << Result;
  return Result;
}

std::optional<std::filesystem::path> getIncludeHeaderPath() {
  const char *EnvVal = std::getenv("THNODE_INCLUDE_HEADER");
  if (EnvVal == nullptr || *EnvVal == '\0') {
    return std::nullopt;
  }
  std::filesystem::path HeaderPath(EnvVal);
  if (!HeaderPath.is_absolute()) {
    FATAL << "THNODE_INCLUDE_HEADER should be set to an absolute path; got "
          << EnvVal;
  }

  if (!std::filesystem::exists(HeaderPath)) {
    FATAL << "THNODE_INCLUDE_HEADER path does not exist: " << HeaderPath;
  }
  if (!std::filesystem::is_regular_file(HeaderPath)) {
    FATAL << "THNODE_INCLUDE_HEADER is not a file: " << HeaderPath;
  }

  return HeaderPath;
}

// Instrumemnt each file, then build instrumented results.
int instrumentAndBuildInSameDir(const CompilerCommand &Cmd) {
  LOG(1) << "START! Args: " << Cmd;

  auto CompilerToCall = deduceCompilerToCall(Cmd.exePath().c_str());
  LOG(1) << "compiler to call: " << CompilerToCall;

  std::optional<std::filesystem::path> ThnodeIncludeHeader =
      getIncludeHeaderPath();

  std::vector<CompilerCommand> InstrumentationCommands =
      Cmd.makeInstrumentationCommands(ThnodeIncludeHeader);
  for (CompilerCommand &InstCmd : InstrumentationCommands) {
    int ExitCode = exec(InstCmd.toString());
    if (ExitCode != 0) {
      FATAL << "Instrumentation with clang plugin failed with shell code "
            << ExitCode;
    }
  }

  CompilerCommand BuildWithInstrumentation =
      Cmd.withCompilerCommand(CompilerToCall)
          .withSourcePathsRewritten()
          .withIncludeAdded(ThnodeIncludeHeader);

  int RetCode = exec(BuildWithInstrumentation.toString());
  if (RetCode != 0) {
    FATAL << "Build of instrumented code failed with shell code " << RetCode;
  }

  // Remove the instrumented file.
  // Some libraries will build instrumented files unless we clean them up.
  for (CompilerCommand &InstCmd : InstrumentationCommands) {
    for (const auto &InstFile : InstCmd.filesWritten()) {
      if (std::filesystem::exists(InstFile)) {
        LOG(1) << "Removing instrumented file " << InstFile;
        std::filesystem::remove(InstFile);
      } else {
        FATAL << "Instrumented file " << InstFile << " does not exist? ";
      }
    }
  }

  return 0;
}

int main(int argc, char **argv, char **envp) {
  CompilerCommand Cmd(argc, argv);

  return instrumentAndBuildInSameDir(Cmd);
}
