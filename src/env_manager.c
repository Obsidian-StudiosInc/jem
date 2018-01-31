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

#include <errno.h>
#include <error.h>
#include <libgen.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "../include/env_manager.h"

/**
 * Cleanup call before program exit, clean up env, free memory, etc
 */
void jemCleanup() {
    jemFreeEnv(&jem_env);
}

/**
 * Frees the allocated memory in a env struct
 *
 * @param params a pointer to an env struct
 */
void jemFreeEnv(struct jem_env *env) {
    if(!env)
        return;
    jemFreePkgs(env->pkgs);
    jemFreeVMs(env->vms);
}

/**
 * Initialize env vms (virtual machines)
 */
void initEnvVMs() {
    if(!jem_env.vms)
        jem_env.vms = jemVmLoadVMs();
}

/**
 * Execute something which is in JAVA_HOME
 */
void jemExeJavaBin(char *exe_name) {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    if(avm) {
        char *exec;
        asprintf(&exec,"%s/bin/%s",jemGetValue(avm->params,"JAVA_HOME"),exe_name);
        if(exec) {
            char *argv[] = { exec, "-version", NULL };
            int e = execve(exec,argv,NULL);
            if(e==-1)
                jemPrintError("Unable to execute command");
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
struct jem_vm *jemGetActiveVM(struct jem_env *env) {
    if(!env->active_vm)
        jemLoadActiveVM(env);
    if(env->active_vm)
        return(env->active_vm);
    jemPrintError("Active vm not set, please run jem -s/-S"); // might need to be changed to throw an exception
    return(NULL);
}

/**
 * Get vms matching name
 *
 * @param name a string name of a VM
 * @return a jem_vm struct pointer array containing the value. The pointer 
 *         array must be freed, but NOT the VM array elements
 */
struct jem_vm **jemFindVM(char *name) {
    initEnvVMs();
    struct jem_vm **vms = NULL;
    int i;
    int vm_count = 0;
    for(i=0;jem_env.vms[i].filename;i++) {
        char *vname = NULL;
        asprintf(&vname,"%s-%s",name,jemVmGetVersion(jem_env.vms[i].params));
        if(!name ||
           strlen(name)==0 ||
           strcmp(jemVmGetName(&(jem_env.vms[i])),name)==0 ||
           strcmp(jemVmGetName(&(jem_env.vms[i])),vname)==0) {
            struct jem_vm **tmp = realloc(vms,sizeof(struct jem_vm *)*(vm_count+2));
            if(tmp) {
                vms = tmp;
                vms[vm_count] = &(jem_env.vms[i]);
                vms[vm_count+1] = NULL;
                vm_count++;
            } else
                jemPrintError("Unable to reallocate memory for VM pointers");
        }
        if(vname)
            free(vname);
    }
    return(vms);
}

/**
 * Initialize env struct
 *
 * @param env pointer to an uninitialized env structure
 */
void jemInitEnv(struct jem_env *env) {
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
struct jem_vm *jemLoadActiveVM(struct jem_env *env) {
    struct jem_vm *vm = NULL;
    char *tainted = NULL;
    char vm_name[1024] = {0};
    int len;
    if((tainted = getenv("JEM_VM"))) {
        len = strlen(tainted);
        if(len>0 && len<1024) { 
            strncpy(vm_name,tainted,1024);
            vm = jemVmGetVM(env->vms,vm_name);
        }
    } else {
        int i;
        char **vm_links = jemVmGetVMLinks();
        for(i=0;vm_links[i];i++) {
            char *abs_file = calloc(JEM_BASE_NAME_SIZE+1,sizeof(char));
            if(readlink(vm_links[i],abs_file,JEM_BASE_NAME_SIZE)<0) {
                if(errno==EACCES)
                    jemPrintError("VM link not readable"); // might need to be changed to throw an exception
                else if(errno==EINVAL)
                    jemPrintError("VM file is not a symlink"); // might need to be changed to throw an exception
                else if(errno==EIO)
                    jemPrintError("A hardware error has occurred"); // might need to be changed to throw an exception
                free(abs_file);
                continue;
            }
            vm = jemVmGetVM(env->vms,basename(abs_file));
            free(abs_file);
            if(vm)
                break;
        }
        jemFreeVMLinks(vm_links);
    }
    if(vm)
        env->active_vm = vm;
    return(vm);
}

/**
 * Print a list of the Available VMs
 */
void jemListAvailableVMs() {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    bool has_build_only = false;
    jemPrintMsg(stdout, "%H", NULL, "The following VMs are available:", "%$");
    int i;
    for(i=0;jem_env.vms[i].filename;i++) {
        char *msg;
        if(avm && strcmp(avm->filename,jem_env.vms[i].filename)==0) {
            if(jemVmIsBuildOnly(jem_env.vms[i].params)) {
                asprintf(&msg,
                         "%%G*)\t%s [%s] %%r(Build Only)%%$",
                         jemVmGetVersion(jem_env.vms[i].params),
                         jemVmGetName(&(jem_env.vms[i])));
                has_build_only = true;
            } else
                asprintf(&msg,"%%G*)\t%s [%s]%%$",
                         jemVmGetVersion(jem_env.vms[i].params),
                         jemVmGetName(&(jem_env.vms[i])));
        } else {
            if(jemVmIsBuildOnly(jem_env.vms[i].params)) {
                asprintf(&msg,
                         "%d)\t%s [%s] %%r(Build Only)%%$",
                         i+1,
                         jemVmGetVersion(jem_env.vms[i].params),
                         jemVmGetName(&(jem_env.vms[i])));
                has_build_only = true;
            } else
                asprintf(&msg,"%d)\t%s [%s]",
                         i+1,
                         jemVmGetVersion(jem_env.vms[i].params),
                         jemVmGetName(&(jem_env.vms[i])));
        }
        if(msg) {
            jemPrint(stdout,msg);
            free(msg);
        }
    }
    if(has_build_only)
        jemPrintMsg(stdout,
                    "\n%r",
                    NULL,
                    "VMs marked as Build Only may contain Security Vulnerabilities and/or be EOL.",
                    "%$");
}

/**
 * Print a list of installed Packages
 */
void jemListPackages() {
    jem_env.pkgs = jemPkgLoadPackages(false);
    if(jem_env.pkgs) {
        int i;
        for(i=0;jem_env.pkgs[i].filename;i++) {
            fprintf(stdout,
                    "[%s] %s (%s)\n",
                    jem_env.pkgs[i].name,
                    jemPkgGetDescription(jem_env.pkgs[i].params),
                    jem_env.pkgs[i].filename);
        }
        fprintf(stdout,"%d Java packages\n\n",i);
        jemCleanup();
    }
    jem_env.pkgs = jemPkgLoadPackages(true);
    if(jem_env.pkgs) {
        int i;
        for(i=0;jem_env.pkgs[i].filename;i++) {
            char *active = jemPkgGetActiveVirtualProvider(jem_env.pkgs[i].name);
            bool free_active = true;
            char *providers = jemPkgGetVirtualProviders(jem_env.pkgs[i].name,true);
            if(active && strcmp(active,"")==0) {
                free_active = false;
                free(active);
                struct jem_vm *vm = jemGetActiveVM(&jem_env);
                active = jemVmGetName(vm);
            }
            fprintf(stdout,
                    "[%s] Using: %s; Providers: %s (%s)\n",
                    jem_env.pkgs[i].name,
                    active,
                    providers,
                    jem_env.pkgs[i].filename);
            if(active && free_active)
                free(active);
            if(providers)
                free(providers);
        }
        fprintf(stdout,"%d Java virtual packages\n",i);
    }
}

/**
 * Print the active VM
 */
void jemPrintActiveVM() {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    if(avm)
        jemPrint(stdout,jemVmGetName(avm));
}

/**
 * Print the active VM parameters
 */
void jemPrintVMParams(const char *vm_name) {
    initEnvVMs();
    struct jem_vm *vm = jemVmGetVM(jem_env.vms,vm_name);
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
void jemPrintExe(const char *exe) {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    if(avm) {
        char *e = jemVmGetExec(avm->params,exe);
        if(e) {
            jemPrint(stdout,e);
            free(e);
        }
    }
}

/**
 * Print the active VM java version
 */
void jemPrintJavaVersion() {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    if(avm) {
        char *exec = jemVmGetExec(avm->params,"java");
        if(exec) {
            char *argv[] = { exec, "-version", NULL };
            int e = execve(exec,argv,NULL);
            if(e==-1)
                jemPrintError("Unable to print java version");
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
void jemPrintPackageClasspath(const char *name) {
    bool package_found = false;
    char *pkg_name = NULL;
    char *pkgs_str = calloc(strlen(name)+1,sizeof(char));
    char *cursor = pkgs_str;
    char *classpath = NULL;
    int pkg_name_len;
    int i;
    memcpy(cursor,name,strlen(name));
    while((pkg_name = strsep(&cursor,","))) {
        pkg_name_len = strlen(pkg_name);
        for( i=0; i<pkg_name_len; i++)
            if(pkg_name[i] == ':')
                pkg_name[i] = '-';
        package_found = false;
        struct jem_pkg *pkg = jemPkgLoadPackage(pkg_name);
        if(pkg) {
            char *pkg_classpath = jemPkgGetClasspath(pkg->params);
            package_found = true;
            if(jem_with_dependencies) {
                struct jem_dep *deps = jemPkgGetDeps(pkg->params);
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
                                } else if (!classpath)
                                    asprintf(&classpath,"/usr/share/%s/lib/%s",deps[i].name,deps[i].jars[j]);
                            }
                        } else {
                            struct jem_pkg *dep_pkg = jemPkgLoadPackage(deps[i].name);
                            if(dep_pkg) {
                                classpath = jemAppendStrs(classpath,":",jemPkgGetClasspath(dep_pkg->params));
                                jemFreePkg(dep_pkg);
                                free(dep_pkg);
                            } else {
                                char *msg;
                                asprintf(&msg,"Package %s a dependency of package %s was not found!",deps[i].name,pkg_name);
                                jemPrintError(msg);
                                free(msg);
                                package_found = false;
                                break;
                            }
                        }
                        jemFreeDep(&deps[i]);
                    }
                    free(deps);
                }
            }
            classpath = jemAppendStrs(classpath,":",pkg_classpath);
            jemFreePkg(pkg);
            free(pkg);
        } else {
            char *msg;
            asprintf(&msg,"Package %s was not found!",pkg_name);
            jemPrintError(msg);
            free(msg);
            package_found = false;
            break;
        }
    }
    if(classpath) {
        if(package_found)
            jemPrint(stdout,classpath);
        free(classpath);
    }
    free(pkgs_str);
}

/**
 * Print the active VM absolute path to tools.jar
 */
void jemPrintToolsJar() {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    if(avm &&
       !jemVmIsBuildOnly(avm->params)) {
        char *path;
        asprintf(&path,"%s/lib/tools.jar",jemGetValue(avm->params,"JAVA_HOME"));
        if(path) {
            struct stat st;
            if(stat(path,&st)==0)
                jemPrint(stdout,path);
            free(path);
        }
    }
}

/**
 * Print one or more parameter values from the active VM config file
 *
 * @param name string containing the name(s) of the parameter(s), multiple comma separated parameter names can be specified
 */
void jemPrintValueFromActiveVM(const char *name) {
    initEnvVMs();
    struct jem_vm *avm = jemGetActiveVM(&jem_env);
    if(avm) {
        char *var = NULL;
        char *vars_str = calloc(strlen(name)+1,sizeof(char));
        char *cursor = vars_str;
        memcpy(cursor,name,strlen(name));
        while((var = strsep(&cursor,","))) {
            char *value = jemGetValue(avm->params,var);
            if(value)
                jemPrint(stdout,value);
            else
                jemPrintError("Value could not be found in the active VM environment");
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
void jemPrintValueFromPackage(const char *name,const char *param) {
    char *package = NULL;
    char *package_str = calloc(strlen(name)+1,sizeof(char));
    char *p_cursor = package_str;
    memcpy(p_cursor,name,strlen(name));
    while((package = strsep(&p_cursor,","))) {
        struct jem_pkg *pkg = jemPkgLoadPackage(package);
        if(pkg) {
            char *var = NULL;
            char *vars_str = calloc(strlen(param)+1,sizeof(char));
            char *cursor = vars_str;
            memcpy(cursor,param,strlen(param));
            while((var = strsep(&cursor,","))) {
                char *value = jemGetValue(pkg->params,var);
                if(value)
                    jemPrint(stdout,value);
                else
                    jemPrint(stdout,"");
            }
            free(vars_str);
            jemFreePkg(pkg);
            free(pkg);
        } else
            jemPrintError("Package not found");
    }
    free(package_str);
}

/**
 * Print providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 */
void jemPrintVirtualProviders(const char *virtual) {
    char *providers = jemPkgGetVirtualProviders(virtual,true);
    if(providers) {
        jemPrint(stdout,providers);
        free(providers);
    }
}

/**
 * Set the System VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void jemSetSystemVM(const char *vm_name) {
    if(getuid()!=0) {
        jemPrintError("Only root user can set the System VM");
        return;
    }
    initEnvVMs();
    struct jem_vm *vm = jemVmGetVM(jem_env.vms,vm_name);
    if(!vm)
        jemPrintError("Could not find matching vm");
    else
        jemVmSetVM(vm,jemVmGetSystemVMLink());
}

/**
 * Set the User VM, create a symlink for the given vm
 *
 * @param vm_name string containing the vm name or number
 */
void jemSetUserVM(const char *vm_name) {
    if(getuid()==0) {
        jemPrintError("The root user can only use the System VM");
        return;
    }
    initEnvVMs();
    struct jem_vm *vm = jemVmGetVM(jem_env.vms,vm_name);
    if(!vm)
        jemPrintError("Could not find matching vm");
    else {
        char *target = jemVmGetUserVMLink();
        jemVmSetVM(vm,target);
        free(target);
    }
}
