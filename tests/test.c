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

#include <stdio.h>

#include "../include/env_manager.h"

#define VM_CONF_FILE "/usr/share/java-config-2/vm/icedtea-bin-7"

testOutputFormatter() {
    int i;

    fprintf(stdout,"Testing output_formatter.h functions\n\n"
                   "char *getCode(const char *key) output effects next terminal line\n");
    for(i=0;i<codes_count;i++)
        fprintf(stdout,"getCode('%c')%s\n",codes[i].key,getCode(codes[i].key));

    fprintf(stdout,"\nbool isValidTerm()\n");
    if(isValidTerm())
        fprintf(stdout,"Yes valid terminal\n");
    else
        fprintf(stdout,"No invalid terminal\n");

    fprintf(stdout,"\nchar *addColor(const char *key)\n");
    for(i=0;i<codes_count;i++) {
        char msg[10];
        memset(msg,'%',1);
        memset(msg+1,codes[i].key,1);
        memcpy(msg+2," text %$",8);
        memset(msg+10,'\0',1);
        char *nmsg = addColor(msg);
        fprintf(stdout,"addColor(%s) -> %s\n",msg,nmsg);
        free(nmsg);
    }

    fprintf(stdout,"\nchar *indent(const char *prefix,const char *msg)\n");
    char *imsg = indent("Preffix : ","A message that does not repeat (no indent test)");
    fprintf(stdout,"\nindent(\"Preffix : \",\"A message that does not repeat\") ->\n%s\n",imsg);
    free(imsg);

    imsg = indent("Preffix : ","An indented message that repeats 1\n"
                               "An indented message that repeats 2\n"
                               "An indented message that repeats 3");
    fprintf(stdout,"\nindent(\"Preffix : \",\"An indented message that repeats 1\n%s%s\") ->\n%s\n",
                   "An indented message that repeats 2\n",
                   "An indented message that repeats 3",imsg);
    free(imsg);

    imsg = indent("!!! Alert : ","Printing an alert that spans\na few lines so we get some\nline indenting\n");
    fprintf(stdout,"\nindent(\"!!! Alert : \",\"Printing an alert that spans\na few lines so we get some\nline indenting\n\") ->\n%s",imsg);
    free(imsg);

    fprintf(stdout,"\nvoid printAlert(const char *msg)\n");
    printAlert("Printing an alert that spans\na few lines so we get some\nline indenting\n");

    fprintf(stdout,"void printError(const char *msg)\n");
    printError("Printing an error that spans\na few lines so we get some\nline indenting\n");

    fprintf(stdout,"void printWarning(const char *msg)\n");
    printWarning("Printing a warning that spans\na few lines so we get some\nline indenting\n");

}

testFileParser() {
    fprintf(stdout,"\nTesting file_parser.h functions\n");
    int i;
    fprintf(stdout,"\nstruct param *params;\n");
    struct param *params = NULL;

    fprintf(stdout,"\nparse vm config (also tests/fills in variables in values)");
    fprintf(stdout,"\nparams = parseFile(\"%s\"); ->\n",VM_CONF_FILE);
    params = parseFile(VM_CONF_FILE);
    for(i=0;params[i].name;i++)
        fprintf(stdout,"\tparams[%d]->name=%s\n\tparams[%d]->value=%s\n",i,params[i].name,i,params[i].value);

    fprintf(stdout,"\ngetValue(params,\"BOOTCLASSPATH\") ->\n%s\n",getValue(params,"BOOTCLASSPATH"));

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    freeParams(params);
    
    fprintf(stdout,"\nparse package.env");
    fprintf(stdout,"\nparams = parseFile(\"/usr/share/ant-core/package.env\"); ->\n");
    params = parseFile("/usr/share/ant-core/package.env");
    for(i=0;params[i].name;i++)
        fprintf(stdout,"\tparams[%d]->name=%s\n\tparams[%d]->value=%s\n",i,params[i].name,i,params[i].value);

    fprintf(stdout,"\nchar *getValue(struct param *params,const char *name) ->\n");

    fprintf(stdout,"\ngetValue(params,\"MERGE_VM\") ->\n%s\n",getValue(params,"MERGE_VM"));

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    freeParams(params);

}

