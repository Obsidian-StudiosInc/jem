/****************************************************************************
 *  Copyright 2015-2018 Obsidian-Studios, Inc.
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

#define JEM_BASE_NAME_SIZE 128

#define JEM "jem"
#define JEM_SYSTEM_CONFIG_PATH "/etc/" JEM "/"
#define JEM_SYSTEM_VM_LINK JEM_SYSTEM_CONFIG_PATH "vm"
#define JEM_USER_SHARE "/usr/share/"
#define JEM_USER_VM_LINK_SUFFIX ".java/vm"
#define JEM_VMS_PATH JEM_SYSTEM_CONFIG_PATH "vms.d"

/**
 * java virtual machine
 */
struct jem_vm {
    char *filename;         /** config file absolute name */
    struct jem_param *params;   /** config file parameters */
};

/**
 * Frees the allocated memory used by an array of vm structs
 *
 * @param pkgs a pointer an array of vm structs
 */
void jemFreeVMs(struct jem_vm *vms);

/**
 * Get the path to an executable by name
 *
 * @param params an array of param structs
 * @param exec name of the executables path to get
 * @return a string containing the value. The string must be freed!
 */
char *jemVmGetExec(struct jem_param *params,const char *exec);

/**
 * Get the name of the vm
 *
 * @param vm a pointer to a vm struct
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemVmGetName(struct jem_vm *vm);

/**
 * Get the types the vm provides
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemVmGetProvidesType(struct jem_param *params);

/**
 * Get the versions of the vm provides
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemVmGetProvidesVersion(struct jem_param *params);

/**
 * Get the versions the vm
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemVmGetVersion(struct jem_param *params);

/**
 * Check to see if the vm is build only
 *
 * @return true if build only, false otherwise
 */
bool jemVmIsBuildOnly(struct jem_param *params);

/**
 * Check to see if the vm is/provides a particular type
 *
 * @param type the type of vm
 * @return true if the vm is/provides a particular type , false otherwise
 */
bool jemVmIsType(struct jem_param *params,const char *type);

/**
 * Check to see if the vm is a JDK
 *
 * @return true if the vm is a JDK, false otherwise
 */
bool jemVmIsJDK(struct jem_param *params);
/**
 * Check to see if the vm is a JRE
 *
 * @return true if the vm is a JRE, false otherwise
 */
bool jemVmIsJRE(struct jem_param *params);

int jemVmProvides(struct jem_param *params,
                 char **virtuals);

/**
 * Frees the allocated memory used by VM links in string pointer array
 *
 * @param vm_links a pointer an array of strings
 */
void jemFreeVMLinks(char **vm_links);

/**
 * Get the system VM's link
 *
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemVmGetSystemVMLink();

/**
 * Get the system VM's name by returning the basename of the system VM link
 *
 * @return a string containing the value. The string must be freed!
 */
char *jemVmGetSystemVMName();

/**
 * Get the user VM's link
 *
 * @return a string containing the value. The string must be freed!
 */
char *jemVmGetUserVMLink();

/**
 * Get a VM by index, filename, VM name including partial match, or JAVA_HOME
 * from an array of VM structs
 *
 * @param vms array of vm structs
 * @param file the name of the file to parse
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct jem_vm *jemVmGetVM(struct jem_vm *vms,const char *vm_name);

/**
 * Get user and system VM links
 *
 * @return a pointer to an array of strings containing the user and/or system
 * vm links, or null if not found. Array and links must be freed!
 */
char **jemVmGetVMLinks();

/**
 * Compares the filenames of two vms, used soley by qsort in loadVMs()
 *
 * @return an integer -1, 0, or 1.
 */

int jemVmCompareVMs(const void *v1, const void *v2);

/**
 * Loads all installed VMs config files. Storing them in an dynamically allocated
 * vm struct array.
 *
 * @return an array of vm structs. Which must be freed, including struct members!
 */
struct jem_vm *jemVmLoadVMs();

/**
 * Set the VM, create a symlink for the given vm to target
 *
 * @param vm pointer to an vm struct
 * @param target a string representing the vm symlink target
 */
void jemVmSetVM(struct jem_vm *vm,char *target);
