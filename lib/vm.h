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

#define BASE_NAME_SIZE 128

#define SYSTEM_CONFIG_PATH "/etc/java-config-2/"
#define SYSTEM_VM_LINK "/etc/java-config-2/current-system-vm"
#define USER_VM_LINK_SUFFIX ".java/current-user-vm"
#define VMS_PATH "/usr/share/java-config-2/vm"

/**
 * java virtual machine
 */
struct vm {
    char *filename;         /** config file absolute name */
    struct param *params;   /** config file parameters */
};

/**
 * Frees the allocated memory used by an array of vm structs
 *
 * @param pkgs a pointer an array of vm structs
 */
void freeVMs(struct vm *vms);

/**
 * Get the path to an executable by name
 *
 * @param params an array of param structs
 * @param exec name of the executables path to get
 * @return a string containing the value. The string must be freed!
 */
char *gjvmGetExec(struct param *params,const char *exec);

/**
 * Get the name of the vm
 *
 * @param vm a pointer to a vm struct
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetName(struct vm *vm);

/**
 * Get the types the vm provides
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetProvidesType(struct param *params);

/**
 * Get the versions of the vm provides
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetProvidesVersion(struct param *params);

/**
 * Get the versions the vm
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetVersion(struct param *params);

/**
 * Check to see if the vm is build only
 *
 * @return true if build only, false otherwise
 */
bool gjvmIsBuildOnly(struct param *params);

/**
 * Check to see if the vm is/provides a particular type
 *
 * @param type the type of vm
 * @return true if the vm is/provides a particular type , false otherwise
 */
bool gjvmIsType(struct param *params,const char *type);

/**
 * Check to see if the vm is a JDK
 *
 * @return true if the vm is a JDK, false otherwise
 */
bool gjvmIsJDK(struct param *params);
/**
 * Check to see if the vm is a JRE
 *
 * @return true if the vm is a JRE, false otherwise
 */
bool gjvmIsJRE(struct param *params);

int gjvmProvides(struct param *params,
                 char **virtuals);

/**
 * Frees the allocated memory used by VM links in string pointer array
 *
 * @param vm_links a pointer an array of strings
 */
void freeVMLinks(char **vm_links);

/**
 * Get the system VM's link
 *
 * @return a string containing the value. The string must NOT be freed!
 */
char *getSystemVMLink();

/**
 * Get the system VM's name by returning the basename of the system VM link
 *
 * @return a string containing the value. The string must be freed!
 */
char *getSystemVMName();

/**
 * Get the user VM's link
 *
 * @return a string containing the value. The string must be freed!
 */
char *getUserVMLink();

/**
 * Get a VM by index, filename, VM name including partial match, or JAVA_HOME
 * from an array of VM structs
 *
 * @param vms array of vm structs
 * @param file the name of the file to parse
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct vm *getVM(struct vm *vms,const char *vm_name);

/**
 * Get user and system VM links
 *
 * @return a pointer to an array of strings containing the user and/or system
 * vm links, or null if not found. Array and links must be freed!
 */
char **getVMLinks();

/**
 * Compares the filenames of two vms, used soley by qsort in loadVMs()
 *
 * @return an integer -1, 0, or 1.
 */

int compareVMs(const struct vm *vm1, const struct vm *vm2);

/**
 * Loads all installed VMs config files. Storing them in an dynamically allocated
 * vm struct array.
 *
 * @return an array of vm structs. Which must be freed, including struct members!
 */
struct vm *loadVMs();

/**
 * Set the VM, create a symlink for the given vm to target
 *
 * @param vm pointer to an vm struct
 * @param target a string representing the vm symlink target
 * @return an array of vm structs. Which must be freed, including struct members!
 */
bool setVM(struct vm *vm,const char *target);