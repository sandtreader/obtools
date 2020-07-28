# obtools
C++ utility libraries

These libraries were started in 2003 by Paul Clark (@sandtreader) for use across multiple consultancy projects.  They became the foundation of [Packet Ship](https://packetship.com)'s streaming media products, a custom digital coin architecture, an e-commerce retail offering and more recently the [ViGraph](https://vigraph.com) creative media platform.  

Alex Woods and Jon Barber of Packet Ship also contributed significant new functionality between 2010 and 2020.

In 2020, Paul made ObTools Open Source under a permissive MIT licence.

## Functionality

The libraries (obtools/libs) include:

* XML parser
* JSON parser/writer
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

XMLMesh (obtools/xmlmesh) is an XML-based publish-subscribe system with clients / bindings for C, C++, PHP, Perl and Javascript.

The tools in obtools/obcache and obtools/tools should be considered an early experiment in automated ORM code generation in C++ direct from XMI.  If you don't know what that is, it's unlikely you will need it :-)

## Building

The libraries are in portable C++14 (using clang), primarily designed for Linux and use [Tup](http://gittup.org/tup/) to build.  A cross-compilation to Windows using MinGW is also possible.  Native build on MacOS has worked in the past, and could probably easily be added again, but isn't currently supported.  Compilation to JavaScript through emscripten is a Work In Progress!

### How to build debug versions (with automated tests):

1. Clone this repo, e.g.

        $ git clone git@github.com:sandtreader/obtools.git

2. Initialise debug build

        $ cd obtools
        $ build/init.sh -t debug

3. Build it

        $ tup

That gives you debug static libraries (.a) in the build-debug/libs directories (see note below).

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

This gives you release static libraries in build-release/libs, plus XMLMesh executables and Debian packages (if building on Debian/Ubuntu) in build-release/xmlmesh.

## Using

The build currently only produces individual static libraries (.a) rather than a combined dynamic library (.so).  This is deliberate - the libraries are small enough to not worry too much about size, are heavily reliant on templating, and I don't want the pain of maintaining a binary-compatible API!  If anyone would like to add a combined .so output on top, feel free :-)

The simplest way to use the libraries is within the same build system - see the Tupfile in xmlmesh/server for an example.  You just have to name the libraries you are directly dependent on - further dependencies within the libraries are handled automagically.

If you want to use the libraries in your own build system, you'll need to explicitly include the individual '.a's from build-debug/build-release, and include the header files from each library.  The Tupfiles in each library say what other libraries it depends on.




