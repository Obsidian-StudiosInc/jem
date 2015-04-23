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
#include "../include/package.h"

bool with_dependencies = false;

/**
 * Frees the allocated memory used by a dep struct
 *
 * @param pkgs a pointer to a pkg struct
 */
void freeDep(struct dep *dep) {
    if(!dep)
        return;
    if(dep->name)
         free(dep->name);
    if(!dep->jars)
        return;
    int i;
    for(i=0;dep->jars[i];i++)   // <- ugly, nasty, etc but works! :)
        free(dep->jars[i]);
    free(dep->jars);
}

/**
 * Frees the allocated memory used by a pkg struct
 *
 * @param pkgs a pointer to a pkg struct
 */
void freePkg(struct pkg *pkg) {
    if(!pkg)
        return;
    if(pkg->filename)
         free(pkg->filename);
    if(pkg->name)
         free(pkg->name);
    freeParams(pkg->params);
}

/**
 * Frees the allocated memory used by an array of pkg structs
 *
 * @param pkgs a pointer an array of pkg structs
 */
void freePkgs(struct pkg *pkgs) {
    if(!pkgs)
        return;
    int i;
    for(i=0;pkgs[i].filename;i++)   // <- ugly, nasty, etc but works! :)
        freePkg(&pkgs[i]);
    free(pkgs);
}

/**
 * Get providers/packages for one or more virtual package(s)
 *
 * @param name string containing the name(s) of the virtual package(s), 
 *        multiple comma separated virtual package names can be specified
 * @return a string containing the value. The string must be freed!
 */
