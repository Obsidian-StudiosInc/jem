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
#include <error.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../lib/env_manager.h"

/**
 * Cleanup call before program exit, clean up env, free memory, etc
 */
void cleanup() {
    freeEnv(&jem_env);
}

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
 * Initialize env vms (virtual machines)
 */
void initEnvVMs() {
    if(!jem_env.vms)
        jem_env.vms = loadVMs();
}

/**
 * Execute something which is in JAVA_HOME
 */
void exeJavaBin(char *exe_name) {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    if(avm) {
        char *exec;
        asprintf(&exec,"%s/bin/%s",getValue(avm->params,"JAVA_HOME"),exe_name);
        if(exec) {
            char *argv[] = { exec, "-version", NULL };
            int e = execve(exec,argv,NULL);
            if(e==-1)
                printError("Unable to execute command");
            free(exec);
        }
    }
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

/**
 * Print a list of the Available VMs
 */
void listAvailableVMs() {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    bool has_build_only = false;
    print("%HThe following VMs are available:%$");
    int i;
    for(i=0;jem_env.vms[i].filename;i++) {
        char **msg;
        if(avm && strcmp(avm->filename,jem_env.vms[i].filename)==0) {
            if(gjvmIsBuildOnly(jem_env.vms[i].params)) {
                asprintf(&msg,
                         "%%G*)\t%s [%s] %%r(Build Only)%%$",
                         gjvmGetVersion(jem_env.vms[i].params),
                         gjvmGetName(&(jem_env.vms[i])));
                has_build_only = true;
            } else
                asprintf(&msg,"%%G*)\t%s [%s]%%$",
                         gjvmGetVersion(jem_env.vms[i].params),
                         gjvmGetName(&(jem_env.vms[i])));
        } else {
            if(gjvmIsBuildOnly(jem_env.vms[i].params)) {
                asprintf(&msg,
                         "%d)\t%s [%s] %%r(Build Only)%%$",
                         i+1,
                         gjvmGetVersion(jem_env.vms[i].params),
                         gjvmGetName(&(jem_env.vms[i])));
                has_build_only = true;
            } else
                asprintf(&msg,"%d)\t%s [%s]",
                         i+1,
                         gjvmGetVersion(jem_env.vms[i].params),
                         gjvmGetName(&(jem_env.vms[i])));
        }
        if(msg) {
            print(msg);
            free(msg);
        }
    }
    if(has_build_only)
        print("\n%rVMs marked as Build Only may contain Security Vulnerabilities and/or be EOL.%$");
}

/**
 * Print a list of installed Packages
 */
void listPackages() {
    jem_env.pkgs = loadPackages();
    if(jem_env.pkgs) {
        int i;
        for(i=0;jem_env.pkgs[i].filename;i++) {
            fprintf(stdout,
                    "[%s] %s (%s)\n",
                    jem_env.pkgs[i].name,
                    gjpGetDescription(jem_env.pkgs[i].params),
                    jem_env.pkgs[i].filename);
        }
    }
}

/**
 * Print the active VM
 */
void printActiveVM() {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    if(avm)
        print(gjvmGetName(avm));
}

/**
 * Print the active VM parameters
 */
void printVMParams(const char *vm_name) {
    initEnvVMs();
    struct vm *vm = getVM(jem_env.vms,vm_name);
    int i;
    if(vm)
        for(i=0;vm->params[i].name;i++)
            fprintf(stdout,"%s=%s\n",vm->params[i].name,vm->params[i].value);
}

/**
 * Print a command including path using the active VM
 * 
 * @param exec string containing the executable to print
 */
void printExe(const char *exe) {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    if(avm) {
        char *e = gjvmGetExec(avm->params,exe);
        if(e) {
            print(e);
            free(e);
        }
    }
}

/**
 * Print the active VM java version
 */
void printJavaVersion() {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    if(avm) {
        char *exec = gjvmGetExec(avm->params,"java");
        if(exec) {
            char *argv[] = { exec, "-version", NULL };
            int e = execve(exec,argv,NULL);
            if(e==-1)
                printError("Unable to print java version");
            free(exec);
        }
    }
}

/**
 * Print one or more package classpath values from the package.env file
 *
 * @param name string containing the name(s) of the package(s), 
 *             multiple comma separated package names can be specified
 */
void printPackageClasspath(const char *name) {
    bool package_found = false;
    char *pkg_name = NULL;
    char *pkgs_str = calloc(strlen(name)+1,sizeof(char));
    char *cursor = pkgs_str;
    char *classpath = NULL;
    memcpy(cursor,name,strlen(name));
    while((pkg_name = strsep(&cursor,","))) {
        package_found = false;
        struct pkg *pkg = loadPackage(pkg_name);
        if(pkg) {
            package_found = true;
            if(with_dependencies) {
                struct dep *deps = gjpGetDeps(pkg->params);
                if(deps) {
                    int i;
                    for(i=0;deps[i].name;i++) {
                        int j;
                        if(deps[i].jars) {
                            for(j=0;deps[i].jars[j];j++) {
                                if(classpath) {
                                    char *old_cp = classpath;
                                    asprintf(&classpath,"%s:/usr/share/%s/lib/%s",classpath,deps[i].name,deps[i].jars[j]);
                                    free(old_cp);
                                } else
                                    asprintf(&classpath,"/usr/share/%s/lib/%s",deps[i].name,deps[i].jars[j]);
                            }
                        } else {
                            struct pkg *dep_pkg = loadPackage(deps[i].name);
                            if(dep_pkg) {
                                char *dep_cp = getValue(dep_pkg->params,"CLASSPATH");
                                if(dep_cp && classpath) {
                                    char *old_cp = classpath;
                                    asprintf(&classpath,"%s:%s",classpath,dep_cp);
                                    free(old_cp);
                                } else if(dep_cp)
                                    asprintf(&classpath,"%s",dep_cp);
                                freePkg(dep_pkg);
                                free(dep_pkg);
                            } else {
                                char *msg;
                                asprintf(&msg,"Package %s a dependency of package %s was not found!",deps[i].name,pkg_name);
                                printError(msg);
                                free(msg);
                                break;
                            }
                        }
                        freeDep(&deps[i]);
                    }
                    free(deps);
                }
            }
            char *cp = getValue(pkg->params,"CLASSPATH");
            if(cp && classpath) {
                char *old_cp = classpath;
                asprintf(&classpath,"%s:%s",classpath,cp);
                free(old_cp);
            } else if(cp)
                asprintf(&classpath,"%s",cp);
            freePkg(pkg);
            free(pkg);
        } else {
            char *msg;
            asprintf(&msg,"Package %s was not found!",pkg_name);
            printError(msg);
            free(msg);
            break;
        }
    }
    if(classpath) {
        if(package_found)
            print(classpath);
        free(classpath);
    }
    free(pkgs_str);
}

/**
 * Print the active VM absolute path to tools.jar
 */
void printToolsJar() {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    if(avm &&
       !gjvmIsBuildOnly(avm->params)) {
        char *path;
        asprintf(&path,"%s/lib/tools.jar",getValue(avm->params,"JAVA_HOME"));
        if(path) {
            struct stat st;
            if(stat(path,&st)==0)
                print(path);
            free(path);
        }
    }
}

/**
 * Print one or more parameter values from the active VM config file
 *
 * @param name string containing the name(s) of the parameter(s), multiple comma separated parameter names can be specified
 */
void printValueFromActiveVM(const char *name) {
    initEnvVMs();
    struct vm *avm = getActiveVM(&jem_env);
    if(avm) {
        char *var = NULL;
        char *vars_str = calloc(strlen(name)+1,sizeof(char));
        char *cursor = vars_str;
        memcpy(cursor,name,strlen(name));
        while((var = strsep(&cursor,","))) {
            char *value = getValue(avm->params,var);
            if(value)
                print(value);
            else
                printError("Value could not be found in the active VM environment");
        }
        free(vars_str);
    }
}

/**
 * Print one or more parameter values from one or more package.env file
 *
 * @param name string containing the name(s) of the package(s), multiple package(s) separated parameter names can be specified
 * @param param string containing the parameter(s) names, multiple comma separated parameter names can be specified
 */
void printValueFromPackage(const char *name,const char *param) {
    char *package = NULL;
    char *package_str = calloc(strlen(name)+1,sizeof(char));
    char *p_cursor = package_str;
    memcpy(p_cursor,name,strlen(name));
    while((package = strsep(&p_cursor,","))) {
        struct pkg *pkg = loadPackage(package);
        if(pkg) {
            char *var = NULL;
            char *vars_str = calloc(strlen(param)+1,sizeof(char));
            char *cursor = vars_str;
            memcpy(cursor,param,strlen(param));
            while((var = strsep(&cursor,","))) {
                char *value = getValue(pkg->params,var);
                if(value)
                    print(value);
                else
                    print("");
            }
            free(vars_str);
            freePkg(pkg);
            free(pkg);
        } else
            printError("Package not found");
    }
    free(package_str);
}

/**
 * Print providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 */
void printVirtualProviders(const char *virtual) {
    char *providers = gjpGetVirtualProviders(virtual);
    if(providers) {
        print(providers);
        free(providers);
    }
}

/**
 * Set the System VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void setSystemVM(const char *vm_name) {
    if(getuid()!=0) {
        printError("Only root user can set the System VM");
        return;
    }
    initEnvVMs();
    struct vm *vm = getVM(jem_env.vms,vm_name);
    if(!vm)
        printError("Could not find matching vm");
    else
        setVM(vm,getSystemVMLink());
}

/**
 * Set the User VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void setUserVM(const char *vm_name) {
    if(getuid()==0) {
        printError("The root user can only use the System VM");
        return;
    }
    initEnvVMs();
    struct vm *vm = getVM(jem_env.vms,vm_name);
    if(!vm)
        printError("Could not find matching vm");
    else {
        char *target = getUserVMLink();
        setVM(vm,target);
        free(target);
    }
}
