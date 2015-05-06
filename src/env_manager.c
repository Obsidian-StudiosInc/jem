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
#include <sys/dir.h>
#include <sys/stat.h>
#include "../include/env_manager.h"

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
 * Get active package for virtual
 *
 * @param name string containing the name of the virtual
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetActiveVirtualProvider(const char *virtual) {
    char *package = NULL;
    char *providers = NULL;
    struct param *conf = parseFile(PKG_VIRTUAL_CONFIG);
    if(conf) {
        int i;
        for(i=0;conf[i].name;i++) {
            if(strcmp(virtual,conf[i].name)==0) {
                asprintf(&providers,"%s",conf[i].value);
                break;
            }
        }
        freeParams(conf);
    }
    if(!providers)
        providers = gjpGetVirtualProviders(virtual);
    if(!providers)
        return(package);
    else if(strcmp(providers,"")==0)
        return(providers);
    char *provider = NULL;
    char *provider_str = calloc(strlen(providers)+1,sizeof(char));
    char *p_cursor = provider_str;
    memcpy(p_cursor,providers,strlen(providers));
    while((provider = strsep(&p_cursor,","))) {
        char *package_env = NULL;
        asprintf(&package_env,"%s%s%s",PKG_PATH,provider,PKG_ENV);
        if(!package_env)
            continue;
        struct stat st;
        if(stat(package_env,&st)==0) {
            asprintf(&package,"%s",provider);
            free(package_env);
            break;
        }
        free(package_env);
    }
    free(provider_str);
    if(!package) {
        char *msg = NULL;
        asprintf(&msg,"No virtual providers for %s, please ensure you have\n"
                      "one of the following package's installed;\n%s",virtual,providers);
        printError(msg);
        free(msg);
    }
    free(providers);
    return(package);
}

/**
 * Get providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetVirtualProviders(const char *virtual) {
    char *packages = NULL;
    char *virtual_name = NULL;
    char *virtual_str = calloc(strlen(virtual)+1,sizeof(char));
    char *v_cursor = virtual_str;
    memcpy(v_cursor,virtual,strlen(virtual));
    while((virtual_name = strsep(&v_cursor,","))) {
        if(virtual_name) {
            char *virtual_file = NULL;
            asprintf(&virtual_file,"%s%s",PKG_VIRTUAL_PATH,virtual_name);
            struct stat st;
            if(virtual_file) {
                struct param *params = NULL;
                if(stat(virtual_file,&st)==0) // no output if file exist, remove for error if it does not
                    params = parseFile(virtual_file);
                if(params) {
                    char *providers = getValue(params,"PROVIDERS");
                    char *vvm_version = getValue(params,"VM");
                    if(vvm_version) {
                        while (*vvm_version && !isdigit(*vvm_version)) // skip through non-digit/alpha characters
                            vvm_version++;
                        initEnvVMs();
                        struct vm *vm = getActiveVM(&jem_env);
                        float vm_version = atof(gjvmGetProvidesVersion(vm->params));
                        if(atof(vvm_version)<=vm_version)
                            providers = "";
                    }
                    if(providers) {
                        char *provide = NULL;
                        char *pkgs_str = calloc(strlen(providers)+1,sizeof(char));
                        char *p_cursor = pkgs_str;
                        memcpy(p_cursor,providers,strlen(providers));
                        while((provide = strsep(&p_cursor," "))) {
                            if(provide && packages) {
                                char *old_packages = packages;
                                asprintf(&packages,"%s,%s",packages,provide);
                                free(old_packages);
                            } else if(provide)
                                asprintf(&packages,"%s",provide);
                        }
                        free(pkgs_str);
                    }
                    freeParams(params);
                }
                free(virtual_file);
            }
        }
    }
    free(virtual_str);
    return(packages);
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
        char *msg;
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
 * Loads a installed Package env file. Storing them in an dynamically allocated
 * pkg struct array.
 *
 * @return a pkg struct. Which must be freed, including struct members!
 */
