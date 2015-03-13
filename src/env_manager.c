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

#include <errno.h>

#include "../lib/env_manager.h"

/**
 * Frees the allocated memory in a env struct
 *
 * @param params a pointer to an env struct
 */
void freeEnv(struct env *env) {
    if(!env)
        return;
    freePkgs(env->pkgs);
    freeVMs(env->vms);
}

/**
 * Get the active VM
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct vm *getActiveVM(struct env *env) {
    if(!env->active_vm)
        loadActiveVM(env);
    if(env->active_vm)
        return(env->active_vm);
    printError("Active vm not set, please run jem -s/-S"); // might need to be changed to throw an exception
    return(NULL);
}

/**
 * Initialize env struct
 *
 * @param env pointer to an uninitialized env structure
 */
void initEnv(struct env *env) {
    env->pkgs = NULL;
    env->vms = NULL;
    env->active_vm = NULL;
}

/**
 * Load the active VM, first by env variable. If that does not exist, by
 * looking at the symlinks, starting with user if it exists, then system.
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct vm *loadActiveVM(struct env *env) {
    struct vm *vm = NULL;
    char *vm_name = NULL;
    int i;
    char **vm_links = getVMLinks();
    for(i=0;vm_links[i];i++) {
        char *abs_file = calloc(BASE_NAME_SIZE+1,sizeof(char));
        if(readlink(vm_links[i],abs_file,BASE_NAME_SIZE)<0) {
            if(errno==EACCES)
                printError("VM link not readable"); // might need to be changed to throw an exception
            else if(errno==EINVAL)
                printError("VM file is not a symlink"); // might need to be changed to throw an exception
            else if(errno==EIO)
                printError("A hardware error has occurred"); // might need to be changed to throw an exception
            free(abs_file);
            continue;
        }
        vm = getVM(env->vms,basename(abs_file));
        free(abs_file);
        if(vm)
            break;
    }
    freeVMLinks(vm_links);
    if(vm)
        env->active_vm = vm;
    return(vm);
}