testPackage() {
    fprintf(stdout,"\nTesting package.h functions\n");
    fprintf(stdout,"\nstruct param *params;\n");
    struct param *params = NULL;

    fprintf(stdout,"\nparams = parseFile(\"/usr/share/idontexist/package.env\");\n");
    params = parseFile("/usr/share/idontexist/package.env");

    fprintf(stdout,"\nparams = parseFile(\"/usr/share/cglib-2.2/package.env\");\n");
    params = parseFile("/usr/share/cglib-2.2/package.env");

    fprintf(stdout,"\nchar *gjpGetDescription(struct params *params) ->\n%s\n",gjpGetDescription(params));
    fprintf(stdout,"\nchar *gjpGetClasspath(struct params *params) ->\n%s\n",gjpGetClasspath(params));

    fprintf(stdout,"\nstruct dep *gjpGetDeps(struct params *params) ->\n");
    struct dep *deps = gjpGetDeps(params);
    if(deps) {
        int i;
        for(i=0;deps[i].name;i++) {
            fprintf(stdout,"\t%s\n",deps[i].name);
            int j;
            if(deps[i].jars)
                for(j=0;deps[i].jars[j];j++)
                    fprintf(stdout,"\t\t%s\n",deps[i].jars[j]);
            freeDep(&deps[i]);
        }
        free(deps);
    }

    fprintf(stdout,"\nstruct dep *gjpGetBuildDeps(struct params *params) ->\n");
    struct dep *build_deps = gjpGetBuildDeps(params);
    if(build_deps) {
        int i;
        for(i=0;build_deps[i].name;i++) {
            fprintf(stdout,"\t%s\n",build_deps[i].name);
            int j;
            if(build_deps[i].jars)
                for(j=0;build_deps[i].jars[j];j++)
                    fprintf(stdout,"\t\t%s\n",build_deps[i].jars[j]);
            freeDep(&build_deps[i]);
        }
        free(build_deps);
    }

    fprintf(stdout,"\nstruct dep *gjpGetOptDeps(struct params *params) ->\n");
    struct dep *opt_deps = gjpGetOptDeps(params);
    if(opt_deps) {
        int i;
        for(i=0;opt_deps[i].name;i++) {
            fprintf(stdout,"\t%s\n",opt_deps[i].name);
            int j;
            if(opt_deps[i].jars)
                for(j=0;opt_deps[i].jars[j];j++)
                    fprintf(stdout,"\t\t%s\n",opt_deps[i].jars[j]);
            freeDep(&opt_deps[i]);
        }
        free(opt_deps);
    }

    fprintf(stdout,"\nDoesn't seem like any package.env file has this yet?");
    fprintf(stdout,"\nchar **gjpGetProvides(params) ->\n");

    char **provides = gjpGetProvides(params);
    if(provides) {
        int i;
        for(i=0;provides[i];i++) {
            fprintf(stdout,"\t%s\n",provides[i]);
            free(provides[i]);
        }
        free(provides);
    }

    fprintf(stdout,"\nchar *gjpGetTarget(struct params *params) -> %s\n",gjpGetTarget(params));

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    freeParams(params);

    fprintf(stdout,"\nconst char *gjpGetVirtualProviders(\"javamail\",true)->\n");
    char *virtual = gjpGetVirtualProviders("javamail",true);
    fprintf(stdout,"%s\n",virtual);
    free(virtual);

    fprintf(stdout,"\nconst char *gjpGetActiveVirtualProvider(\"javamail\")->\n");
    virtual = gjpGetActiveVirtualProvider("javamail");
    fprintf(stdout,"%s\n",virtual);
    free(virtual);

}

testVM() {
    fprintf(stdout,"\nTesting vm.h functions\n");
    fprintf(stdout,"\nstruct param *params;\n");
    struct param *params = NULL;

    fprintf(stdout,"\nparams = parseFile(\"%s\");\n",VM_CONF_FILE);
    params = parseFile(VM_CONF_FILE);

    fprintf(stdout,"\nchar *gjvmGetExec(params,\"javah\") ->\n");
    char *exec;
    if((exec = gjvmGetExec(params,"javah"))) {
        fprintf(stdout,"exec = %s\n",exec);
        free(exec);
    }

    fprintf(stdout,"\nchar *gjvmGetExec(params,\"your_momma\") ->\n");
    if((exec = gjvmGetExec(params,"your_momma"))) {
        fprintf(stdout,"exec = %s\n",exec);
        free(exec);
    }

//    fprintf(stdout,"\nchar *gjvmGetName(struct vm *vm) ->\n%s\n",gjvmGetName(params));
    
    fprintf(stdout,"\nchar *gjvmGetProvidesType(struct params *params) ->\n%s\n",gjvmGetProvidesType(params));

    fprintf(stdout,"\nchar *gjvmGetVersion(struct params *params) ->\n%s\n",gjvmGetVersion(params));

    fprintf(stdout,"\nbool gjvmIsBuildOnly(struct params *params) ->\n");
    if(gjvmIsBuildOnly(params))
        fprintf(stdout,"true\n");
    else
        fprintf(stdout,"false\n");

    fprintf(stdout,"\nbool gjvmIsType(params,\"JDK\") ->\n");
    if(gjvmIsType(params,"JDK"))
        fprintf(stdout,"true\n");
    else
        fprintf(stdout,"false\n");

    fprintf(stdout,"\nbool gjvmIsJDK(struct *params) ->\n");
    if(gjvmIsJDK(params))
        fprintf(stdout,"Yes its a JDK\n");

    fprintf(stdout,"\nbool gjvmIsJRE(struct params *params) ->\n");
    if(gjvmIsJRE(params))
        fprintf(stdout,"Yes its a JRE\n");

    fprintf(stdout,"\nchar **gjpGetProvides(struct params *params) ->\n");
    char **provides = gjpGetProvides(params);
    if(provides) {
        int i;
        for(i=0;provides[i];i++) {
            fprintf(stdout,"\t%s\n",provides[i]);
            free(provides[i]);
        }
        free(provides);
    }

    fprintf(stdout,"\nvoid freeParams(struct params *params)\n");
    freeParams(params);
}

