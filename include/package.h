/****************************************************************************
 *  Copyright 2015 Obsidian-Studios, Inc.
 *  Author William L. Thomson Jr.
 *         wlt@o-sinc.com
 ****************************************************************************/

/*
 *  This file is part of jem.
 *  
 *  jem is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  jem is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with jem.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file_parser.h"

#define JEM_PKG_ENV "/package.env"
#define JEM_PKG_PATH JEM_USER_SHARE
#define JEM_PKG_VIRTUALS "virtuals"
#define JEM_PKG_VIRTUAL_CONFIG JEM_SYSTEM_CONFIG_PATH JEM_PKG_VIRTUALS
#define JEM_PKG_VIRTUAL_PATH JEM_PKG_PATH JEM "/" JEM_PKG_VIRTUALS "/"

extern bool jem_with_dependencies;

/**
 * java package
 */
struct pkg {
    char *filename;         /** package.env absolute file name */
    char *name;             /** package name */
    struct param *params;   /** package.env file parameters */
};

/**
 * java package dependency
 */
struct dep {
    char *name;             /** package name */
    char **jars;            /** array of names */
    bool parsed_sub_deps;
};

/**
 * Frees the allocated memory used by a dep struct
 *
 * @param dep a pointer to a dep struct
 */
void freeDep(struct dep *dep);

/**
 * Frees the allocated memory used by a pkg struct
 *
 * @param pkgs a pointer to a pkg struct
 */
void freePkg(struct pkg *pkg);

/**
 * Frees the allocated memory used by an array of pkg structs
 *
 * @param pkgs a pointer an array of pkg structs
 */
void freePkgs(struct pkg *pkgs);

/**
 * Get active package for virtual
 *
 * @param virtual string containing the name of the virtual
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetActiveVirtualProvider(const char *virtual);

/**
 * Get a package's classpath
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjpGetClasspath(struct param *params);

/**
 * Get a package's description
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjpGetDescription(struct param *params);

/**
 * Get a package's dependencies, internal function called buy wrappers
 *
 * @param params an array of param structs
 * @name string containing the variable name, DEPEND/BUILD_DEPEND/OPTIONAL_DEPEND
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *__gjpGetDeps(struct dep *deps,
                         struct param *params,
                         char *name);

/**
 * Get a package's dependencies
 *
 * @param params an array of param structs
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *gjpGetDeps(struct param *params);

/**
 * Get a package's build dependencies
 *
 * @param params an array of param structs
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *gjpGetBuildDeps(struct param *params);


/**
 * Get the packages this package provides
 *
 * @param params an array of param structs
 * @return an array of strings containing the value. The array and strings must be freed!
 */
char **gjpGetProvides(struct param *params);

/**
 * Get a package's optional dependencies
 *
 * @param params an array of param structs
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *gjpGetOptDeps(struct param *params);

/**
 * Get a package's target
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetTarget(struct param *params);

/**
 * Get providers/packages for one or more virtual package(s)
 *
 * @param virtual string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 * @param ignore_vm boolean get providers regardless if vm is a provider
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetVirtualProviders(const char *virtual,bool ignore_vm);

/**
 * Loads a installed env file into a dynamically allocated pkg struct
 *
 * @return a pkg struct. Which must be freed, including struct members!
 */
struct pkg *loadFile(char *filename, char *name);

/**
 * Loads a installed package env into a dynamically allocated pkg struct
 *
 * @return a pkg struct. Which must be freed, including struct members!
 */
struct pkg *loadPackage(char *name);

/**
 * Loads all installed package env into a dynamically allocated pkg struct array
 *
 * @param virtual boolean to control loading of virtual or package.env file
 * @return an array of pkg structs. Which must be freed, including struct members!
 */
struct pkg *loadPackages(bool virtual);

/**
 * Compares the names of two packages, used soley by qsort in loadPackages()
 *
 * @return an integer -1, 0, or 1.
 */

int loadPackagesCompare(const void *v1, const void *v2);

/**
 * Loads a installed virtual into a dynamically allocated pkg struct
 *
 * @return a pkg struct. Which must be freed, including struct members!
 */
struct pkg *loadVirtual(char *name);

