--==========================================================================
-- Tuprules.lua for ObTools build system
--
-- Copyright (c) 2017 Paul Clark. All rights reserved
--==========================================================================

-- Notes for those unfamiliar with Lua/Tup
-- Variables coming in from the Tupfiles are all Lua tables
-- Tup adds the += operator which is like push_back() for Lua tables
-- Lua parameters are all by value, but objects and tables are like pointers
-- Lua string comparison by == is effectively a comparison of the values of the
-- strings

--==========================================================================
-- Guard against being included again
if done ~= nil then
  return
end

--==========================================================================
-- Include project dependency paths
tup.include("Tuppath.lua")

--==========================================================================
-- Set up global variables

----------------------------------------------------------------------------
-- All the things set in Tupfiles will be tables, and these need squishing
function flatten(t)
  if t then
    return table.concat(t, " ")
  else
    return ""
  end
end
local NAME = flatten(NAME)
local TYPE = flatten(TYPE)
local PACKAGE = flatten(PACKAGE)
local VERSION = flatten(VERSION)
local REVISION = flatten(REVISION)
local CFLAGS = flatten(CFLAGS)
local LFLAGS = flatten(LFLAGS)

local PLATFORM = tup.getconfig("PLATFORM")
local ARCH = tup.getconfig("ARCH")
local CPPSTD = tup.getconfig("CPPSTD")
if CPPSTD == nil or CPPSTD == "" then
  CPPSTD = 14
end

----------------------------------------------------------------------------
-- If platforms are specified, then ensure configured platform is in list
if PLATFORMS ~= nil then
  local found = false
  for x, value in ipairs(PLATFORMS) do
    if value == PLATFORM then
      found = true
    end
  end
  if found == false then
    return
  end
end

----------------------------------------------------------------------------
-- Add platform specific dependencies into main table
local platform_depends = _G[PLATFORM:upper() .. "-DEPENDS"]
if platform_depends ~= nil
then
  if DEPENDS == nil
  then
    DEPENDS = {}
  end
  for k, v in pairs(_G[PLATFORM:upper() .. "-DEPENDS"])
  do
    table.insert(DEPENDS, v)
  end
end

----------------------------------------------------------------------------
-- Tools

if PLATFORM == "linux" then
  COMPILER = "clang++"
  ARCHIVER = "ar"
  LINKER = COMPILER
  PLATFORM_CFLAGS = "-DPLATFORM_LINUX"
  PLATFORM_LFLAGS = ""
  PLATFORM_LIB_EXT = ".a"
  PLATFORM_SHARED_FLAGS = "-rdynamic"
  PLATFORM_SHARED_EXT = ".so"
  PLATFORM_EXE_EXT = ""
  PACKAGEDIR = "DEBIAN"
elseif PLATFORM == "windows" then
  COMPILER = "x86_64-w64-mingw32-g++"
  ARCHIVER = "x86_64-w64-mingw32-ar"
  WINDRES = "x86_64-w64-mingw32-windres"
  LINKER = COMPILER
  PLATFORM_CFLAGS = "-DPLATFORM_WINDOWS -DWIN32_LEAN_AND_MEAN -D_GLIBCXX_HAVE_TLS"
  PLATFORM_LFLAGS = ""
  PLATFORM_LIB_EXT = ".a"
  PLATFORM_SHARED_FLAGS = ""
  PLATFORM_SHARED_EXT = ".dll"
  PLATFORM_EXE_EXT = ".exe"
  PACKAGEDIR = "WINDOWS"
else
  error("Unhandled platform: '" .. PLATFORM .. "'")
end

----------------------------------------------------------------------------
-- Add platform specific flags
PLATFORM_CFLAGS = PLATFORM_CFLAGS .. " "
                  .. flatten(_G[PLATFORM:upper() .. "-CFLAGS"])
PLATFORM_LFLAGS = PLATFORM_LFLAGS .. " "
                  .. flatten(_G[PLATFORM:upper() .. "-LFLAGS"])

----------------------------------------------------------------------------
-- Basic compiler options
CFLAGS = CFLAGS .. " --std=c++" .. CPPSTD ..
                " -pedantic -Wall -Wextra -Werror -fPIC "
                .. PLATFORM_CFLAGS
LFLAGS = LFLAGS .. " --std=c++" .. CPPSTD .. " " .. PLATFORM_LFLAGS

----------------------------------------------------------------------------
-- Debug settings
if tup.getconfig("DEBUG") == "y" then
  CFLAGS = CFLAGS .. " -ggdb3 -DDEBUG -D_GLIBCXX_DEBUG"
end

