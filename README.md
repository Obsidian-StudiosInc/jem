# jem - Java Environment Manager
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/jem/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/jem/master.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/jem)
[![Build Status](https://img.shields.io/shippable/5840e5d8112fc80f00e0cb41/master.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/5840e5d8112fc80f00e0cb41/)
[![Code Quality](https://img.shields.io/coverity/scan/12326.svg?colorA=9977bb&style=plastic)](https://scan.coverity.com/projects/obsidian-studiosinc-jem)

## About

jem is a Java Environment Manager, a re-write and port to C of a 
program on Gentoo Linux called 
[java-config](https://github.com/gentoo/java-config), written in 
python, specific to Gentoo Linux and derivatives. jem being in C, is 
Linux distro agnostic and should work for any POSIX compliant 
operating system. The intention is to standardize java management and 
tools for scripts and programs.

jem provides means to manage and set system and user virtual machines 
independently. Along with ability to build package classpath and other 
things for a variety of uses. By using jem systems can have portable  
and consistent access to java virtual machines and packages.

## Documentation:

Documentation is generated from jem sources in the docs subdirectory  
after compiling jem. API documentation is created using doxygen and man 
page created from help2man. At some point a version will be placed 
online. For time being need to generation documentation yourself as 
needed. Or part of a system package.

## Download:

jem binaries and sources are currently available for download via 
[releases](https://github.com/Obsidian-StudiosInc/jem/releases).

## Configuring:

In the root directory of the sources, run the following commands to  
configure cmake sources. cmake uses autotools by default. jem can be  
built with ninja instead of autotools for faster builds. Little chance  
of switching to meson build system.

 - To build jem with debugging symbols

```
cmake -D CMAKE_BUILD_TYPE=Debug ./
```
 - To build jem as normal

```
cmake -D CMAKE_BUILD_TYPE=Release ./
```
 - To build documentation add -D BUILD_DOC=ON to either

 - To build using ninja instead of autotools add -G Ninja to either


## Compiling:
jem can be compiled via autotools or ninja

### Autotools
After configuring via cmake, to compile jem run make as normal in the 
root directory of the sources.
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

### Ninja
After configuring via cmake, to compile jem run ninja as normal in the
root directory of the sources.
```
ninja
```
 - To compile and run tests

```
ninja jem-test
```

## Installing:

Installing jem should be done through the systems package manager. For 
integration with package managers and/or manual installation you can 
run the following command.

### Autotools
Install using autotools
```
make install
```

### Ninja
Install using ninja
```
ninja install
```

## Packaging:

jem's build system can also build DEB, RPM, and TBZ2 packages for 
distribution. To build all the above targets (they cannot be build 
individually at this time) run the following command.

### Autotools
```
  make package
```

### Ninja
```
  ninja package
```

