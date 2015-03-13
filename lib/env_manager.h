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
#include "vm.h"

/**
 * java environment
 */
struct env {
    struct pkg *pkgs;       /** packages */
    struct vm *vms;         /** virtual machines */
    struct vm *active_vm;   /** pointer to the active vm struct in the virtual machines vms struct array */
};

/**
 * Frees the allocated memory in a env struct
 *
 * @param params a pointer to an env struct
 */
void freeEnv(struct env *env);

/**
 * Get the active VM
 *
 * @param env pointer to an env struct
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct vm *getActiveVM(struct env *env);

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
