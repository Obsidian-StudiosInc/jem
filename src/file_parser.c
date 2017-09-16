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
#include <stdio.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "../include/file_parser.h"

/**
 * Frees the allocated memory in a array of param structs
 *
 * @param params a pointer to an array of param structs
 */
void jemFreeParams(struct jem_param *params) {
    if(!params)
        return;
    int i;
    for(i=0;params[i].name;i++) {
        free(params[i].name);
        if(params[i].value)
            free(params[i].value);
    }
    free(params);
}

/**
 * Returns the value of a parameter in the array of param structs
 *
 * @param params an array of param structs
 * @param name the name of the parameter
 * @return a string containing the value. The string must NOT be freed!
 */
char *jemGetValue(struct jem_param *params,const char *name) {
    int i;
    for(i=0;params[i].name;i++)
        if(strcmp(params[i].name,name)==0)
            return(params[i].value);
    return(NULL);
}

/**
 * Parses a config/package.env file's parameters. Storing them in an dynamically
 * allocated struct array.
 *
 * @param file the name of the file to parse
 * @return an array of param structs. Which must be freed, including struct members!
 */
struct jem_param *jemParseFile(const char *file) {
    struct jem_param *params = NULL;
    FILE *fp = fopen(file,"r");
    if(!fp) {
        if(errno==EACCES)
            jemPrintError("File not readable"); // needs to be changed to throw an exception
        else
            jemPrintError("Invalid file, does not exist"); // needs to be changed to throw an exception
        return(params);
    }
    short bytes_read = 0;
    char *line = NULL;
    size_t line_size = 0;
    int i = 0;
    char *var_name = NULL;      // ${VAR} name in a paramter value, global to for speedy re-use
    char *var_value = NULL;     // ${VAR} value to replace the variable in paramter value
    int var_value_len = 0;
    while((bytes_read = getline(&line,&line_size,fp))>0 && !feof(fp)) {
        char *value = NULL;     // value of a parameter in a config/env file
        if(strlen(line)>0 &&
           line[0]!='#' &&
           (value = strchr(line,'='))) {
            value++;
            int value_len = strlen(value);
            int name_len = bytes_read - value_len;
            if(value_len<=0)
                continue;
            if(value[0]=='"') {
                value++;
                value_len-=2;
                memset(line+bytes_read-1,'\0',1); // end the string in case extra data in buffer
            }
            struct jem_param *nparams = realloc(params,sizeof(struct jem_param)*(i+3));
            if(!nparams) {
                jemPrintError("Unable to allocate memory to hold all file parameters"); // needs to clean up and exit under error, not just print a message
                break;
            }
            params = nparams;
            params[i+1].name = NULL;
            params[i+1].value = NULL;
            params[i].name = calloc(name_len,sizeof(char));
            if(!params[i].name) {
                jemPrintError("Unable to allocate memory to hold all file parameter names"); // needs to clean up and exit under error, not just print a message
                break;
            }
            memcpy(params[i].name,line,name_len-1);
            params[i].value = NULL;
            // Expand ${VAR} before storing value
            char *subptr = value;
            int cur_len = 0;
            while((subptr = strstr(subptr,"${"))) {
                char *endstr = strstr(subptr,"}");
                int end_len = strlen(endstr);
                int sub_len = strlen(subptr);
                int var_len = sub_len - end_len - 1;
                end_len -= 2;
                // Re-use var/value if same, if not get new var name/value
                if(!var_name ||
                   strncasecmp(var_name,subptr+2,var_len-1)) {
                    if(var_name)
                        free(var_name);
                    var_name = calloc(var_len,sizeof(char));
                    if(!var_name) {
                        jemPrintError("Unable to allocate memory to hold all file parameter variable name"); // needs to clean up and exit under error, not just print a message
                        break;
                    }
                    memcpy(var_name,subptr+2,var_len-1);
                    var_value = jemGetValue(params,var_name);
                    if(!var_value)
                        break;
                    var_value_len = strlen(var_value);
                }
                value_len += var_value_len - var_len - 2;
                char *tmp = realloc(params[i].value,(value_len+1) * sizeof(char));
                if(!tmp) {
                    jemPrintError("Unable to re-allocate memory while parsing file parameter variables"); // needs to clean up and exit under error, not just print a message
                    break;
                }
                params[i].value = tmp;
                if(cur_len>0)
                    cur_len -= sub_len-1;                                       // back up so we overwrite/start where the next variable starts
                memcpy(params[i].value+cur_len,var_value,var_value_len);        // store var value
                memcpy(params[i].value+cur_len+var_value_len,endstr+1,end_len); // store end of unmodified str to modify
                memset(params[i].value+cur_len+var_value_len+end_len,'\0',1);   // end the string in case extra data in buffer
                cur_len += var_value_len + end_len;
                subptr = endstr+1;
            }
            // No ${VAR}s store value as is
            if(!params[i].value) {
                params[i].value = calloc(value_len,sizeof(char));
                if(!params[i].value)
                    jemPrintError("Unable to allocate memory to hold all file parameter values"); // needs to clean up and exit under error, not just print a message
                memcpy(params[i].value,value,value_len-1);
            }
            i++;
        }
        memset(line,'\0',line_size);
    }
    if(var_name)
        free(var_name);
    free(line);
    fclose(fp);
    return(params);
}
