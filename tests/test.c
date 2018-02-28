/***************************************************************************
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

#include <stdio.h>

#include "../include/env_manager.h"

char *jvm;
char *vm_home;
char *vm_conf_file;
char *pkg_env_file;

void testOutputFormatter() {
    int i;

    fprintf(stdout,"Testing output_formatter.h functions\n\n"
                   "char *getCode(const char *key) output effects next terminal line\n");
    for(i=0;i<jem_codes_count;i++)
        fprintf(stdout,"getCode('%c')%s\n",jem_term_codes[i].key,jemGetTermCode(jem_term_codes[i].key));

    fprintf(stdout,"\nbool isValidTerm()\n");
    if(jemIsValidTerm())
        fprintf(stdout,"Yes valid terminal\n");
    else
        fprintf(stdout,"No invalid terminal\n");

    fprintf(stdout,"\nchar *addColor(const char *key)\n");
    for(i=0;i<jem_codes_count;i++) {
        char msg[10];
        memset(msg,'%',1);
        memset(msg+1,jem_term_codes[i].key,1);
        memcpy(msg+2," text %$",8);
        memset(msg+10,'\0',1);
        char *nmsg = jemAddTermColor(msg);
        fprintf(stdout,"addColor(%s) -> %s\n",msg,nmsg);
        free(nmsg);
    }

    fprintf(stdout,"\nchar *indent(const char *prefix,const char *msg)\n");
    char *imsg = jemIndent("Preffix : ","A message that does not repeat (no indent test)");
    fprintf(stdout,"\nindent(\"Preffix : \",\"A message that does not repeat\") ->\n%s\n",imsg);
    free(imsg);

    imsg = jemIndent("Preffix : ","An indented message that repeats 1\n"
                               "An indented message that repeats 2\n"
                               "An indented message that repeats 3");
    fprintf(stdout,"\nindent(\"Preffix : \",\"An indented message that repeats 1\n%s%s\") ->\n%s\n",
                   "An indented message that repeats 2\n",
                   "An indented message that repeats 3",imsg);
    free(imsg);

    imsg = jemIndent("!!! Alert : ","Printing an alert that spans\na few lines so we get some\nline indenting\n");
    fprintf(stdout,"\nindent(\"!!! Alert : \",\"Printing an alert that spans\na few lines so we get some\nline indenting\n\") ->\n%s",imsg);
    free(imsg);

    fprintf(stdout,"\nvoid printAlert(const char *msg)\n");
    jemPrintAlert("Printing an alert that spans\na few lines so we get some\nline indenting\n");

    fprintf(stdout,"void printError(const char *msg)\n");
    jemPrintError("Printing an error that spans\na few lines so we get some\nline indenting\n");

    fprintf(stdout,"void printWarning(const char *msg)\n");
    jemPrintWarning("Printing a warning that spans\na few lines so we get some\nline indenting\n");

}

void testFileParser() {
    fprintf(stdout,"\nTesting file_parser.h functions\n");
    int i;
    fprintf(stdout,"\nstruct param *params;\n");
    struct jem_param *params = NULL;

    fprintf(stdout,"\nparse vm config (also tests/fills in variables in values)");
    fprintf(stdout,"\nparams = parseFile(\"%s\"); ->\n",vm_conf_file);
    params = jemParseFile(vm_conf_file);
    for(i=0;params[i].name;i++)
        fprintf(stdout,"\tparams[%d]->name=%s\n\tparams[%d]->value=%s\n",i,params[i].name,i,params[i].value);

    fprintf(stdout,"\ngetValue(params,\"BOOTCLASSPATH\") ->\n%s\n",jemGetValue(params,"BOOTCLASSPATH"));

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    jemFreeParams(params);

    fprintf(stdout,"\nparse package.env");
    fprintf(stdout,"\nparams = parseFile(\"%s\"); ->\n",pkg_env_file);
    params = jemParseFile(pkg_env_file);
    for(i=0;params[i].name;i++)
        fprintf(stdout,"\tparams[%d]->name=%s\n\tparams[%d]->value=%s\n",i,params[i].name,i,params[i].value);

    fprintf(stdout,"\nchar *getValue(struct param *params,const char *name) ->\n");

    fprintf(stdout,"\ngetValue(params,\"MERGE_VM\") ->\n%s\n",jemGetValue(params,"MERGE_VM"));

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    jemFreeParams(params);

}

void testPackage() {
    fprintf(stdout,"\nTesting package.h functions\n");
    fprintf(stdout,"\nstruct param *params;\n");
    struct jem_param *params = NULL;

    fprintf(stdout,"\nparams = parseFile(\"/usr/share/idontexist/package.env\");\n");
    params = jemParseFile("/usr/share/idontexist/package.env");

    fprintf(stdout,"\nparams = parseFile(\"samples/usr/share/cglib-2.1/package.env\");\n");
    params = jemParseFile("samples/usr/share/cglib-2.1/package.env");

    fprintf(stdout,"\nchar *jemPkgGetDescription(struct params *params) ->\n%s\n",jemPkgGetDescription(params));
    fprintf(stdout,"\nchar *jemPkgGetClasspath(struct params *params) ->\n%s\n",jemPkgGetClasspath(params));

    fprintf(stdout,"\nstruct dep *jemPkgGetDeps(struct params *params) ->\n");
    struct jem_dep *deps = jemPkgGetDeps(params);
    if(deps) {
        int i;
        for(i=0;deps[i].name;i++) {
            fprintf(stdout,"\t%s\n",deps[i].name);
            int j;
            if(deps[i].jars)
                for(j=0;deps[i].jars[j];j++)
                    fprintf(stdout,"\t\t%s\n",deps[i].jars[j]);
            jemFreeDep(&deps[i]);
        }
        free(deps);
    }

    fprintf(stdout,"\ncleanup()\n");
    jemCleanup();

    fprintf(stdout,"\nchar **jemPkgGetJarNames(char *pkg_name) ->\n");
    char **jars = jemPkgGetJarNames("ant-core");
    if(jars) {
        int i;
        for(i=0;jars[i];i++) {
            fprintf(stdout,"\t%s\n",jars[i]);
            free(jars[i]);
        }
        free(jars);
    }

    fprintf(stdout,"\nstruct dep *jemPkgGetBuildDeps(struct params *params) ->\n");
    struct jem_dep *build_deps = jemPkgGetBuildDeps(params);
    if(build_deps) {
        int i;
        for(i=0;build_deps[i].name;i++) {
            fprintf(stdout,"\t%s\n",build_deps[i].name);
            int j;
            if(build_deps[i].jars)
                for(j=0;build_deps[i].jars[j];j++)
                    fprintf(stdout,"\t\t%s\n",build_deps[i].jars[j]);
            jemFreeDep(&build_deps[i]);
        }
        free(build_deps);
    }

    fprintf(stdout,"\nstruct dep *jemPkgGetOptDeps(struct params *params) ->\n");
    struct jem_dep *opt_deps = jemPkgGetOptDeps(params);
    if(opt_deps) {
        int i;
        for(i=0;opt_deps[i].name;i++) {
            fprintf(stdout,"\t%s\n",opt_deps[i].name);
            int j;
            if(opt_deps[i].jars)
                for(j=0;opt_deps[i].jars[j];j++)
                    fprintf(stdout,"\t\t%s\n",opt_deps[i].jars[j]);
            jemFreeDep(&opt_deps[i]);
        }
        free(opt_deps);
    }

    fprintf(stdout,"\nDoesn't seem like any package.env file has this yet?");
    fprintf(stdout,"\nchar **jemPkgGetProvides(params) ->\n");

    char **provides = jemPkgGetProvides(params);
    if(provides) {
        int i;
        for(i=0;provides[i];i++) {
            fprintf(stdout,"\t%s\n",provides[i]);
            free(provides[i]);
        }
        free(provides);
    }

    fprintf(stdout,"\nchar *jemPkgGetTarget(struct params *params) -> %s\n",jemPkgGetTarget(params));

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    jemFreeParams(params);

    fprintf(stdout,"\nconst char *jemPkgGetVirtualProviders(\"javamail\",true)->\n");
    char *virtual = jemPkgGetVirtualProviders("javamail",true);
    fprintf(stdout,"%s\n",virtual);
    free(virtual);

    fprintf(stdout,"\nconst char *jemPkgGetActiveVirtualProvider(\"javamail\")->\n");
    virtual = jemPkgGetActiveVirtualProvider("javamail");
    fprintf(stdout,"%s\n",virtual);
    free(virtual);

}

void testVM() {
    fprintf(stdout,"\nTesting vm.h functions\n");
    fprintf(stdout,"\nstruct param *params;\n");
    struct jem_param *params = NULL;

    fprintf(stdout,"\nparams = parseFile(\"%s\");\n",vm_conf_file);
    params = jemParseFile(vm_conf_file);

    fprintf(stdout,"\nchar *jemVmGetExec(params,\"javah\") ->\n");
    char *exec;
    if((exec = jemVmGetExec(params,"javah"))) {
        fprintf(stdout,"exec = %s\n",exec);
        free(exec);
    }

    fprintf(stdout,"\nchar *jemVmGetExec(params,\"your_momma\") ->\n");
    if((exec = jemVmGetExec(params,"your_momma"))) {
        fprintf(stdout,"exec = %s\n",exec);
        free(exec);
    }

//    fprintf(stdout,"\nchar *jemVmGetName(struct vm *vm) ->\n%s\n",jemVmGetName(params));

    fprintf(stdout,"\nchar *jemVmGetProvidesType(struct params *params) ->\n%s\n",jemVmGetProvidesType(params));

    fprintf(stdout,"\nchar *JemVmGetVersion(struct params *params) ->\n%s\n",jemVmGetVersion(params));

    fprintf(stdout,"\nbool jemVmIsBuildOnly(struct params *params) ->\n");
    if(jemVmIsBuildOnly(params))
        fprintf(stdout,"true\n");
    else
        fprintf(stdout,"false\n");

    fprintf(stdout,"\nbool jemVmIsType(params,\"JDK\") ->\n");
    if(jemVmIsType(params,"JDK"))
        fprintf(stdout,"true\n");
    else
        fprintf(stdout,"false\n");

    fprintf(stdout,"\nbool jemVmIsJDK(struct *params) ->\n");
    if(jemVmIsJDK(params))
        fprintf(stdout,"Yes its a JDK\n");

    fprintf(stdout,"\nbool jemVmIsJRE(struct params *params) ->\n");
    if(jemVmIsJRE(params))
        fprintf(stdout,"Yes its a JRE\n");

    fprintf(stdout,"\nchar **jemPkgGetProvides(struct params *params) ->\n");
    char **provides = jemPkgGetProvides(params);
    if(provides) {
        int i;
        for(i=0;provides[i];i++) {
            fprintf(stdout,"\t%s\n",provides[i]);
            free(provides[i]);
        }
        free(provides);
    }

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    jemFreeParams(params);
}

void testEnvManager() {
    fprintf(stdout,"\nTesting env_manager.h functions\n");
    int i;

    fprintf(stdout,"\nstruct pkg *pkgs;\n");
    struct jem_pkg *pkgs;

    fprintf(stdout,"\npkgs = loadPackages(false);\n");
    pkgs = jemPkgLoadPackages(false);

    for(i=0;pkgs[i].filename;i++) {
        fprintf(stdout,"\tpkgs[%d]->filename=%s\n",i,pkgs[i].filename);
        fprintf(stdout,"\tpkgs[%d]->name=%s\n",i,pkgs[i].name);
    }

    fprintf(stdout,"\nvoid freePkgs(struct pkg *pkgs)\n");
    jemFreePkgs(pkgs);

    fprintf(stdout,"\npkgs = loadPackages(true);\n");
    pkgs = jemPkgLoadPackages(true);

    if(pkgs) {
        for(i=0;pkgs[i].filename;i++) {
            fprintf(stdout,"\tpkgs[%d]->filename=%s\n",i,pkgs[i].filename);
            fprintf(stdout,"\tpkgs[%d]->name=%s\n",i,pkgs[i].name);
        }
    } else
	jemPrintError("^ Test failed!\nUnable to load virtual packages");

    fprintf(stdout,"\nvoid freePkgs(struct pkg *pkgs)\n");
    jemFreePkgs(pkgs);

    fprintf(stdout,"\nchar *getSystemVMLink() ->\n%s\n",jemVmGetSystemVMLink());

    char *vm_name = jemVmGetSystemVMName();
    fprintf(stdout,"\nchar *getSystemVMName() ->\n%s\n",vm_name);
    free(vm_name);

    vm_name = jemVmGetUserVMLink();
    fprintf(stdout,"\nchar *getUserVMLink() ->\n%s\n",vm_name);
    free(vm_name);

    fprintf(stdout,"\nchar **getVMLinks() ->\n");
    char **links = jemVmGetVMLinks();
    if(links) {
        for(i=0;links[i];i++)
            fprintf(stdout,"\t%s\n",links[i]);
    }
    fprintf(stdout,"\nvoid freeVMLinks(char **vm_links)\n");
    jemFreeVMLinks(links);

    fprintf(stdout,"\nstruct vm *vms;\n");
    struct jem_vm *vms;

    fprintf(stdout,"\nvms = loadVMs();\n");
    vms = jemVmLoadVMs();
    if(vms) {
        for(i=0;vms[i].filename;i++)
            fprintf(stdout,"\tvms[%d]->filename=%s\n",i,vms[i].filename);
    } else
	jemPrintError("^ Test failed!\nUnable to load VMs");

    fprintf(stdout,"\nstruct vm *vm;\n");
    struct jem_vm *vm;

    fprintf(stdout,"\nstruct vm *getVM(struct vm *vms,const char *vm_name);\n");
    fprintf(stdout,"\nvm = getVM(vms,\"2\") ->\n");
    vm = jemVmGetVM(vms,"2");
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");

    fprintf(stdout,"\nvm = getVM(vms,\"%s\") ->\n",jvm);
    vm = jemVmGetVM(vms,jvm);
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");

    fprintf(stdout,"\nvm = getVM(vms,\"%s\") ->\n",vm_conf_file);
    vm = jemVmGetVM(vms,vm_conf_file);
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");

    fprintf(stdout,"\nvm = getVM(vms,\"%s\") ->\n",vm_home);
    vm = jemVmGetVM(vms,vm_home);
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");

    fprintf(stdout,"\nvoid freeVMs(struct vm *vms)\n");
    jemFreeVMs(vms);

    fprintf(stdout,"\nstruct env env;\n");
    struct jem_env env;

    fprintf(stdout,"\ninitEnv(&env)\n");
    jemInitEnv(&env);

    fprintf(stdout,"\nenv.vms = loadVMs()\n");
    env.vms = jemVmLoadVMs();

    fprintf(stdout,"\nstruct vm *avm = getActiveVM(&env);\n");
    struct jem_vm *avm = jemGetActiveVM(&env);

    if(avm)
        fprintf(stdout,"\navm->filename =%s\n",avm->filename);

    fprintf(stdout,"\nvoid freeEnv(struct env *env)\n");
    jemFreeEnv(&env);

    fprintf(stdout,"\nstruct jem_vm *found = jemFindVM(\"%s\");\n",jvm);
    struct jem_vm **found = jemFindVM(jvm);
    if(found) {
        for(i=0;found[i];i++)
            fprintf(stdout,"\tvms[%d]->filename=%s\n",i,found[i]->filename);
        free(found);
    }

    fprintf(stdout,"\ncleanup()\n");
    jemCleanup();
}

int main(int argc, char **argv) {

    if(argc<5) {
        fprintf(stdout,"Missing args <vm> <vm_home> <vm_conf_file> <pkg_env_file>\n");
        exit(EXIT_FAILURE);
    }

    jvm = argv[1];
    vm_home = argv[2];
    vm_conf_file = argv[3];
    pkg_env_file = argv[4];

    fprintf(stdout,"\n\\********** Starting jem tests **********\\\n\n");

    testOutputFormatter();
    testFileParser();
    testPackage();
    testVM();
    testEnvManager();

    fprintf(stdout,"\n\\********** Finished jem tests **********\\\n\n");

    exit(EXIT_SUCCESS);
}
