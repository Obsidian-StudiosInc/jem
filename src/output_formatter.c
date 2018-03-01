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

#include <libintl.h>
#include <stdio.h>
#include "../include/output_formatter.h"

bool jem_color_output = true;
bool jem_auto_indent = true;
bool jem_exit_status = EXIT_SUCCESS;

/**
 * array of terminal codes
 */
struct jem_term_code jem_term_codes[] = {
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

int jem_codes_count = sizeof(jem_term_codes) / sizeof(struct jem_term_code);

/**
 * Returns the value of a code from a static array of code structs
 *
 * @param key the terminal code key
 * @return a pointer to a string containing the value. The string must NOT be freed!
 */
char *jemGetTermCode(char key) {
    int i;
    for(i=0;i<jem_codes_count;i++)
        if(jem_term_codes[i].key==key)
            return(jem_term_codes[i].value);
    return(NULL);
}

/**
 * Checks to see if the present terminal supports colors/codes
 *
 * @return true if valid terminal, false otherwise
 */
bool jemIsValidTerm() {
    if(getenv("LS_COLORS") || getenv("XTERM_256_COLORS"))
       return(true);
    return(false);
}

/**
 * Adds terminal color codes to a message
 *
 * @param msg the message
 * @return a string containing the message and terminal color codes. The string must be freed!
 */
char *jemAddTermColor(char *msg) {
    // The following check might be moved to jem's main function
    if(!jemIsValidTerm())
        jem_color_output = false;
    int len = strlen(msg);
    int nlen = 0;
    char *nmsg = calloc(len*2,sizeof(char));
    int i;
    for(i=0;i<len;i++) {
        char *c;
        if(jem_color_output &&
           msg[i]=='%' &&
           (c = jemGetTermCode(msg[i+1])) &&
           c[0]!=msg[i+1]) {
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
 * Appends two strings with a separator if specified
 *
 * @param cur_str a the current/original string the other should be added to
 * @param sep_str a non-null string separator or "" for none.
 * @param add_str a string to add
 * @return a string containing the two strings with separator. The string must be freed!
 */
char *jemAppendStrs(char* cur_str,char *sep_str,char *add_str) {
    char *new_str = NULL;
    if(cur_str && add_str) {
        char *old_str = cur_str;
        if(sep_str)
            asprintf(&new_str,"%s%s%s",cur_str,sep_str,add_str);
        else
            asprintf(&new_str,"%s%s",cur_str,add_str);
        free(old_str);
    }
    return(new_str);
}

/**
 * Adds preffix to a message that is indented by the length of the preffix
 *
 * @param preffix the message preffix
 * @param msg the message to indent
 * @return a string containing the combined preffix and indented message. The string must be freed!
 */
char *jemIndent(const char *preffix,const char *msg) {
    int plen = strlen(preffix);
    int mlen = strlen(msg);
    char *nmsg;
    if(jem_auto_indent &&
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
void jemPrint(FILE *stream, char *msg) {
    char *cmsg = jemAddTermColor(msg);
    fprintf(stream,gettext("%s\n"),cmsg);
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
void jemPrintMsg(FILE *stream,
                 const char *preffix,
                 const char *title,
                 char *msg,
                 const char *suffix) {
    bool color = jemIsValidTerm();
    if(title)
        msg = jemIndent(title,msg);
    char *pmsg;
    if(preffix && color)
        asprintf(&pmsg,"%s%s",preffix,msg);
    else
        asprintf(&pmsg,"%s",msg);
    if(title)
        free(msg);
    if(suffix && color) {
        char *tmp_msg = pmsg;
        asprintf(&pmsg,"%s%s",pmsg,suffix);
        free(tmp_msg);
    }
    jemPrint(stream,pmsg);
    free(pmsg);
}

/**
 * Print an error
 *
 * @param msg the error/message
 */
void jemPrintError(char *msg) {
    jemPrintMsg(stderr,"%H%R","!!! ERROR: ",msg,"%$");
    jem_exit_status = EXIT_FAILURE;
}

/**
 * Print a warning
 *
 * @param msg the warning/message
 */
void jemPrintWarning(char *msg) {
    jemPrintMsg(stderr,"%H%Y","!!! WARNING: ",msg,"%$");
}

/**
 * Print an alert
 *
 * @param msg the alert/message
 */
void jemPrintAlert(char *msg) {
    jemPrintMsg(stderr,"%H%C","!!! ALERT: ",msg,"%$");
}
