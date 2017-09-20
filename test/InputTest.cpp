#include "test.h"
#include "TestUtils.h"

#include "Component.h"
#include "Configuration.h"
#include "Constants.h"
#include "Input.h"
#include "FilesystemInclude.h"
#include "FstreamInclude.h"
#include <algorithm>
#include <sstream>

void CreateCMakeProject(const std::string& projectName,
                        const std::string& alias,
                        const filesystem::path& workDir)
{
  filesystem::create_directories(workDir / projectName);
  streams::ofstream out(workDir / projectName / "CMakeLists.txt");
  out << "project(" << projectName << ")\n"
      << alias << "(${PROJECT_NAME}\n"
      << "  somesourcefile.cpp\n"
      << ")\n";
}

TEST(Input_Aliases)
{
  TemporaryWorkingDirectory workDir(name);

  CreateCMakeProject("Renderer", "add_render_library", workDir());
  CreateCMakeProject("UI", "add_ui_library", workDir());
  CreateCMakeProject("DrawTest", "add_test", workDir());
  CreateCMakeProject("Service", "add_service", workDir());

  {
    streams::ofstream out(workDir() / "CMakeLists.txt");
    out << "add_subdirectory(Renderer)\n"
        << "add_subdirectory(UI)\n"
        << "add_subdirectory(DrawTest)\n"
        << "add_subdirectory(Service)\n";
  }

  std::stringstream ss;
  ss << "addLibraryAlias: [\n"
     << "  add_render_library\n"
     << "  add_ui_library\n"
     << "  ]\n"
     << "addExecutableAlias: [\n"
     << "  add_service\n"
     << "  add_test\n"
     << "  ]\n";

  Configuration config;
  config.read(ss);

  std::unordered_map<std::string, Component*> components;
  std::unordered_map<std::string, File> files;

  LoadFileList(config, components, files, workDir(), false, false);

  ASSERT(components.size() == 5);

  for (auto& pair : components) {
    const Component& comp = *pair.second;

    if (comp.CmakeName() == "Renderer") {
      ASSERT(comp.type == "add_render_library");
    } else if (comp.CmakeName() == "UI") {
      ASSERT(comp.type == "add_ui_library");
    } else if (comp.CmakeName() == "DrawTest") {
      ASSERT(comp.type == "add_test");
    } else if (comp.CmakeName() == "Service") {
      ASSERT(comp.type == "add_service");
    } else {
      ASSERT(comp.CmakeName() == "ROOT");
    }
  }
}

TEST(Input_Children)
{
  TemporaryWorkingDirectory workDir(name);

  CreateCMakeProject("BigComponent", "add_library", workDir());
  CreateCMakeProject("SubComponentA", "add_library", workDir() / "BigComponent");
  streams::ofstream(workDir() / "CMakeLists.txt").close();

  Configuration config;
  std::unordered_map<std::string, Component*> components;
  std::unordered_map<std::string, File> files;

  LoadFileList(config, components, files, workDir(), false, false);

  ASSERT(components.size() == 3); // 2 components + ROOT

  for (auto& pair : components) {
    const Component& comp = *pair.second;

    if (comp.CmakeName() == "BigComponent") {
      ASSERT(comp.children.size() == 1);
      ASSERT(std::count_if(comp.children.cbegin(),
                           comp.children.cend(),
                           [](const Component* c) { return c->CmakeName() == "SubComponentA"; }));
    } else if (comp.CmakeName() == "SubComponentA") {
      ASSERT(comp.children.size() == 0);
    } else {
      ASSERT(comp.CmakeName() == "ROOT");
      ASSERT(comp.children.size() == 1);
      ASSERT(std::count_if(comp.children.cbegin(),
                           comp.children.cend(),
                           [](const Component* c) { return c->CmakeName() == "BigComponent"; }));
    }
  }
}

TEST(Input_Children_Inferred)
{
  TemporaryWorkingDirectory workDir(name);

  CreateCMakeProject("BigComponent", "add_library", workDir());
  filesystem::create_directories(workDir() / "BigComponent" / "NewSubComponent");
  streams::ofstream(workDir() / "BigComponent" / "NewSubComponent" / "Source.c").close();
  filesystem::create_directories(workDir() / "NewComponent");
  streams::ofstream(workDir() / "NewComponent" / "Source.c").close();
  streams::ofstream(workDir() / "CMakeLists.txt").close();

  Configuration config;
  std::unordered_map<std::string, Component*> components;
  std::unordered_map<std::string, File> files;

  LoadFileList(config, components, files, workDir(), true, false);

  ASSERT(components.size() == 4); // 3 components + ROOT

  for (auto& pair : components) {
    const Component& comp = *pair.second;

    if (comp.CmakeName() == "BigComponent") {
      ASSERT(comp.children.size() == 1);
      ASSERT(std::count_if(comp.children.cbegin(),
                           comp.children.cend(),
                           [](const Component* c) { return c->CmakeName() == "BigComponent.NewSubComponent"; }) == 1);
    } else if (comp.CmakeName() == "BigComponent.NewSubComponent") {
      ASSERT(comp.children.size() == 0);
    } else if (comp.CmakeName() == "NewComponent") {
      ASSERT(comp.children.size() == 0);
    } else {
      ASSERT(comp.CmakeName() == "ROOT");
      ASSERT(comp.children.size() == 2);
      ASSERT(std::count_if(comp.children.cbegin(),
                           comp.children.cend(),
                           [](const Component* c) { return c->CmakeName() == "BigComponent"; }) == 1);
      ASSERT(std::count_if(comp.children.cbegin(),
                           comp.children.cend(),
                           [](const Component* c) { return c->CmakeName() == "NewComponent"; }) == 1);
    }
  }
}
