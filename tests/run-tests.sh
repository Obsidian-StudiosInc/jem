#!/bin/bash
# runs each command of jem through valgrind as well as the tests binary
# run from root project directory, or will error

VG="/usr/bin/valgrind --leak-check=yes --leak-check=full --read-var-info=yes"
VG="${VG} --show-reachable=yes --track-origins=yes --error-exitcode=1"
VM="${1}"

JEM_TEST="$(find .  | grep 'dist/jem-test')"
JEM="${JEM_TEST%%-*}"

[[ "${CI}" ]] && export COLORS=1

check_rc() {
	[[ ${1} -ne 0 ]] && exit "${1}"
}

test_code() {
	${VG} "${JEM_TEST}" "$@"
	check_rc $?
}

test_jem() {

	if [[ "${CI}" ]]; then
		if [[ ! "${SHIPPABLE}" ]]; then
			${VG} "${JEM}" -s "${VM}"
			check_rc $?

			# shellcheck disable=SC2086
			sudo ${VG} "${JEM}" -S "${VM}"
			check_rc $?
		else
			${VG} "${JEM}" -S "${VM}"
			check_rc $?
		fi
	fi

	ARGS="c f j J l L o O t v ? V"

	for arg in ${ARGS}; do
		${VG} "${JEM}" -"${arg}"
		check_rc $?
	done

	${VG} "${JEM}" -a "${VM}" -g JAVA_HOME
	check_rc $?

	${VG} "${JEM}" -e javac
	check_rc $?

	${VG} "${JEM}" -g LDPATH
	check_rc $?

	${VG} "${JEM}" -i jna
	check_rc $?

	${VG} "${JEM}" -dp xom
	check_rc $?

	${VG} "${JEM}" --package xom -q DEPEND
	check_rc $?

	${VG} "${JEM}" -P "${VM}"
	check_rc $?
}

case "$1" in

	-c | --code)
		shift
		test_code "$@"
		;;

	-j | --jem)
		test_jem
		;;

	*)
		test_jem
		test_code "$@"
		;;

esac

exit 0
