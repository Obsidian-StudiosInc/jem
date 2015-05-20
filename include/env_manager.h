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
struct jem_env {
    struct jem_pkg *pkgs;       /** packages */
    struct jem_vm *vms;         /** virtual machines */
    struct jem_vm *active_vm;   /** pointer to the active vm struct in the virtual machines vms struct array */
};

struct jem_env jem_env;

/**
 * Cleanup call before program exit, clean up env, free memory, etc
 */
void jemCleanup();

/**
 * Frees the allocated memory in a env struct
 *
 * @param params a pointer to an env struct
 */
void jemFreeEnv(struct jem_env *env);

/**
 * Execute something which is in JAVA_HOME
 */
void jemExeJavaBin(char *exe_name);

/**
 * Get the active VM
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct jem_vm *jemGetActiveVM(struct jem_env *env);

/**
 * Get vms matching name
 *
 * @param name a string name of a VM
 * @return a jem_vm struct pointer array containing the value. The pointer 
 *         array must be freed, but NOT the VM array elements
 */
struct jem_vm **jemFindVM(char *name);

/**
 * Initialize env struct
 *
 * @param env pointer to an uninitialized env structure
 */
void jemInitEnv(struct jem_env *env);

/**
 * Load the active VM, first by env variable. If that does not exist, by
 * looking at the symlinks, starting with user if it exists, then system.
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed! 
 */
struct jem_vm *jemLoadActiveVM(struct jem_env *env);

/**
 * Print a list of the Available VMs
 */
void jemListAvailableVMs();

/**
 * Print a list of installed Packages
 */
void jemListPackages();

/**
 * Print the active VM
 */
void jemPrintActiveVM();

/**
 * Print the active VM parameters
 */
void jemPrintVMParams(const char *vm_name);

/**
 * Print a command including path using the active VM
 * 
 * @param exec string containing the executable to print
 */
void jemPrintExe(const char *exe);

/**
 * Print the active VM java version
 */
void jemPrintJavaVersion();

/**
 * Print one or more package classpath values from the package.env file
 *
 * @param name string containing the name(s) of the package(s), 
 *             multiple comma separated package names can be specified
 */
void jemPrintPackageClasspath(const char *name);

/**
 * Print the active VM absolute path to tools.jar
 */
void jemPrintToolsJar();

/**
 * Print one or more parameter values from the active VM config file
 *
 * @param name string containing the name(s) of the parameter(s), multiple comma separated parameter names can be specified
 */
void jemPrintValueFromActiveVM(const char *name);

/**
 * Print one or more parameter values from one or more package.env file
 *
 * @param name string containing the name(s) of the package(s), multiple package(s) separated parameter names can be specified
 * @param param string containing the parameter(s) names, multiple comma separated parameter names can be specified
 */
void jemPrintValueFromPackage(const char *name,const char *param);

/**
 * Print providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 */
void jemPrintVirtualProviders(const char *virtual);

/**
 * Set the System VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void jemSetSystemVM(const char *vm_name);

/**
 * Set the User VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void jemSetUserVM(const char *vm_name);
