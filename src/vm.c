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
#include <sys/dir.h>
#include <sys/stat.h>

#include "../lib/vm.h"

/**
 * Frees the allocated memory used by an array of vm structs
 *
 * @param pkgs a pointer an array of vm structs
 */
void freeVMs(struct vm *vms) {
    if(!vms)
        return;
    int i;
    for(i=0;vms[i].filename;i++) {   // <- ugly, nasty, etc but works! :)
        free(vms[i].filename);
        freeParams(vms[i].params);
    }
    free(vms);
}

/**
 * Get the path to an executable by name
 *
 * @param params an array of param structs
 * @param exec name of the executables path to get
 * @return a string containing the value. The string must be freed!
 */
char *gjvmGetExec(struct param *params,const char *exec) {
    char *paths = getValue(params,"PATH");
    char *path = NULL;
    while((path = strsep(&paths,":"))) {
        char *cmd;
        asprintf(&cmd,"%s/%s",path,exec);
        struct stat st;
        if(stat(cmd,&st)<0) {
            free(cmd);
            if(errno==EACCES)
                printError("Java executable not readable"); // might need to be changed to throw an exception
            else
                printError("Invalid java executable, bad path or file name"); // might need to be changed to throw an exception
            return(NULL);
        }
        return(cmd);
    }
    return(NULL);
}

/**
 * Get the name of the vm
 *
 * @param vm a pointer to a vm struct
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetName(struct vm *vm) {
    return(basename(vm->filename));
}

/**
 * Get the types the vm provides
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetProvidesType(struct param *params) {
    return(getValue(params,"PROVIDES_TYPE"));
}

/**
 * Get the versions of the vm provides
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetProvidesVersion(struct param *params) {
    return(getValue(params,"PROVIDES_VERSION"));
}

/**
 * Get the versions the vm
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjvmGetVersion(struct param *params) {
    return(getValue(params,"VERSION"));
}

/**
 * Check to see if the vm is build only
 *
 * @return true if build only, false otherwise
 */
bool gjvmIsBuildOnly(struct param *params) {
    char *v = getValue(params,"BUILD_ONLY");
    if(v &&
       strcasecmp(v,"TRUE")==0)
        return(true);
    return(false);
}

/**
 * Check to see if the vm is/provides a particular type
 *
 * @param type the type of vm
 * @return true if the vm is/provides a particular type , false otherwise
 */
bool gjvmIsType(struct param *params,const char *type) {
    bool is_type = false;
    char *types = gjvmGetProvidesType(params);
    char *types_str = calloc(strlen(types)+1,sizeof(char));
    char *cursor = types_str;
    memcpy(cursor,types,strlen(types));
    char *t = NULL;
    while((t = strsep(&cursor," ")))
        if(strcasecmp(type,t)==0)
            is_type = true;
    free(types_str);
    return(is_type);
}

/**
 * Check to see if the vm is a JDK
 *
 * @return true if the vm is a JDK, false otherwise
 */
bool gjvmIsJDK(struct param *params) {
    return(gjvmIsType(params,"JDK"));
}

/**
 * Check to see if the vm is a JRE
 *
 * @return true if the vm is a JRE, false otherwise
 */
bool gjvmIsJRE(struct param *params) {
    return(gjvmIsType(params,"JRE"));
}

int gjvmProvides(struct param *params,
                 char **virtuals) {
    int provides = 0;
    char **vp = gjpGetProvides(params);
    if(vp) {
        int a;
        int b;
        for(a=0;vp[a];a++) {
            for(b=0;!provides && virtuals[b];b++)
                if(strcasecmp(vp[a],virtuals[b]))
                    provides++;
            free(vp[a]);
        }
        free(vp);
    }
    return(provides);
}


/**
 * Frees the allocated memory used by VM links in string pointer array
 *
 * @param vm_links a pointer an array of strings
 */
