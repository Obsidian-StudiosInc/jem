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

#include "package.h"
#include "version.h"
#include "vm.h"

/**
 * java environment
 */
struct env {
    struct pkg *pkgs;       /** packages */
    struct vm *vms;         /** virtual machines */
    struct vm *active_vm;   /** pointer to the active vm struct in the virtual machines vms struct array */
};

struct env jem_env;

/**
 * Cleanup call before program exit, clean up env, free memory, etc
 */
void cleanup();

/**
 * Frees the allocated memory in a env struct
 *
 * @param params a pointer to an env struct
 */
void freeEnv(struct env *env);

/**
 * Execute something which is in JAVA_HOME
 */
void exeJavaBin(char *exe_name);

/**
 * Get active package for virtual
 *
 * @param name string containing the name of the virtual
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetActiveVirtualProvider(const char *virtual);

/**
 * Get the active VM
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct vm *getActiveVM(struct env *env);

/**
 * Get providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetVirtualProviders(const char *virtual);

/**
 * Initialize env struct
 *
 * @param env pointer to an uninitialized env structure
 */
void initEnv(struct env *env);

/**
 * Load the active VM, first by env variable. If that does not exist, by
 * looking at the symlinks, starting with user if it exists, then system.
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed! 
 */
struct vm *loadActiveVM(struct env *env);

/**
 * Print a list of the Available VMs
 */
void listAvailableVMs();

/**
 * Print a list of installed Packages
 */
void listPackages();

/**
 * Loads a installed Package env file. Storing them in an dynamically allocated
 * pkg struct array.
 *
 * @return a pkg struct. Which must be freed, including struct members!
 */
struct pkg *loadPackage(char *name);

/**
 * Loads all installed Package env files. Storing them in an dynamically allocated
 * pkg struct array.
 *
 * @return an array of pkg structs. Which must be freed, including struct members!
 */
struct pkg *loadPackages();

/**
 * Compares the names of two packages, used soley by qsort in loadPackages()
 *
 * @return an integer -1, 0, or 1.
 */

int loadPackagesCompare(const void *v1, const void *v2);

/**
 * Print the active VM
 */
void printActiveVM();

/**
 * Print the active VM parameters
 */
void printVMParams(const char *vm_name);

/**
 * Print a command including path using the active VM
 * 
 * @param exec string containing the executable to print
 */
void printExe(const char *exe);

/**
 * Print the active VM java version
 */
void printJavaVersion();

/**
 * Print one or more package classpath values from the package.env file
 *
 * @param name string containing the name(s) of the package(s), 
 *             multiple comma separated package names can be specified
 */
void printPackageClasspath(const char *name);

/**
 * Print the active VM absolute path to tools.jar
 */
void printToolsJar();

/**
 * Print one or more parameter values from the active VM config file
 *
 * @param name string containing the name(s) of the parameter(s), multiple comma separated parameter names can be specified
 */
void printValueFromActiveVM(const char *name);

/**
 * Print one or more parameter values from one or more package.env file
 *
 * @param name string containing the name(s) of the package(s), multiple package(s) separated parameter names can be specified
 * @param param string containing the parameter(s) names, multiple comma separated parameter names can be specified
 */
void printValueFromPackage(const char *name,const char *param);

/**
 * Print providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 */
void printVirtualProviders(const char *virtual);

/**
 * Set the System VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void setSystemVM(const char *vm_name);

/**
 * Set the User VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void setUserVM(const char *vm_name);
