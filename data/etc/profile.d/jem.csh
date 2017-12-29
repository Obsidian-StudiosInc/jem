# Copyright 2015 Obsidian-Studios, Inc.
# Distributed under the terms of the GNU General Public License, v3 or later

set user_vm = "${HOME}/.java/vm"
set system_vm = "/etc/jem/vm"

## If we have a current-user-vm (and aren't root)... set it to JAVA_HOME
## Otherwise set to the current system vm
if ( ( "$uid" != "0" ) && ( -l $user_vm ) ) then
    setenv JAVA_HOME $user_vm
else if ( -l $system_vm ) then
    setenv JAVA_HOME $system_vm
endif
unset user_vm system_vm

if ( $?JAVA_HOME ) then
	setenv MANPATH "${JAVA_HOME}/man:${MANPATH}"
	setenv JDK_HOME $JAVA_HOME
	setenv JAVAC ${JDK_HOME}/bin${HOME}c
endif