testEnvManager() {
    fprintf(stdout,"\nTesting env_manager.h functions\n");
    int i;

    fprintf(stdout,"\nstruct pkg *pkgs;\n");
    struct pkg *pkgs;
 
    fprintf(stdout,"\npkgs = loadPackages(false);\n");
    pkgs = loadPackages(false);
    
    for(i=0;pkgs[i].filename;i++) {
        fprintf(stdout,"\tpkgs[%d]->filename=%s\n",i,pkgs[i].filename);
        fprintf(stdout,"\tpkgs[%d]->name=%s\n",i,pkgs[i].name);
    }

    fprintf(stdout,"\nvoid freePkgs(struct pkg *pkgs)\n");
    freePkgs(pkgs);

    fprintf(stdout,"\npkgs = loadPackages(true);\n");
    pkgs = loadPackages(true);
    
    for(i=0;pkgs[i].filename;i++) {
        fprintf(stdout,"\tpkgs[%d]->filename=%s\n",i,pkgs[i].filename);
        fprintf(stdout,"\tpkgs[%d]->name=%s\n",i,pkgs[i].name);
    }

    fprintf(stdout,"\nvoid freePkgs(struct pkg *pkgs)\n");
    freePkgs(pkgs);
    
    fprintf(stdout,"\nchar *getSystemVMLink() ->\n%s\n",getSystemVMLink());

    char *vm_name = getSystemVMName();
    fprintf(stdout,"\nchar *getSystemVMName() ->\n%s\n",vm_name);
    free(vm_name);

    vm_name = getUserVMLink();
    fprintf(stdout,"\nchar *getUserVMLink() ->\n%s\n",vm_name);
    free(vm_name);

    fprintf(stdout,"\nchar **getVMLinks() ->\n");
    char **links = getVMLinks();
    if(links) {
        for(i=0;links[i];i++)
            fprintf(stdout,"\t%s\n",links[i]);
    }
    fprintf(stdout,"\nvoid freeVMLinks(char **vm_links)\n");
    freeVMLinks(links);

    fprintf(stdout,"\nstruct vm *vms;\n");
    struct vm *vms;
 
    fprintf(stdout,"\nvms = loadVMs();\n");
    vms = loadVMs();
    for(i=0;vms[i].filename;i++)
        fprintf(stdout,"\tvms[%d]->filename=%s\n",i,vms[i].filename);

    fprintf(stdout,"\nstruct vm *vm;\n");
    struct vm *vm;

    fprintf(stdout,"\nstruct vm *getVM(struct vm *vms,const char *vm_name);\n");
    fprintf(stdout,"\nvm = getVM(vms,\"2\") ->\n");
    vm = getVM(vms,"2");
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");
    
    fprintf(stdout,"\nvm = getVM(vms,\"/usr/share/jem-2/vm/icedtea-bin-7\") ->\n");
    vm = getVM(vms,"/usr/share/jem-2/vm/icedtea-bin-7");
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");
    
    fprintf(stdout,"\nvm = getVM(vms,\"icedtea-bin-7\") ->\n");
    vm = getVM(vms,"icedtea-bin-7");
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");

    fprintf(stdout,"\nvm = getVM(vms,\"/usr/lib/jvm/icedtea-bin-7\") ->\n");
    vm = getVM(vms,"/usr/lib/icedtea-bin-7");
    if(vm)
        fprintf(stdout,"vm->filename=%s\n",vm->filename);
    else
        fprintf(stdout,"VM pointer is null\n");

    fprintf(stdout,"\nvoid freeVMs(struct vm *vms)\n");
    freeVMs(vms);

    fprintf(stdout,"\nstruct env env;\n");
    struct env env;
    
    fprintf(stdout,"\ninitEnv(&env)\n");
    initEnv(&env);
    
    fprintf(stdout,"\nenv.vms = loadVMs()\n");
    env.vms = loadVMs();
    
    fprintf(stdout,"\nstruct vm *avm = getActiveVM(&env);\n");
    struct vm *avm = getActiveVM(&env);
    
    if(avm)
        fprintf(stdout,"\navm->filename =%s\n",avm->filename);

    fprintf(stdout,"\nvoid freeEnv(struct env *env)\n");
    freeEnv(&env);
}

int main(int argc, char **argv) {

    fprintf(stdout,"\n\\********** Starting jem tests **********\\\n\n");

    testOutputFormatter();
    testFileParser();
    testPackage();
    testVM();
    testEnvManager();

    fprintf(stdout,"\n\\********** Finished jem tests **********\\\n\n");

    exit(EXIT_SUCCESS);
}
