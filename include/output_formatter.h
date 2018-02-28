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

#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern bool jem_color_output;
extern bool jem_console_title;
extern bool jem_auto_indent;
extern bool jem_exit_status;

extern const char *jem_terms[];

extern int jem_codes_count;

/**
 * terminal codes
 */
struct jem_term_code {
    char key;     /** code key */
    char *value;  /** code value */
};

/**
 * array of terminal codes
 */
extern struct jem_term_code jem_term_codes[];

/**
 * Returns the value of a code from a static array of code structs
 *
 * @param key the terminal code key
 * @return a pointer to a string containing the value. The string must NOT be freed!
 */
char *jemGetTermCode(char key);

/**
 * Checks to see if the present terminal supports colors/codes
 *
 * @return true if valid terminal, false otherwise
 */
bool jemIsValidTerm();

/**
 * Adds terminal color codes to a message
 *
 * @param msg the message
 * @return a string containing the message and terminal color codes. The string must be freed!
 */
char *jemAddTermColor(char *msg);

/**
 * Appends two strings with a separator if specified
 *
 * @param cur_str a the current/original string the other should be added to
 * @param sep_str a non-null string separator or "" for none.
 * @param add_str a string to add
 * @return a string containing the two strings with separator. The string must be freed!
 */
char *jemAppendStrs(char* cur_str,char *sep_str,char *add_str);

/**
 * Adds preffix to a message that is indented by the length of the preffix
 *
 * @param preffix the message preffix
 * @param msg the message to indent
 * @return a string containing the combined preffix and indented message. The string must be freed!
 */
char *jemIndent(const char *preffix,const char *msg);

/**
 * Print a message with colors and no formating
 *
 * @param msg the message
 */
void jemPrint(FILE *stream, char *msg);

/**
 * Print a message with colors and formatting
 *
 * @param preffix the message preffix, used for terminal codes (not required)
 * @param title the message title, indents the message by title width (not required)
 * @param msg the message to print
 * @param suffix the message suffix, used for terminal codes (not required)
 */
void jemPrintMsg(FILE *stream,
                 const char *preffix,
                 const char *title,
                 char *msg,
                 const char *suffix);

/**
 * Print an error
 *
 * @param msg the error/message
 */
void jemPrintError(char *msg);

/**
 * Print a warning
 *
 * @param msg the warning/message
 */
void jemPrintWarning(char *msg);

/**
 * Print an alert
 *
 * @param msg the alert/message
 */
void jemPrintAlert(char *msg);

/**
 * Set the terminal title
 *
 * @param title the terminal title
 */
void jemSetTermTitle(const char *title);
