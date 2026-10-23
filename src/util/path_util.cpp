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

#include "path_util.h"

#include <iostream>
#include <string>
#include <vector>

#include "logging.h"

using std::filesystem::path;

namespace path_util {

path changeDirInPath(path InitialPath, std::string DirFrom, std::string DirTo) {
  // Convert dir_from and dir_to to path for comparison
  path FromPath = DirFrom;
  path ToPath = DirTo;

  // Break the initial_path into components
  std::vector<path> Components;
  for (const auto &Part : InitialPath) {
    Components.push_back(Part);
  }

  // Find the deepest occurrence of dir_from
  int DeepestIdx = -1;
  for (size_t I = 0; I < Components.size(); ++I) {
    if (Components[I] == FromPath) {
      DeepestIdx = static_cast<int>(I);
    }
  }

  // If not found, return the original path
  if (DeepestIdx == -1) {
    return InitialPath;
  }

  // Replace the deepest occurrence with dir_to
  Components[DeepestIdx] = ToPath;

  // Reconstruct the path
  path NewPath;
  for (const auto &Part : Components) {
    NewPath /= Part;
  }
  return NewPath;
}

// This function emulates 'mkdir -p' behavior: create all directories in the
// path if they don't exist.
void mkdirs(path Mapped) {
  if (Mapped.empty())
    return;

  std::error_code Ec;
  if (!std::filesystem::exists(Mapped)) {
    if (!std::filesystem::create_directories(Mapped, Ec)) {
      FATAL << "ERROR: Failed to create directories for path: " << Mapped
            << " (" << Ec.message() << ")" << std::endl;
    }
  }
}

} // namespace path_util
