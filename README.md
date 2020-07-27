# obtools
C++ utility libraries

These libraries were started in 2003 by Paul Clark (@sandtreader) for use across multiple consultancy projects.  They became the foundation of [Packet Ship](https://packetship.com)'s streaming media products, a custom digital coin architecture, an e-commerce retail offering and more recently the [ViGraph](https://vigraph.com) creative media platform.  

Alex Woods and Jon Barber of Packet Ship also contributed significant new functionality between 2010 and 2020.  In 2020, Paul made ObTools Open Source under a permissive MIT licence.

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

The tools in obtools/tools should be considered an early experiment in automated ORM code generation in C++ direct from XMI.  If you don't know what that is, it's unlikely you will need it :-)

## Building

The libraries are in portable C++14 (using clang), primarily designed for Linux and use [Tup](http://gittup.org/tup/) to build.  A cross-compilation to Windows using MinGW is also possible.  Native build on MacOS has worked in the past, and could probably easily be added again, but isn't currently supported.  Compilation to JavaScript through emscripten is a Work In Progress!





