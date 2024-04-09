# ObTools
C++ utility libraries for high-performance Unix servers & applications.

These libraries were started in 2003 by Paul Clark
([@sandtreader](https://github.com/sandtreader)) as a common platform
for use across multiple consultancy projects.  They became the
foundation of [Packet Ship](https://packetship.com)'s streaming media
products, a custom digital coin architecture, an e-commerce retail
offering and more recently the [ViGraph](https://vigraph.com) creative
media platform.

Alex Woods and Jon Barber of Packet Ship also contributed significant
new functionality between 2010 and 2020.

In 2020, Paul made ObTools Open Source under a permissive MIT licence.

## Functionality

The libraries (`obtools/libs`) include:

* XML parser
* JSON parser/writer with CBOR encoder
* Multithreading support
* Crypto sugaring over OpenSSL
* Common database interface for MySQL/MariaDB, PostgreSQL and SQLite
* Multi-channel binary protocol/file packer/unpacker
* IP networking and SSL
* HTTP clients and servers
* SOAP clients and servers
* Framework for Linux daemons
* Timestamp manipulation
* Logging
* ... and lots of other handy stuff

XMLMesh (`obtools/xmlmesh`) is an XML-based publish-subscribe system
with clients / bindings for C, C++, PHP, Perl and Javascript.

The tools in `obtools/obcache` and `obtools/tools` should be
considered an early experiment in automated ORM code generation in C++
direct from XMI.  If you don't know what that is, it's unlikely you
will need it 😀

## Building

The libraries are in portable C++14 (using `clang`), primarily
designed for Linux and use [Tup](http://gittup.org/tup/) to build.  A
cross-compilation to Windows using MinGW is also possible.  Native
build on MacOS has worked in the past, and could probably easily be
added again, but isn't currently supported.  Compilation to JavaScript
through emscripten is a Work In Progress!

The libraries also have a basic Makefile, which may simplify your CI system.

### Dependencies

To build ObTools you'll need the following packages on top of the standard Ubuntu 20.04 or Debian 10 install:

       $ sudo apt install build-essential tup clang git pkg-config debhelper dh-exec
       $ sudo apt install libssl-dev libsqlite3-dev libmysqlclient-dev libpq-dev libnl-genl-3-dev

If using another or older distribution which doesn't have Tup, you can build it yourself from the [Tup sources](http://gittup.org/tup/).  Why Tup?  It's blindingly fast and intelligent - I used to have a set of arcane [recursive Makefiles](https://www.cse.iitb.ac.in/~soumen/teach/1999.2A.CS699/make.html) which took ages and you could never quite trust for deep library changes - not any more!

If you have a newer distribution with MariaDB instead of MySql, you'll need `libmariadb-dev-compat` instead of `libmysqlclient-dev` in the above.

### Installing gtest

The unit tests in ObTools use GTest, which you need to build yourself:

        $ sudo apt install libgtest-dev cmake
        $ cd /usr/src/gtest
        $ sudo cmake CMakeLists.txt
        $ sudo make
        $ sudo cp lib/*.a /usr/lib/

### How to build debug versions (with automated tests):

1. Clone this repo, e.g.

        $ git clone git@github.com:sandtreader/obtools.git

2. Initialise debug build

        $ cd obtools
        $ build/init.sh -t debug

3. Build it

        $ tup

That gives you debug static libraries (.a) in the `build-debug/libs` directories (see note below).

### How to build release versions

1. Clone this repo, e.g.

        $ git clone git@github.com:sandtreader/obtools.git

2. Initialise release build

        $ cd obtools
        $ build/init.sh -t release

    (ignore the warning about tup database already existing)

3. Build it

        $ tup

(you can combine the two and build both together if you want)

This gives you release static libraries in `build-release/libs`, plus XMLMesh executables and Debian packages (if building on Debian/Ubuntu) in `build-release/xmlmesh`.

### Simple library release build with make

1. Clone this repo, e.g.

        $ git clone git@github.com:sandtreader/obtools.git

2. Make the libraries

        $ cd obtools/libs
        $ make

This will give you libraries (`ot-xxx.a`) in `libs/build/lib`
and headers (`ot-xxx.h`) in `libs/build/include`.

## Using

The build only produces individual static libraries (`.a`) rather than
a combined dynamic library (`.so`).  This is deliberate - the
libraries are small enough to not worry too much about duplicated code
size, are heavily reliant on templating, and I don't want the pain of
maintaining a binary-compatible API!

The simplest way to use the libraries is within the same build
system - see the Tupfile in `xmlmesh/server` for an example.  You just
have to name the libraries you are directly dependent on - further
dependencies within the libraries are handled automagically.

If you're using it in a separate project, you probably want to make
ObTools a sub-module, then run `obtools/build/init.sh/` at your
top-level.  You'll need to provide a `Tuppath.lua` at the top level to
direct the build system to the right paths for both the ObTools
libraries and any of your own which are used as dependencies.  You'll
probably want to grab ObTools' `.gitignore` too.

       $ cd myproject
       $ git submodule add git@github.com:sandtreader/obtools.git
       $ vi Tuppath.lua

       function get_dependency_path(name)
         if name == 'ot-xmlmesh-core' then
           return 'obtools/xmlmesh/core'
         elseif name == 'ot-xmlmesh-send' then
           return 'obtools/xmlmesh/bindings/cli/send'
         elseif name == 'ot-xmlmesh-receive' then
           return 'obtools/xmlmesh/bindings/cli/receive'
         elseif string.sub(name, 1, 11) == 'ot-xmlmesh-' then
           return 'obtools/xmlmesh/' .. string.sub(name, 12)
         elseif name == 'ot-toolgen' then
           return 'obtools/tools/toolgen'
         elseif string.sub(name, 1, 11) == 'ot-obcache-' then
           return 'obtools/obcache/libs/' .. string.sub(name, 12)
         elseif string.sub(name, 1, 3) == 'ot-' then
           return 'obtools/libs/' .. string.sub(name, 4)

         -- Add your own name to path matches here --

         end
         return nil
       end

       $ obtools/build/init.sh -t debug
       $ tup
          --- should build successfully! ---
       $ cp obtools/.gitignore .
       $ git add .gitignore Tuppath.lua

You can see this all in operation in the [ViGraph server
source](https://github.com/vigraph/vg-server)

If you want to use the libraries in your own build system, you'll need
to explicitly link with the individual `ot-xxx.a`'s from
`build-debug/libs/xxx` or `build-release/libs/xxx`, and include the
`ot-xxx.h` header files from each library.  The Tupfiles in each
library say what other libraries it depends on.

## Contributions

Yes please!

If it's a bug-fix, test or tidy, please just go ahead and send a PR.  If it's anything major, please discuss it with me first...

I ask all contributors to sign a standard, FSF-approved [Contributor License Agreement](http://contributoragreements.org/) to make the project easier to manage.  You can sign it when you generate a PR, or in advance [here](https://cla-assistant.io/sandtreader/obtools).  

Thanks!



