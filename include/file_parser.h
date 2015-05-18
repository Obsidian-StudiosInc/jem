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

#pragma once

#include "output_formatter.h"

/**
 * config/package.env file parameter
 */
struct jem_param {
    char *name;   /** param name */
    char *value;  /** param value */
};

/**
 * Frees the allocated memory in a array of param structs
 *
 * @param params a pointer to an array of param structs
 */
void jemFreeParams(struct jem_param *params);

/**
 * Returns the value of a parameter in the array of param structs
 *
 * @param params an array of param structs
 * @param name the name of the parameter
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemGetValue(struct jem_param *params,const char *name);

/**
 * Parses a config/package.env file's parameters. Storing them in an dynamically
 * allocated struct array.
 *
 * @param file the name of the file to parse
 * @return an array of param structs. Which must be freed, including struct members!
 */
struct jem_param *jemParseFile(const char *file);
