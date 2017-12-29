#!/bin/bash
# Copyright 2016 Obsidian-Studios, Inc.
# Distributed under the terms of the GNU General Public License v3
#
# Original work Copyright 1999-2016 Gentoo Foundation

tool=${0##*/}

if [[ "${tool}" = "javaws" ]] && [[ -x "/usr/bin/itweb-javaws" ]]; then
	exec "/usr/bin/itweb-javaws" "${@}"
fi

user_vm="${HOME}/.java/vm"
system_vm="/etc/jem/vm"

# Try JEM_VM
if [[ -n "${JEM_VM}" ]]; then
	vmpath="/usr/lib/jvm/${JEM_VM}"
# Then user VM
elif [[ -h "${user_vm}" ]]; then
	vmpath=$(readlink "${user_vm}")
# And fall back to the system VM
else
	vmpath=$(readlink "${system_vm}")
fi

vm_handle=${vmpath##*/}

toolpath=$(
	export PATH=
        # shellcheck disable=SC1090
	. "/etc/jem/vms.d/${vm_handle}" 2> /dev/null
	: ${PATH:=${vmpath}/bin:${vmpath}/jre/bin}
	command -v "${tool}" 2> /dev/null
)

if [[ -x "${toolpath}" ]]; then
	exec "${toolpath}" "${@}"
else
	if [[ ! -d "${vmpath}" ]]; then
		echo "* Home for VM '${vm_handle}' does not exist: ${vmpath}" >&2
		if [[ -n "${JEM_VM}" ]]; then
			echo "* Invalid value for JEM_VM: ${JEM_VM}"
		elif [[ -h "${user_vm}" ]]; then
			echo "* Invalid User VM: ${vm_handle}" >&2
		else
			echo "* Invalid System VM: ${vm_handle}" >&2
		fi
	else
		if [[ "${tool}" = "run-java-tool.bash" ]]; then
			echo "* run-java-tool was invoked directly" >&2
			echo "* run-java-tool should only be used via symlinks to it" >&2
		else
			echo "* ${tool} is not available for ${vm_handle} on $(uname -m)" >&2
			echo "* IMPORTANT: some Java tools are not available on some VMs on some architectures" >&2
			if [[ "$(id -u)" != 0 ]] && [[ -n "${DISPLAY}" ]] && command -v notify-send > /dev/null; then
				notify-send -i java-icon48 "Java Launcher" "<b>${tool}</b> is not available for ${vm_handle}"
			fi
		fi
	fi
	exit 1
fi