----------------------------------------------------------------------------
-- Thread safety settings
if tup.getconfig("DEBUGTHREAD") == "y" then
  CFLAGS = CFLAGS .. " -ggdb3 -O2 -fsanitize=thread"
  LFLAGS = LFLAGS .. " -fsanitize=thread"
end

----------------------------------------------------------------------------
-- Profiling settings
if tup.getconfig("PROFILE") == "y" then
  CFLAGS = CFLAGS .. " -ggdb3 -O2"
end

----------------------------------------------------------------------------
-- Release settings
if tup.getconfig("RELEASE") == "y" then
  CFLAGS = CFLAGS .. " -O2"
  LFLAGS = LFLAGS .. " -s"
end

----------------------------------------------------------------------------
-- Version
if VERSION then
  CFLAGS = CFLAGS .. " -DVERSION='\"" .. VERSION
  if tup.getconfig("DEBUG") == "y" then
    CFLAGS = CFLAGS .. " (debug)"
  elseif tup.getconfig("PROFILE") == "y" then
    CFLAGS = CFLAGS .. " (profile)"
  end
  CFLAGS = CFLAGS .. "\"'"
end

--==========================================================================
-- Sort source files into groups
local sources = {}
local test_sources = {}

function add_sources_for_dir(dir, sources, test_sources)
  local glob = '*.cc'
  if dir ~= "." then
    glob = dir .. '/*.cc'
  end
  for index, filename in ipairs(tup.glob(glob)) do
    local basename = filename:match("([^/]+).cc$")
    if basename:sub(1, 1) ~= "." then
      if basename:sub(1, 5) == "test-" then
        test_sources += filename
      elseif basename:sub(1, 7) ~= "legacy-" then
        if basename ~= "main" then
          sources += filename
        end
      end
    end
  end
end

add_sources_for_dir(".", sources, test_sources)
if SUBDIRS then
  for index, dir in ipairs(SUBDIRS) do
    add_sources_for_dir(dir, sources, test_sources)
  end
end

--==========================================================================
-- Check for windows resources
local windows_resources = {}

if PLATFORM == "windows" then
  local glob = '*.rc'
  for index, filename in ipairs(tup.glob(glob)) do
    windows_resources += filename
  end
end

--==========================================================================
-- Calculate the complete dependency list for this pacakge
local full_depends = {};

function get_deps(name, depends, platform)
  if depends[name] == nil then
    depends[name] = {}
    local libdir = get_dependency_path(name)
    if libdir ~= nil then
      local f = io.open(libdir .. "/Tupfile", "r")
      if f == nil then
        error("Could not open Tupfile in " .. libdir .. " required by " .. name)
      end
      depends[name]["dir"] = libdir
      local cont = false
      for line in f:lines() do
        if cont then
          for n in line:gmatch("([^\\ ]+)") do
            get_deps(n, depends, platform)
          end
          if line:sub(-1) ~= "\\" then
            cont = false
          end
        else
          line = line:gsub("^" .. platform:upper() .. "%-", "")
          local d = line:match("^DEPENDS[ ]*=(.*)")
          if d ~= nil then
            for n in d:gmatch("([^\\ ]+)") do
              get_deps(n, depends, platform)
            end
            if d:sub(-1) == "\\" then
              cont = true
            end
          else
            local t = line:match("^TYPE[ ]*=[ ]*(.*)")
            if t ~= nil then
              depends[name]["type"] = t
            end
          end
        end
      end
      f:close()
    end
  end
end

if DEPENDS then
  if not (next(DEPENDS) == nil) then
    for index, dep in pairs(DEPENDS) do
      get_deps(dep, full_depends, PLATFORM)
    end
  end
end

-- sort the full dependencies so they product a consistant order for command
-- line options (inconsistent order causes un-necessary rebuilding)
local full_deps = {}
for dep, value in pairs(full_depends) do
  table.insert(full_deps, dep)
end
table.sort(full_deps)

-- Group the various types of dependency
local dep_includes = {}
local ext_includes = {}
local dep_static_libs = {}
local dep_shared_libs = {}
local ext_shared_libs = {}
local dep_products = {}
for index, dep in ipairs(full_deps) do
  if string.sub(dep, 1, 4) == "ext-" then
    if string.sub(dep, 5, 8) == "pkg-" then
      -- get info from pkg-config
      local pkg = string.sub(dep, 9)
      ext_includes += "`pkg-config --cflags-only-I " .. pkg .. "`"
      ext_shared_libs += "`pkg-config --libs " .. pkg .. "`"
    else
      -- external, so just link libs
      ext_shared_libs += "-l" .. string.sub(dep, 5)
    end
  elseif full_depends[dep]["dir"] then
    local libdir = tup.getcwd() .. "/" .. full_depends[dep]["dir"]
    dep_includes += "-I" .. libdir
    if full_depends[dep]["type"] == "lib" then
      dep_static_libs += libdir .. "/" .. dep .. PLATFORM_LIB_EXT
    elseif full_depends[dep]["type"] == "shared" then
      dep_shared_libs += libdir .. "/" .. dep .. PLATFORM_SHARED_EXT
      dep_products += libdir .. "/" .. dep .. PLATFORM_SHARED_EXT
    elseif full_depends[dep]["type"] == "exe" then
      dep_products += libdir .. "/" .. dep .. PLATFORM_EXE_EXT
    end
  end
