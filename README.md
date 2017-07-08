# jem - Java Environment Manager
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/jem/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/jem/master.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/jem)
[![Build Status](https://img.shields.io/shippable/5840e5d8112fc80f00e0cb41/master.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/5840e5d8112fc80f00e0cb41/)
[![Code Quality](https://img.shields.io/coverity/scan/12326.svg?colorA=9977bb&style=plastic)](https://scan.coverity.com/projects/obsidian-studiosinc-jem)

These are the release notes for jem. Read them carefully,
as they tell you what this is all about, explain how to install jem,
and what to do if something goes wrong.

## WHAT IS JEM?

  jem is a java environment manager, a re-write and port to C of 
  a program on Gentoo Linux called java-config, written in python and
  specific to Gentoo Linux and derivatives. jem being in C, is Linux 
  distro agnostic and should work for any POSIX compliant operating 
  system. The intention is to standardize java management and tools for 
  scripts and programs.

  jem provides means to manage and set system and user virtual machines 
  independently. Along with ability to build package classpath and other 
  things for a variety of uses. By using jem systems can have portable 
  and consistent access to java virtual machines and packages.

## DOCUMENTATION:

  Documentation is generated from jem sources in the docs subdirectory 
  after compiling jem. API documentation is created using doxygen and 
  man page created from help2man.

## DOWNLOAD:

  jem binaries and sources are currently available for download at
  http://o-sinc.com/#/software/jem

## CONFIGURING:

  In the root directory of the sources, run the following commands to 
  configure cmake sources.

 - To build jem with debugging symbols

```
cmake -D CMAKE_BUILD_TYPE=Debug ./
```
 - To build jem as normal

```
cmake -D CMAKE_BUILD_TYPE=Release ./
```
 - To build documentation add -D BUILD_DOC=ON to either

## COMPILING:

 - After configuring via cmake, to compile jem run make as normal in 
   the root directory of the sources.
```
   make
```
 - To see all make targets

```
make help
``` 
 - To compile and run tests

```
make jem-test
```

## INSTALLING:

  Installing jem should be done through the systems package manager. For 
  integration with package managers and/or manual installation you can 
  run the following command.

  make install

## PACKAGING:

  jem's build system can also build DEB, RPM, and TBZ2 packages for 
  distribution. To build all the above targets (they cannot be build 
  individually at this time) run the following command.

  make package

