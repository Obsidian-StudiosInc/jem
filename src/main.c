/***************************************************************************
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

#include <argp.h>
#include <error.h>
#include <sys/stat.h>

#include "../lib/env_manager.h"

#define ENCODING "UTF-8"

#define OPT_SELECT_VM -10
#define OPT_PACKAGE -20
#define OPT_VIRT_PROVIDERS -30

const char *argp_program_version = "jem, version 0.1";
const char *argp_program_bug_address = "Obsidian-Studios, Inc. <support@o-sinc.com>";
static char doc[] = "\nJava Environment Manager\n"
                    "Copyright 2015 Obsidian-Studios, Inc.\n"
                    "Distributed under the terms of the GNU General Public License v3";
/* Unused arguments description*/
static char args_doc[] = "";

struct env jem_env;

/**
 * Cleanup call before program exit, clean up env, free memory, etc
 */
void cleanup() {
    freeEnv(&jem_env);
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

static struct argp_option options[] = {
    {0,0,0,0,"Global Options:"},
    {"nocolor", 'n', 0, 0, "Disable color output"},
    {0,0,0,0,"VM Options:", 2},
    {"active-vm", 'a', "VM",  0, "Use this vm instead of the active vm when returning information", 2},
    {"select-vm", 'a', 0,  OPTION_ALIAS, 2},
    {"java", 'J', 0, 0, "Print the location of the java executable", 2},
    {"javac", 'c', 0, 0, "Print the location of the javac executable", 2},
    {"jar", 'j', 0, 0, "Print the location of the jar executable", 2},
    {"tools", 't', 0, 0, "Print the path to tools.jar", 2},
    {"show-active-vm", 'f', 0, 0, "Print the active Virtual Machine", 2},
    {"java-version", 'v', 0, 0, "Print version information for the active VM", 2},
    {"get-env", 'g', "VAR", 0, "Print an environment variable from the active VM", 2},
    {"print", 'P', "VM", 0, "Print the environment for the specified VM", 2},
    {"exec_cmd", 'e', "COMMAND", 0, "Execute something which is in JAVA_HOME", 2},
    {"list-vms", 'L', 0, 0, "List available Java Virtual Machines", 2},
    {"list-available-vms", 'L', 0, OPTION_ALIAS},
    {"set-system-vm", 'S', "VM", 0, "Set the default Java VM for the system", 2},
    {"set-user-vm", 's', "VM", 0, "Set the default Java VM for the user", 2},
    {"runtime", 'r', 0, 0, "Print the runtime classpath", 2},
    {"jdk-home", 'O', 0, 0, "Print the location of the active JAVA_HOME", 2},
    {"jre-home", 'o', 0, 0, "Print the location of the active JAVA_HOME", 2},
    {0,0,0,0,"Package Options:", 3},
    {"list-packages", 'l', 0, 0, "List all available packages on the system", 3},
    {"list-available-packages", 'l', 0, OPTION_ALIAS},
    {"with-dependencies", 'd', 0, 0, "Include package dependencies in --classpath and --library calls", 3},
    {"classpath", 'p', "PACKAGE(s)", 0, "Print entries in the environment classpath for these packages", 3},
    {"package", OPT_PACKAGE, "PACKAGE(s)", 0, "Retrieve a value from a package(s) package.env file, value is specified by --query", 3},
    {"query", 'q', "PARAM(s)", 0, "Parameter(s) value(s) to retrieve from package(s) package.env file, specified by --package", 3},
    {"library", 'i', "LIRBARY(s)", 0, "Print java library paths for these packages", 3},
    {"get-virtual-providers", OPT_VIRT_PROVIDERS, "PACKAGE(S)", 0, "Return a list of packages that provide a virtual", 3},
    {0,0,0,0,"GNU Options:", 4},
    {0}
};

struct args {
    bool color;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct args *args = state->input;
    switch(key) {
        case 'a':
            initEnvVMs();
            jem_env.active_vm = getVM(jem_env.vms,arg);
            break;
        case 'd':
            with_dependencies = true;
            break;
        case 'n':
            color_output = false;
            break;
        case 'J':
            printExe("java");
            return(1);
        case 'c':
            printExe("javac");
            return(1);
        case 'j':
            printExe("jar");
            return(1);
        case 't':
            printToolsJar();
            return(1);
        case 'f':
            printActiveVM();
            return(1);
        case 'v':
            printJavaVersion();
            return(1);
        case 'g':
            printValueFromActiveVM(arg);
            return(1);
        case 'P':
            printVMParams(arg);
            return(1);
        case 'e':
            exeJavaBin(arg);
            return(1);
        case 'L':
            listAvailableVMs();
            return(1);
        case 'S':
            setSystemVM(arg);
            return(1);
        case 's':
            setUserVM(arg);
            return(1);
        case 'l':
            listPackages();
            return(1);
        case 'p':
            printPackageClasspath(arg);
            return(1);
        case 'q':
            if(state->argv[4])
                printValueFromPackage(state->argv[4],arg);
            return(1);
        case OPT_PACKAGE:
            if(state->argv[4])
                printValueFromPackage(arg,state->argv[4]);
            return(1);
        case 'i':
            printValueFromPackage(arg,"LIBRARY_PATH");
            return(1);
        case 'r':
            printValueFromActiveVM("BOOTCLASSPATH");
            return(1);
        case 'O':
            printValueFromActiveVM("JAVA_HOME");
            return(1);
        case 'o':
            printValueFromActiveVM("JAVA_HOME");
            return(1);
        case OPT_VIRT_PROVIDERS:
            printVirtualProviders(arg);
            return(1);
        case ARGP_KEY_NO_ARGS:
            if(!state->argv[1])
                argp_usage(state);
            return(1);
    }
    return(0);
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv) {
    struct args args;
    args.color = true;

    initEnv(&jem_env);

    argp_parse(&argp, argc, argv, ARGP_NO_EXIT, 0, &args);

    /* Invalid argument checks */

    atexit(cleanup);
    exit(EXIT_SUCCESS);
}