struct pkg *loadPackage(char *name) {
    struct pkg *pkg = NULL;
    char *package_env = NULL;
    char *virt_pkg = gjpGetActiveVirtualProvider(name);
    if(virt_pkg) {
        if(strcmp(virt_pkg,"")==0)
            asprintf(&package_env,"%s%s",PKG_VIRTUAL_PATH,name);
        else
            asprintf(&package_env,"%s%s%s",PKG_PATH,virt_pkg,PKG_ENV);
        free(virt_pkg);
    } else
        asprintf(&package_env,"%s%s%s",PKG_PATH,name,PKG_ENV);
    if(package_env) {
        struct stat st;
        if(stat(package_env,&st)==0) {
            struct pkg *npkg = calloc(1,sizeof(struct pkg));
            if(!npkg)
                printError("Unable to allocate memory to hold package.env file"); // needs to clean up and exit under error, not just print a message
            pkg = npkg;
            asprintf(&(pkg->filename),"%s",package_env);
            if(!pkg->filename)
                printError("Unable to allocate memory to hold package.env file name"); // needs to clean up and exit under error, not just print a message
            asprintf(&(pkg->name),"%s",name);
            if(!pkg->name)
                printError("Unable to allocate memory to hold package name"); // needs to clean up and exit under error, not just print a message
            pkg->params = parseFile(pkg->filename);
        }
        free(package_env);
    }
    return(pkg);
}

/**
 * Loads all installed Package env files. Storing them in an dynamically allocated
 * pkg struct array.
 *
 * @return an array of pkg structs. Which must be freed, including struct members!
 */
struct pkg *loadPackages() {
    DIR *dp;
    struct pkg *pkgs = NULL;
    int i = 0;
    if((dp = opendir(PKG_PATH))) {
        struct dirent *file;
        while((file = readdir(dp))) {
            if(!strcmp(file->d_name,".") ||
               !strcmp(file->d_name,".."))
                continue;
            struct pkg *pkg = loadPackage(file->d_name);
            if(pkg) {
                struct pkg *npkgs = realloc(pkgs,sizeof(struct pkg)*(i+2));
                if(!npkgs)
                    printError("Unable to allocate memory to hold all package.env files"); // needs to clean up and exit under error, not just print a message
                pkgs = npkgs;
                pkgs[i+1].filename = NULL;
                pkgs[i+1].name = NULL;
                pkgs[i+1].params = NULL;
                pkgs[i] = *pkg;
                free(pkg);
                i++;
            }
        }
    } else {
        if(errno==EACCES)
            printError("Package directory not readable"); // needs to be changed to throw an exception
        else
            printError("Invalid package directory"); // needs to be changed to throw an exception
    }
    if(pkgs)
        qsort(pkgs,i+2,sizeof(struct pkg),loadPackagesCompare);
    closedir(dp);
    return(pkgs);
}

/**
 * Compares the names of two packages, used soley by qsort in loadPackages()
 *
 * @return an integer -1, 0, or 1.
 */

int loadPackagesCompare(const void *v1, const void *v2) {
    const struct pkg *p1 = v1;
    const struct pkg *p2 = v2;
    if(p1->name && p2->name)
        return strcmp (p1->name, p2->name);
    return(-1);
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
                                    asprintf(&classpath,"%s:/usr/share/%s/include/%s",classpath,deps[i].name,deps[i].jars[j]);
                                    free(old_cp);
                                } else
                                    asprintf(&classpath,"/usr/share/%s/include/%s",deps[i].name,deps[i].jars[j]);
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
            } else if(!cp && classpath) {
                char *old_cp = classpath;
                asprintf(&classpath,"%s",classpath);
                free(old_cp);
            } else if(cp)
                asprintf(&classpath,"%s",cp);
            else 
                asprintf(&classpath,"");
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
        asprintf(&path,"%s/include/tools.jar",getValue(avm->params,"JAVA_HOME"));
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