const char *gjpGetVirtualProviders(const char *virtual) {
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
 * Get active package for virtual
 *
 * @param name string containing the name of the virtual
 * @return a string containing the value. The string must be freed!
 */
const char *gjpGetActiveVirtualProvider(const char *virtual) {
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
 * Compares the names of two packages, used soley by qsort in loadPackages()
 *
 * @return an integer -1, 0, or 1.
 */

int packageCompare(const struct pkg *p1, const struct pkg *p2) {
    if(p1->name && p2->name)
        return strcmp (p1->name, p2->name);
    return(-1);
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
        qsort(pkgs,i+2,sizeof(struct pkg),packageCompare);
    closedir(dp);
    return(pkgs);
}

/**
 * Get a package's description
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjpGetDescription(struct param *params) {
    char *desc = getValue(params,"DESCRIPTION");
    if(desc)
        return(desc);
    return("No Description");
}

/**
 * Get a package's classpath
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must NOT be freed!
 */
char *gjpGetClasspath(struct param *params) {
    return(getValue(params,"CLASSPATH"));
}

/**
 * Get a package's dependencies, internal function called buy wrappers
 *
 * @param params an array of param structs
 * @name string containing the variable name, DEPEND/BUILD_DEPEND/OPTIONAL_DEPEND
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *__gjpGetDeps(struct dep *deps,
                         struct param *params,
                         char *name) {
    char *value = getValue(params,name);
    if(!value)
        return(deps);
    char *dep_name = NULL;
    char *deps_str = calloc(strlen(value)+1,sizeof(char));
    char *cursor = deps_str;
    memcpy(cursor,value,strlen(value));
    int i = 0;
    OUTER:while(dep_name = strsep(&cursor,":")) {
        bool has_jar = false;
        char *pkg_name = dep_name;
        if(pkg_name = strstr(pkg_name,"@")) {
            pkg_name++; // skip @
            has_jar = true;
        } else 
            pkg_name = dep_name; // re-assign since null
        if(deps) {
            for(i=0;deps[i].name;i++) {
                if(strcmp(deps[i].name,pkg_name)==0)
                    goto ADDJARS;
            }
        }
        struct dep *tmp = realloc(deps,sizeof(struct dep)*(i+2));
        if(!tmp)
            printError("Unable to allocate memory to hold all dependencies"); // needs to clean up and exit under error, not just print a message
        deps = tmp;
        deps[i+1].name = NULL;
        deps[i+1].jars = NULL;
        deps[i+1].parsed_sub_deps = false;
        deps[i].jars = NULL;
        asprintf(&(deps[i].name),"%s",pkg_name);
        if(!deps[i].name)
            printError("Unable to allocate memory to hold dependency name"); // needs to clean up and exit under error, not just print a message
        deps[i].parsed_sub_deps = false;
        ADDJARS:
        if(!has_jar)
            continue;
        memset(dep_name+strlen(dep_name)-strlen(pkg_name)-1,'\0',1);
        int j = 0;
        bool jar_added = false;
        if(deps[i].jars) {
            for(j=0;deps[i].jars[j];j++) {
                if(strcmp(deps[i].jars[j],dep_name)==0)
                    jar_added = true;
            }
        }
        if(jar_added)
            continue;
        char **tmp_jars = realloc(deps[i].jars,sizeof(char *)*(j+2));
        if(!tmp_jars)
            printError("Unable to allocate memory to hold all dependency jars"); // needs to clean up and exit under error, not just print a message
        deps[i].jars = tmp_jars;
        deps[i].jars[j+1]= NULL;
        asprintf(&(deps[i].jars[j]),"%s",dep_name);
        if(!deps[i].jars[j])
            printError("Unable to allocate memory to hold dependency jar"); // needs to clean up and exit under error, not just print a message
    }
    if(deps) {
        for(i=0;deps[i].name && !deps[i].parsed_sub_deps;i++) {
            deps[i].parsed_sub_deps = true;
            struct pkg *pkg = loadPackage(deps[i].name);
            if(pkg) {
                deps = __gjpGetDeps(deps,pkg->params,name);
                freePkg(pkg);
                free(pkg);
            }
        }
    }
    free(deps_str);
    return(deps);
}

/**
 * Get a package's dependencies
 *
 * @param params an array of param structs
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *gjpGetDeps(struct param *params) {
    struct dep *deps = NULL;
    return(__gjpGetDeps(deps,params,"DEPEND"));
}

/**
 * Get a package's build dependencies
 *
 * @param params an array of param structs
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *gjpGetBuildDeps(struct param *params) {
    struct dep *deps = NULL;
    return(__gjpGetDeps(deps,params,"BUILD_DEPEND"));
}

/**
 * Get a package's optional dependencies
 *
 * @param params an array of param structs
 * @return an array of dep structs. Which must be freed, including struct members!
 */
struct dep *gjpGetOptDeps(struct param *params) {
    struct dep *deps = NULL;
    return(__gjpGetDeps(deps,params,"OPTIONAL_DEPEND"));
}

/**
 * Get the packages this package provides
 *
 * @param params an array of param structs
 * @return an array of strings containing the value. The array and strings must be freed!
 */
char **gjpGetProvides(struct param *params) {
    char *value = getValue(params,"PROVIDES");
    if(!value)
        return(NULL);
    char *provide = NULL;
    char **provides = NULL;
    char *provides_str = calloc(strlen(value)+1,sizeof(char));
    char *cursor = provides_str;
    memcpy(cursor,value,strlen(value));
    int i;
    for(i=0;(provide = strsep(&cursor," "));i++) {
        char **tmp = realloc(provides,sizeof(char *)*(i+2));
        if(!tmp)
            printError("Unable to allocate memory to hold all provides"); // needs to clean up and exit under error, not just print a message
        provides = tmp;
        provides[i+1] = NULL;
        int plen = strlen(provide);
        provides[i] = calloc(plen+1,sizeof(char));
        memcpy(provides[i],provide,plen);
    }
    free(provides_str);
    return(provides);
}

/**
 * Get a package's target
 *
 * @param params an array of param structs
 * @return a string containing the value. The string must be freed!
 */
char *gjpGetTarget(struct param *params) {
    return(getValue(params,"TARGET"));
}
