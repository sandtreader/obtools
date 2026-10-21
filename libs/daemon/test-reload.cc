//==========================================================================
// ObTools::Daemon: test-reload.cc
//
// GTest harness for reloading configuration (SIGHUP)
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-daemon.h"
#include "ot-file.h"
#include <gtest/gtest.h>
#include <fstream>

namespace {

using namespace std;
using namespace ObTools;

// Application recording the config it was given
class TestApplication: public Daemon::Application
{
public:
  int reads{0};
  int reconfigures{0};
  string main_value;
  string fragment_value;

  void read_config(const XML::Configuration& config) override
  {
    reads++;
    main_value = config.get_value("main/@value");
    fragment_value = config.get_value("fragment/@value");
  }

  void reconfigure() override { reconfigures++; }
};

class ReloadTest: public ::testing::Test
{
protected:
  const File::Directory dir{"/tmp/ot-daemon-test-reload"};
  const File::Path main_file{dir, "test.cfg.xml"};
  const File::Directory fragments{dir, "test.cfg.xml.d"};

  void write(const File::Path& path, const string& xml)
  {
    ofstream f(path.str());
    f << xml;
  }

  void SetUp() override
  {
    dir.ensure(true);
    fragments.ensure(true);
    write(main_file,
          "<test>\n"
          "  <main value='main-1'/>\n"
          "  <include file='test.cfg.xml.d/*.cfg.xml'/>\n"
          "</test>\n");
    write(File::Path(fragments, "010-fragment.cfg.xml"),
          "<test><fragment value='fragment-1'/></test>\n");
  }

  void TearDown() override
  {
    dir.erase();
  }
};

TEST_F(ReloadTest, TestReloadReadsIncludes)
{
  TestApplication app;
  Daemon::Shell shell(app, "test", "1.0", main_file.str(), "test", "", "");
  shell.config.add_file(main_file.str());

  // Change both files, then reload as SIGHUP does
  write(main_file,
        "<test>\n"
        "  <main value='main-2'/>\n"
        "  <include file='test.cfg.xml.d/*.cfg.xml'/>\n"
        "</test>\n");
  write(File::Path(fragments, "010-fragment.cfg.xml"),
        "<test><fragment value='fragment-2'/></test>\n");
  shell.reload();

  EXPECT_EQ(1, app.reads);
  EXPECT_EQ(1, app.reconfigures);
  EXPECT_EQ("main-2", app.main_value);
  EXPECT_EQ("fragment-2", app.fragment_value);
}

TEST_F(ReloadTest, TestReloadFailureKeepsConfig)
{
  TestApplication app;
  Daemon::Shell shell(app, "test", "1.0", main_file.str(), "test", "", "");
  shell.config.add_file(main_file.str());

  write(main_file, "<test><unclosed></test>\n");
  shell.reload();

  // Not re-read, but still reconfigured
  EXPECT_EQ(0, app.reads);
  EXPECT_EQ(1, app.reconfigures);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