end

--==========================================================================
-- Functions that wrap creation of tup rules

----------------------------------------------------------------------------
-- Compile a source file
function compile(source, dep_includes, ext_includes)
  local output = source:gsub("%.(.-)$", ".o")
  tup.definerule{
    inputs = {source},
    command = "^ CC %f^ " .. COMPILER .. " " .. CFLAGS .. " " ..
              table.concat(dep_includes, " ") .. " " ..
              table.concat(ext_includes, " ") ..
              " -c " .. source ..  " -o " .. output,
    outputs = {output}
  }
  return output
end

----------------------------------------------------------------------------
-- Build a windows resource
function windres(resource)
  local output = resource:gsub("%.(.-)$", ".res")
  tup.definerule{
    inputs = {resource},
    command = "^ WINDRES %f^ " .. WINDRES .. " " .. resource ..
              " -O coff -o " .. output,
    outputs = {output}
  }
  return output
end

----------------------------------------------------------------------------
-- Link a static library
function link_static_lib(name, objects)
  local output = name .. PLATFORM_LIB_EXT
  tup.definerule{
    inputs = objects,
    command = "^ AR %o^ " .. ARCHIVER .. " crs " .. output .. " " ..
              table.concat(objects, " "),
    outputs = {output}
  }
  return output
end

----------------------------------------------------------------------------
-- Link a shared library
function link_shared_lib(name, objects, dep_static_libs, dep_shared_libs,
                         ext_shared_libs)
  local output = name .. PLATFORM_SHARED_EXT
  local inputs = {}
  local opts = " -shared " .. PLATFORM_SHARED_FLAGS
  inputs += objects
  inputs += dep_static_libs
  inputs += dep_shared_libs
  tup.definerule{
    inputs = inputs,
    command = "^ LINK %o^ " .. LINKER .. " " .. LFLAGS .. " " ..
              table.concat(objects, " ") ..
              " -Wl,-\\( " .. table.concat(dep_static_libs, " ") ..
              " -Wl,-\\)" ..
              " -Wl,--as-needed " ..  table.concat(ext_shared_libs, " ") ..
              opts .. " -o " .. output,
    outputs = {output}
  }
  return output
end

----------------------------------------------------------------------------
-- Link an executable
function link_executable(name, objects, dep_static_libs, dep_shared_libs,
                         ext_shared_libs)
  local output = name .. PLATFORM_EXE_EXT
  local inputs = {}
  inputs += objects
  inputs += dep_static_libs
  inputs += dep_shared_libs
  tup.definerule{
    inputs = inputs,
    command = "^ LINK %o^ " .. LINKER .. " " .. LFLAGS .. " " ..
              table.concat(objects, " ") .. " " ..
              " -Wl,-\\( " .. table.concat(dep_static_libs, " ") ..
              " -Wl,-\\)" ..
              " -Wl,--as-needed " .. table.concat(ext_shared_libs, " ") ..
              " -o " .. output,
    outputs = {output}
  }
  return output
end

----------------------------------------------------------------------------
-- Run a test
function run_test(name, test_products, dep_shared_libs)
  local inputs = {name}
  inputs += test_products
  inputs += dep_shared_libs
  tup.definerule{
    inputs = inputs,
    command = "^ TEST " .. name .. "^ ./" .. name,
    outputs = {}
  }
end

