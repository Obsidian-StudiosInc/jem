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

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define JEM_CONTACT "Obsidian-Studios, Inc. <jem@o-sinc.com>"
#define JEM_VERSION_MAJOR 0
#define JEM_VERSION_MINOR 2
#define JEM_VERSION_SEPARATOR "."
#define JEM_VERSION_NUMERIC STR(JEM_VERSION_MAJOR) "." STR(JEM_VERSION_MINOR)
#define JEM_VERSION_STR "jem, v" JEM_VERSION_NUMERIC
