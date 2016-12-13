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

#include "Configuration.h"
#include "Constants.h"
#include "FstreamInclude.h"
#include <iostream>
#include <stdlib.h>

// trim from start (in place)
static inline void LTrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void RTrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void Trim(std::string &s) {
    LTrim(s);
    RTrim(s);
}

void ReadSet(std::unordered_set<std::string>& set,
             streams::ifstream& in)
{
  std::string line;
  while (in.good()) {
    std::getline(in, line);
    size_t pos = line.find_first_of("]");
    if (pos != std::string::npos) {
      return;
    }
    Trim(line);
    set.insert(line);
  }
}

Configuration::Configuration()
: companyName("YourCompany")
, licenseString("")
, regenTag("GENERATED BY CPP-DEPENDENCIES")
, versionUsed(CURRENT_VERSION)
, cycleColor("orange")
, publicDepColor("blue")
, privateDepColor("lightblue")
, componentLinkLimit(30)
, componentLocLowerLimit(200)
, componentLocUpperLimit(20000)
, fileLocUpperLimit(2000)
{
  addLibraryAliases.insert("add_library");
  addExecutableAliases.insert("add_executable");

  streams::ifstream in(CONFIG_FILE);
  std::string line;
  while (in.good()) {
    std::getline(in, line);
    size_t pos = line.find_first_of("#");
    if (pos != std::string::npos)
      line.resize(line.find_first_of("#"));

    pos = line.find(": ");
    if (pos == std::string::npos)
      continue;

    std::string name = line.substr(0, pos);
    std::string value = line.substr(pos+2);
    if (name == "cycleColor") { cycleColor = value; }
    else if (name == "publicDepColor") { publicDepColor = value; }
    else if (name == "versionUsed") { versionUsed = value; }
    else if (name == "privateDepColor") { privateDepColor = value; }
    else if (name == "regenTag") { regenTag = value; }
    else if (name == "companyName") { companyName = value; }
    else if (name == "componentLinkLimit") { componentLinkLimit = atol(value.c_str()); }
    else if (name == "componentLocLowerLimit") { componentLocLowerLimit = atol(value.c_str()); }
    else if (name == "componentLocUpperLimit") { componentLocUpperLimit = atol(value.c_str()); }
    else if (name == "fileLocUpperLimit") { fileLocUpperLimit = atol(value.c_str()); }
    else if (name == "addLibraryAlias") { ReadSet(addLibraryAliases, in); }
    else if (name == "addExecutableAlias") { ReadSet(addExecutableAliases, in); }
    else {
      std::cout << "Ignoring unknown tag in configuration file: " << name << "\n";
    }
  }
}

static Configuration config;

const Configuration& Configuration::Get()
{
  return config;
}

void Configuration::Set(Configuration& newConfig)
{
  config = newConfig;
}