void freeVMLinks(char **vm_links) {
    if(!vm_links)
        return;
    int i;
    for(i=0;vm_links[i];i++)
        if(strcmp(vm_links[i],SYSTEM_VM_LINK)!=0) // don't free static system vm link
            free(vm_links[i]);
    free(vm_links);
}

/**
 * Get the system VM's link
 *
 * @return a string containing the value. The string must NOT be freed!
 */
char *getSystemVMLink() {
    return(SYSTEM_VM_LINK); // might do some checking or change to calculated in future
}

/**
 * Get the system VM's name by returning the basename of the system VM link
 *
 * @return a string containing the value. The string must be freed!
 */
char *getSystemVMName() {
    char *abs_file = calloc(BASE_NAME_SIZE+1,sizeof(char));
    if(readlink(getSystemVMLink(),abs_file,BASE_NAME_SIZE)<0) {
        if(errno==EACCES)
            printError("System VM link not readable"); // might need to be changed to throw an exception
        else if(errno==EINVAL)
            printError("System VM file is not a symlink"); // might need to be changed to throw an exception
        else if(errno==EIO)
            printError("A hardware error has occurred"); // might need to be changed to throw an exception
        free(abs_file);
        return(NULL);
    }
    char *bn_ptr = basename(abs_file);
    char *sys_vm_name = calloc(strlen(bn_ptr)+1,sizeof(char));
    memcpy(sys_vm_name,bn_ptr,strlen(bn_ptr));
    free(abs_file);
    return(sys_vm_name);
}

/**
 * Get the user VM's link
 *
 * @return a string containing the value. The string must be freed!
 */
char *getUserVMLink() {
    char *home = NULL;
    char *user_vm = NULL;
    if((home = getenv("HOME"))) 
        asprintf(&user_vm,"%s/%s",home,USER_VM_LINK_SUFFIX);
    return(user_vm);
}

/**
 * Get a VM by index, filename, VM name including partial match, or JAVA_HOME
 * from an array of VM structs
 *
 * @param vms array of vm structs
 * @param file the name of the file to parse
 * @return a pointer to a vm struct, or null if not found. Must NOT be freed!
 */
struct vm *getVM(struct vm *vms,const char *vm_name) {
    unsigned int i = vm_name[0];
    size_t vms_len = sizeof(struct vm) / sizeof(vms);
    if(strlen(vm_name)==1 &&
       isdigit(i)) {
        i = atoi(vm_name) - 1;
        if(i<=vms_len)
            return(&vms[i]);
        else
            return (NULL);
    }
    if(strlen(vm_name)>=2) {
        unsigned int b = vm_name[1];
        if(isdigit(b)) {
            char *end_ptr;
            unsigned long l = strtol(vm_name,end_ptr,10) - 1;
            if(i<=vms_len)
                return(&vms[l]);
            else
                return (NULL);
        }
    }
    for(i=0;vms[i].filename;i++) {
        if(strcasecmp(vm_name,vms[i].filename)==0)
            return(&vms[i]);
        if(strncasecmp(vm_name,gjvmGetName(&vms[i]),strlen(vm_name))==0)  // handles both full and partial matches
            return(&vms[i]);
        if(strcasecmp(vm_name,getValue(vms[i].params,"JAVA_HOME"))==0)
            return(&vms[i]);
    }
    return(NULL);
}

/**
 * Get user and system VM links
 *
 * @return a pointer to an array of strings containing the user and/or system
 * vm links, or null if not found. Array and links must be freed!
 */
char **getVMLinks() {
    char *user_vm = NULL;
    char **links = NULL;
    int c = 0;
    if((user_vm = getUserVMLink())) {
        links = calloc(3,sizeof(char *));
        links[0] = user_vm;
        c++;
    } else
        links = calloc(2,sizeof(char *));
    links[c] = getSystemVMLink();
    return(links);
}

/**
 * Compares the filenames of two vms, used soley by qsort in loadVMs()
 *
 * @return an integer -1, 0, or 1.
 */

