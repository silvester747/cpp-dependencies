/*
 * Copyright (C) 2012-2016. TomTom International BV (http://tomtom.com).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BazelRegen.h"

#include <set>
#include <string>

#include "Component.h"
#include "FstreamInclude.h"

const std::string BAZEL_FILE("BUILD");
//const std::string BAZEL_FILE_GENERATED("BUILD.generated");

const std::string GetBazelType(Component* comp) {
  if (comp->type == "add_executable") {
    return "cc_binary";
  } else {
    return "cc_library";
  }
}

const std::string GetBazelDep(Component* comp) {
  return "/" + comp->root.generic_string().substr(1) + ":" + comp->CmakeName();
}

void RegenerateBazelFilesForComponent(const Configuration& config,
                                      Component* comp,
                                      bool dryRun, 
                                      bool writeToStdout) {

  streams::ofstream out(comp->root / BAZEL_FILE);

  std::set<std::string> hdrs, srcs;
  auto fileNamePrefixLen = comp->root.generic_string().size() + 1;
  for (auto& fp : comp->files) {
    if (fp->hasExternalInclude) {
      hdrs.insert(fp->path.generic_string().substr(fileNamePrefixLen));
    } else {
      srcs.insert(fp->path.generic_string().substr(fileNamePrefixLen));
    }
  }

  std::set<std::string> deps;
  for (auto& dep : comp->privDeps) {
    deps.insert(GetBazelDep(dep));
  }
  for (auto& dep : comp->pubDeps) {
    deps.insert(GetBazelDep(dep));
  }

  out << GetBazelType(comp) << "(\n";
  out << "    name = \"" << comp->CmakeName() << "\",\n";
  if (!srcs.empty()) {
    out << "    srcs = [\n";
    for (auto& srcName : srcs) {
      out << "        \"" << srcName << "\",\n";
    }
    out << "    ],\n";
  }
  if (!hdrs.empty()) {
    out << "    hdrs = [\n";
    for (auto& hdrName : hdrs) {
      out << "        \"" << hdrName << "\",\n";
    }
    out << "    ],\n";
  }
  if (!deps.empty()) {
    out << "    deps = [\n";
    for (auto& dep : deps) {
      out << "        \"" << dep << "\",\n";
    }
    out << "    ],\n";
  }
  out << "    visibility = [\"//visibility:public\"],\n";
  out << "    strip_include_prefix = \"Interface\", \n";
  out << ")\n";

  out.close();
}