----------------------------------------------------------------------------
-- Package (Linux)
function package_linux(products, package_dir, dep_products)
  local inputs = tup.glob(PACKAGEDIR .. "/*")
  inputs += products
  inputs += dep_products
  local outputs = {}

  local suffix = ".deb"
  local noarch = "all"
  local version_separator = "_"
  local arch_separator = "_"

  local f = io.popen("lsb_release -s -i")
  local distro = string.gsub(f:read("*a"),"\n","")
  f:close()
  if distro == "CentOS" then
    suffix = ".rpm"
    noarch = "noarch"
    version_separator = "-"
    arch_separator = "."
    if ARCH == "amd64" then
      ARCH = "x86_64"
    end
  end

  -- Read Debian control file to get a list of package names
  local f = io.open(package_dir .. "/control", "r")
  if f == nil then
    error("Could not open Debian control file in " .. package_dir)
  end
  local package_names = {}
  local last_name = ""
  for line in f:lines() do
    local p = line:match("Package: (.*)")
    if p ~= nil then
      package_names += p
      last_name = p
    end
    local a = line:match("Architecture: (.*)")
    if a ~= nil then
      if a == "all" then
        outputs += last_name .. version_separator .. VERSION .. "-" .. REVISION ..
                   arch_separator .. noarch .. suffix
      else
        outputs += last_name .. version_separator .. VERSION .. "-" .. REVISION ..
                   arch_separator ..  ARCH .. suffix
      end
    end
  end

  tup.definerule{
    inputs = inputs,
    command = "^ PACKAGE %o^ " .. tup.getcwd() .. "/create-deb.sh " ..
              VERSION .. " " .. REVISION .. " " ..
              table.concat(package_names, " "),
    outputs = outputs
  }
end

----------------------------------------------------------------------------
-- Package (Windows)
function package_windows(products, package_dir, dep_products)
  local inputs = tup.glob(PACKAGEDIR .. "/*")
  inputs += products
  inputs += dep_products
  local name = NAME

  local f = io.open(package_dir .. "/control", "r")
  if f ~= nil then
    for line in f:lines() do
      n = line:match("^APPNAME=\"(.*)\"", 1)
      if n then
        name = n:gsub(" ", "-")
      end
    end
  end

  local output = name .. "-" .. VERSION .. "-" .. REVISION .. "-" ..
                 "installer.exe"

  local f = io.open(package_dir .. "/files", "r")
  if f ~= nil then
    for line in f:lines() do
      i = line:match("[^ ]+", 1)
      inputs += i
    end
  end

  tup.definerule{
    inputs = inputs,
    command = "^ PACKAGE %o^ " .. tup.getcwd() .. "/create-nsis.sh " ..
              VERSION .. " " .. REVISION .. " " .. output,
    outputs = {output}
  }
end

if PLATFORM == "linux" then
  package = package_linux
elseif PLATFORM == "windows" then
  package = package_windows
end

--==========================================================================
-- It's time to actually do some building

----------------------------------------------------------------------------
-- Compile sources
local objects = {};
for index, source in pairs(sources) do
  local output = tup.base(source)
  objects += compile(source, dep_includes, ext_includes)
end

----------------------------------------------------------------------------
-- Build windows resources
for index, resource in pairs(windows_resources) do
  local output = tup.base(resource)
  objects += windres(resource)
end

----------------------------------------------------------------------------
-- Link products
local products = {};
if TYPE == "lib" then
  products += link_static_lib(NAME, objects)
elseif TYPE == "shared" then
  products += link_shared_lib(NAME, objects, dep_static_libs, dep_shared_libs,
                              ext_shared_libs)
elseif TYPE == "exe" then
  local exe_objects = {}
  exe_objects += objects
  exe_objects += EXTRAOBJS
  exe_objects += compile("main.cc", dep_includes, ext_includes)
  products += link_executable(NAME, exe_objects, dep_static_libs,
                              dep_shared_libs, ext_shared_libs)
end

----------------------------------------------------------------------------
-- Run unit tests
if tup.getconfig("TEST") == "y" then
  local test_ext_shared_libs = ext_shared_libs
  test_ext_shared_libs += "-lgtest -lpthread"
  for index, test in pairs(test_sources) do
    local test_name = tup.base(test)
    test_object = compile(test, dep_includes, ext_includes)
    local test_objects = {test_object}
    if TYPE == "lib" then
      -- Optimisation because linking against .a is quicker than multiple .o
      -- files
      test_objects += NAME .. PLATFORM_LIB_EXT
    else
      test_objects += objects
    end
    link_executable(test_name, test_objects, dep_static_libs, dep_shared_libs,
                    test_ext_shared_libs)
    local test_products = {}
    if TYPE == "shared" then
      test_products += NAME .. PLATFORM_SHARED_EXT
    elseif TYPE == "exe" then
      test_products += NAME
    end
    run_test(test_name, test_products, dep_shared_libs)
  end
end

----------------------------------------------------------------------------
-- Make packages
if tup.getconfig("RELEASE") == "y" then
  local package_dir = tup.getprocessingdir() .. "/" .. PACKAGEDIR
  local d = io.open(package_dir, "r")
  if d ~= nil then
    package(products, package_dir, dep_products)
  end
end

--==========================================================================
-- Set guard against further inclusion
done = 1