int compareVMs(const struct vm *vm1, const struct vm *vm2) {
    if(vm1->filename && vm2->filename)
        return strcmp (basename(vm1->filename), basename(vm2->filename));
    return(-1);
}

/**
 * Loads all installed VMs config files. Storing them in an dynamically allocated
 * vm struct array.
 *
 * @return an array of vm structs. Which must be freed, including struct members!
 */
struct vm *loadVMs() {
    DIR *dp;
    struct vm *vms = NULL;
    int i = 0;
    if((dp = opendir(VMS_PATH))) {
        struct dirent *file;
        while((file = readdir(dp))) {
            if(!strcmp(file->d_name,".") ||
               !strcmp(file->d_name,".."))
                continue;
            struct vm *nvms = realloc(vms,sizeof(struct vm)*(i+2));
            if(!nvms)
                printError("Unable to allocate memory to hold all VM config files"); // needs to clean up and exit under error, not just print a message
            vms = nvms;
            vms[i+1].filename = NULL;
            vms[i+1].params = NULL;
            asprintf(&(vms[i].filename),"%s/%s",VMS_PATH,file->d_name);
            if(!vms[i].filename)
                printError("Unable to allocate memory to hold VM config file name"); // needs to clean up and exit under error, not just print a message
            vms[i].params = parseFile(vms[i].filename);
            i++;
        }
    } else {
        if(errno==EACCES)
            printError("VMs config directory not readable"); // needs to be changed to throw an exception
        else
            printError("Invalid VMs configuration directory"); // needs to be changed to throw an exception
    }
    if(vms)
        qsort(vms,i+2,sizeof(struct vm),compareVMs);
    closedir(dp);
    return(vms);
}

/**
 * Set the VM, create a symlink for the given vm to target
 *
 * @param vm pointer to an vm struct
 * @param target a string representing the vm symlink target
 * @return an array of vm structs. Which must be freed, including struct members!
 */
bool setVM(struct vm *vm,const char *target) {
    int basename_len = strlen(basename(target));
    int target_len = strlen(target);
    char *buffer = calloc(target_len+1,sizeof(char));
    memcpy(buffer,target,target_len-basename_len);
    struct stat st;
    char **dirs = calloc(3,sizeof(char *));
    dirs[0] = dirname(strdup(buffer));
    dirs[1] = buffer;
    int i;
    for(i=0;dirs[i];i++) {
        if(stat(dirs[i],&st)<0) {
            if(mkdir(dirs[i], S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)<0) {  // create dir if doesn't exist mode 755
                if(errno==EACCES)
                    printError("Write permission denied for System VM link parent directory"); // needs to be changed to throw an exception
                else
                    printError("Invalid VMs configuration directory"); // needs to be changed to throw an exception
                free(buffer);
                return;
            }
        }
    }
    free(dirs[0]);
    free(dirs);
    memset(buffer,'\0',target_len);
    if(readlink(target,buffer,target_len)>0) // if symlinks exists, remove
        unlink(target);
    free(buffer);
    char *vm_name = gjvmGetName(vm);
    char *symlnk;
    asprintf(&symlnk,"/usr/lib/jvm/%s",vm_name);
    if(symlink(symlnk,target)<0)
        printError("Failed to create symlink, unable to set VM"); // needs to be changed to throw an exception
    else {
        char *msg;
        char *vm_type = "system";
        if(strncasecmp(target,getSystemVMLink(),target_len)!=0)
            vm_type = "user";
        asprintf(&msg, "Now using %s as your %s JVM", gjvmGetName(vm),vm_type);
        print(msg);
        free(msg);
        if(gjvmIsBuildOnly(vm->params)) {
            asprintf(&msg,
                     "%s is marked as a build-only JVM. Using this vm is not recommended.",
                     gjvmGetName(vm));
            printWarning(msg);
            free(msg);
        }
    }
    free(symlnk);
}
