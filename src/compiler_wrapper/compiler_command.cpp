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

#include "compiler_command.h"

#include "compiler_info/clang_includes.h"
#include "util/logging.h"
#include "util/string_util.h"

using std::string;
using std::vector;
using std::filesystem::path;

static string deduceCompilerPluginPath() {
  const char *ThnodePluginPath = std::getenv("THNODE_PLUGIN_PATH");
  if (ThnodePluginPath != nullptr && *ThnodePluginPath != '\0') {
    return std::string(ThnodePluginPath);
  }

  FATAL << "failed to determine the compiler plugin path; set env var "
           "THNODE_PLUGIN_PATH.";
}

static const vector<string> &deduceCompilerPluginIncludeFlags() {
  static vector<string> Includes =
      clang_include_path::getIncludePathsForCompiler("clang", true);
  return Includes;
}

static bool hasInstrumentableExtension(const std::string &Filename) {
  if (strings::endsWith(Filename, ".c"))
    return true;

  if (strings::endsWith(Filename, ".cpp"))
    return true;

  return false;
}

static bool isInstrumented(const std::string &Filename) {
  return Filename.find(".inst.") != std::string::npos;
}

static bool shouldInstrument(const std::string &Filename) {
  if (!hasInstrumentableExtension(Filename)) {
    return false;
  }

  // Never instrument something already instrumented.
  if (isInstrumented(Filename)) {
    return false;
  }

  return true;
}

static path instrumentedVersionOf(const path &SourceFile) {
  if (!shouldInstrument(SourceFile.string())) {
    FATAL << "Path " << SourceFile << " is already an instrumented path.";
  }

  auto Dir = SourceFile.parent_path();
  auto Stem = SourceFile.stem().string();
  auto Ext = SourceFile.extension().string();

  path Result = Dir / (Stem + ".inst" + Ext);

  LOG(12) << "InstrumentedVersionOf: " << SourceFile << "\t=>\t" << Result;
  return Result;
}

std::ostream &operator<<(std::ostream &Os, const CompilerCommand &Cmd) {
  Os << std::endl;
  for (size_t I = 0; I < Cmd.Argv_.size(); ++I) {
    Os << "\t\targv[" << I << "] = " << Cmd.Argv_[I] << std::endl;
  }
  return Os;
}

CompilerCommand CompilerCommand::withSourcePathsRewritten() const {
  auto UpdatedArgv = Argv_;

  for (int I = 1; I < Argv_.size(); I++) {
    if (shouldInstrument(Argv_[I])) {
      UpdatedArgv[I] = instrumentedVersionOf(Argv_[I]);
    }
  }

  CompilerCommand Result(UpdatedArgv, FilesWritten_);
  return Result;
}

CompilerCommand CompilerCommand::withIncludeAdded(
    std::optional<std::string> ThnodeIncludeHeader) const {
  auto UpdatedArgv = Argv_;

  if (ThnodeIncludeHeader) {
    std::filesystem::path IncludeHeaderPath(ThnodeIncludeHeader.value());
    std::string IncludeHeaderDir = IncludeHeaderPath.parent_path().string();
    UpdatedArgv.push_back("-I" + IncludeHeaderDir);
  }

  CompilerCommand Result(UpdatedArgv, FilesWritten_);
  return Result;
}

static void vectorAppendAll(vector<string> &AppendTo,
                            const vector<string> &CopyFrom) {
  AppendTo.insert(AppendTo.end(), CopyFrom.begin(), CopyFrom.end());
}

std::vector<CompilerCommand> CompilerCommand::makeInstrumentationCommands(
    std::optional<std::string> ThnodeIncludeHeader) const {
  // Found the set of source files to be instrumented.
  std::vector<std::string> ToInstrument;
  for (int I = 1; I < Argv_.size(); I++) {
    // Locate all source files that should be instrumented.
    if (!shouldInstrument(Argv_[I])) {
      continue;
    }

    ToInstrument.push_back(Argv_[I]);
  }

  if (ToInstrument.empty()) {
    return {}; // Nothing to instrument.
  }

  // Find command line args that need to be passed to clang when running the
  // plugin.
  std::vector<std::string> FlagsToPreserve;
  for (int I = 1; I < Argv_.size(); I++) {
    if (strings::startsWith(Argv_[I], "-D")) {
      FlagsToPreserve.push_back(Argv_[I]);
      continue;
    }
    if (strings::startsWith(Argv_[I], "-I")) {
      FlagsToPreserve.push_back(Argv_[I]);
      continue;
    }

    // TODO: support flags that have a space between flag and value: "-I .",
    // "-D FOO=1", etc.
  }

  static auto PluginPath = deduceCompilerPluginPath();

  static auto IncludesForPlugin = deduceCompilerPluginIncludeFlags();

  std::vector<CompilerCommand> Results;
  for (int J = 0; J < ToInstrument.size(); J++) {

    // Start with clang flags to load the plugin.
    std::vector<std::string> InstrumentCommand = {
        "clang", "-cc1", "-load", PluginPath, "-plugin", "safety-checks"};

    // Add the flags that were passed to the compiler driver.
    vectorAppendAll(InstrumentCommand, FlagsToPreserve);

    // Add the -I flags the normal compiler driver sets by default.
    vectorAppendAll(InstrumentCommand, IncludesForPlugin);

    // Remove any older copy of the file.
    std::filesystem::path InstFile = instrumentedVersionOf(ToInstrument[J]);
    if (std::filesystem::exists(InstFile)) {
      std::filesystem::remove(InstFile);
    }

    if (ThnodeIncludeHeader.has_value()) {
      std::filesystem::path IncludeHeaderPath(ThnodeIncludeHeader.value());
      std::string IncludeHeaderDir = IncludeHeaderPath.parent_path().string();
      InstrumentCommand.push_back("-I" + IncludeHeaderDir);
      std::string IncludeHeaderFile = IncludeHeaderPath.filename().string();
    }

    auto AddFlagForPlugin = [&](const std::string &Flag,
                                const std::string &Value) {
      InstrumentCommand.push_back("-plugin-arg-safety-checks");
      InstrumentCommand.push_back(Flag);
      InstrumentCommand.push_back("-plugin-arg-safety-checks");
      InstrumentCommand.push_back(Value);
    };

    // Read to_instrument[j], direct output to .inst.c .
    InstrumentCommand.push_back(ToInstrument[J]);
    auto InstrumentedPath = instrumentedVersionOf(ToInstrument[J]);

    AddFlagForPlugin("-o", InstrumentedPath.string());

    if (ThnodeIncludeHeader.has_value()) {
      std::filesystem::path IncludeHeaderPath(ThnodeIncludeHeader.value());
      std::string IncludeHeaderDir = IncludeHeaderPath.parent_path().string();
      std::string IncludeHeaderFile = IncludeHeaderPath.filename().string();

      AddFlagForPlugin("-add_include", IncludeHeaderFile);
    }

    std::vector<std::filesystem::path> FilesWritten{InstrumentedPath};
    LOG(1) << "Add to files written: " << InstrumentedPath;
    Results.push_back(CompilerCommand(InstrumentCommand, FilesWritten));
  }
  return Results;
}

std::string CompilerCommand::toString() const {
  std::stringstream SS;
  for (size_t I = 0, IE = Argv_.size(); I < IE; ++I) {
    if (I > 0)
      SS << " ";
    SS << Argv_[I];
  }
  return SS.str();
}
