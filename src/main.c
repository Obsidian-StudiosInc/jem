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

#include "../include/env_manager.h"

#define ENCODING "UTF-8" // presently not used

#define OPT_SELECT_VM -10
#define OPT_PACKAGE -20
#define OPT_VIRT_PROVIDERS -30

const char *argp_program_version = JEM_VERSION_STR;
const char *argp_program_bug_address = JEM_CONTACT;
static char doc[] = "\nJava Environment Manager\n"
                    "Copyright 2015 Obsidian-Studios, Inc.\n"
                    "Distributed under the terms of the GNU General Public License v3";
/* Unused arguments description*/
static char args_doc[] = "";

static struct argp_option options[] = {
    {0,0,0,0,"Global Options:"},
    {"nocolor", 'n', 0, 0, "Disable color output"},
    {0,0,0,0,"VM Options:", 2},
    {"active-vm", 'a', "VM",  0, "Use this vm instead of the active vm when returning information", 2},
    {"select-vm", 'a', 0,  OPTION_ALIAS},
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
    exit(my_exit_status);
}
