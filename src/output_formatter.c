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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../lib/output_formatter.h"

bool color_output = true;
bool console_title = true;
bool auto_indent = true;
bool my_exit_status = EXIT_SUCCESS;

const char *terms[] = {"xterm", "Eterm", "aterm", "rxvt"};

/**
 * array of terminal codes
 */
struct code codes[] = {
    {'H', "\x1b[01m"},        /** Bold */
    {'U', "\x1b[04m"},        /** Underline */
    {'I', "\x1b[07m"},        /** Inverse */
    {'b', "\x1b[34;01m"},     /** Blue */
    {'B', "\x1b[34;06m"},     /** Dark Blue */
    {'c', "\x1b[36;01m"},     /** Cyan */
    {'C', "\x1b[36;06m"},     /** Dark Cyan */
    {'g', "\x1b[32;01m"},     /** Green */
    {'G', "\x1b[32;06m"},     /** Dark Green */
    {'m', "\x1b[35;01m"},     /** Magenta */
    {'M', "\x1b[35;06m"},     /** Dark Magenta */
    {'r', "\x1b[31;01m"},     /** Red */
    {'R', "\x1b[31;06m"},     /** Dark Red */
    {'y', "\x1b[33;01m"},     /** Yellow */
    {'Y', "\x1b[33;06m"},     /** Dark Yellow */
    {'$', "\x1b[0m"},         /** Reset */
    {'%', "%"}                /** Percent */
};

int codes_count = sizeof(codes) / sizeof(struct code);

/**
 * Returns the value of a code from a static array of code structs
 *
 * @param key the terminal code key
 * @return a pointer to a string containing the value. The string must NOT be freed!
 */
char *getCode(const int *key) {
    int i;
    for(i=0;i<codes_count;i++)
        if(codes[i].key==key)
            return(codes[i].value);
    return(NULL);
}

/**
 * Checks to see if the present terminal supports colors/codes
 *
 * @return true if valid terminal, false otherwise
 */
bool isValidTerm() {
    char *term = getenv("TERM");
    if(term) {
        int i;
        for(i=0;i<strlen(terms);i++)
            if(strcasecmp(term,terms[i])==0)
                return(true);
    }
    return(false);
}

/**
 * Adds terminal color codes to a message
 *
 * @param msg the message
 * @return a string containing the message and terminal color codes. The string must be freed!
 */
char *addColor(char *msg) {
    // The following check might be moved to jem's main function
    if(!isValidTerm())
        color_output = false;
    int len = strlen(msg);
    int nlen = 0;
    char *nmsg = calloc(len*2,sizeof(char));
    int i;
    for(i=0;i<len;i++) {
        char *c;
        if(color_output &&
           msg[i]=='%' &&
           (c = getCode(msg[i+1]))!=msg[i+1]) {
            i++;
            memcpy(nmsg+nlen,c,strlen(c));
            nlen += strlen(c);
        } else {
            memset(nmsg+nlen,msg[i],1);
            nlen++;
        }
    }
    return(nmsg);
}

/**
 * Adds preffix to a message that is indented by the length of the preffix
 *
 * @param preffix the message preffix
 * @param msg the message to indent
 * @return a string containing the combined preffix and indented message. The string must be freed!
 */
char *indent(const char *preffix,const char *msg) {
    int plen = strlen(preffix);
    int mlen = strlen(msg);
    char *nmsg;
    if(auto_indent &&
       strchr(msg,'\n')) {
        plen++;
        char *indent = calloc(plen+1,sizeof(char));
        memset(indent,'\n',1);
        memset(indent+1,' ',plen-1);
        nmsg = calloc(plen+mlen*2,sizeof(char));
        memcpy(nmsg,preffix,plen);
        int nlen = plen-1;
        char *str = calloc(mlen+1,sizeof(char));
        char *substr;
        char *cursor = str;
        memcpy(cursor,msg,mlen);
        while((substr = strsep(&cursor,"\n"))) {
            int sublen = strlen(substr);
            if(sublen>0) {
                memcpy(nmsg+nlen,substr,sublen);
                nlen += sublen;
                if(nlen<mlen) {
                    memcpy(nmsg+nlen,indent,plen);
                    mlen += plen;
                    nlen += plen;
                }
            } else
                memcpy(nmsg+nlen,"\n",1);
        }
        free(indent);
        free(str);
    } else {
        nmsg = calloc(plen+mlen+1, sizeof(char));
        memcpy(nmsg,preffix,plen);
        memcpy(nmsg+plen,msg,mlen);
    }
    return(nmsg);
}

/**
 * Print a message with colors and no formating
 *
 * @param msg the message
 */
void print(const char *msg) {
    char *cmsg = addColor(msg);
    fprintf(stdout,gettext("%s\n"),cmsg);
    free(cmsg);
}

/**
 * Print a message with colors and formatting
 *
 * @param preffix the message preffix, used for terminal codes (not required)
 * @param title the message title, indents the message by title width (not required)
 * @param msg the message to print
 * @param suffix the message suffix, used for terminal codes (not required)
 */
void printMsg(const char *preffix,
              const char *title,
              const char *msg,
              const char *suffix) {
    if(title)
        msg = indent(title,msg);
    int len = strlen(msg);
    if(preffix)
        len += strlen(preffix);
    if(suffix)
        len += strlen(suffix);
    char *pmsg = calloc(len+1,sizeof(char));
    if(preffix) {
        memcpy(pmsg,preffix,strlen(preffix));
        memcpy(pmsg+strlen(preffix),msg,strlen(msg));
    } else
        memcpy(pmsg,msg,strlen(msg));
    if(title)
        free(msg);
    if(suffix)
        memcpy(pmsg+strlen(pmsg),suffix,strlen(suffix));
    print(pmsg);
    free(pmsg);
}

/**
 * Print an error
 *
 * @param msg the error/message
 */
void printError(const char *msg) {
    printMsg("%H%R","!!! ERROR: ",msg,"%$\n");
    my_exit_status = EXIT_FAILURE;
}

/**
 * Print a warning
 *
 * @param msg the warning/message
 */
void printWarning(const char *msg) {
    printMsg("%H%Y","!!! WARNING: ",msg,"%$\n");
}

/**
 * Print an alert
 *
 * @param msg the alert/message
 */
void printAlert(const char *msg) {
    printMsg("%H%C","!!! ALERT: ",msg,"%$\n");
}

/**
 * Set the terminal title
 *
 * @param title the terminal title
 */
void setTermTitle(const char *title) {
    if(console_title &&
       isValidTerm())
        fprintf(stdout,gettext("\x1b]1;\x07\x1b]2;%s\x07"),title); // Presently not working no effect :(
}

void write(const char *msg) {
    print(msg); // Need to add strip() functionality
}
