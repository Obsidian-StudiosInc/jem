# Copyright 2015-2018 Obsidian-Studios, Inc.
# Distributed under the terms of the GNU General Public License, v3 or later

# If we have a current-user-vm (and aren't root)... set it to JAVA_HOME
user_vm="${HOME}/.java/vm"
system_vm="/etc/jem/vm"

# Please make sure that this script is POSIX compliant
# See https://bugs.gentoo.org/show_bug.cgi?id=169925
# for more details"

if [ -z "${UID}" ] ; then
	# id lives in /usr/bin which might not be mounted
	if type id >/dev/null 2>/dev/null ; then
		user_id=$(id -u)
	else
		[ "${USER}" = "root" ] && user_id=0
	fi
fi

# The root user uses the system vm
if [ "${user_id}" != 0 -a -L "${user_vm}" ]; then
	export JAVA_HOME=${user_vm}
# Otherwise set to the current system vm
elif [ -L "/etc/jem/vm" ]; then
	export JAVA_HOME=${system_vm}
fi

export MANPATH="${JAVA_HOME}/man:${MANPATH}"
export JDK_HOME=${JAVA_HOME}
export JAVAC=${JDK_HOME}/bin${HOME}c
unset user_vm system_vm user_id
