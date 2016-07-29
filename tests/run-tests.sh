#!/bin/bash
# runs each command of jem through valgrind as well as the tests binary
# run from root project directory, or will error

VG="/usr/bin/valgrind --leak-check=yes --leak-check=full --read-var-info=yes"
VG="${VG} --show-reachable=yes --track-origins=yes"

check_rc() {
	[[ ${1} -ne 0 ]] && exit ${1}
}

test_code() {
	pwd
	ls
	${VG} ./dist/jem-test
}
test_jem() {

	ARGS="c f j J l L o O r t v ? V"

	for arg in ${ARGS}; do
		${VG} ./dist/jem -${arg}
		check_rc $?
	done

	${VG} ./dist/jem -g LDPATH
}
case "$1" in

	-c | --code)
		test_code
		;;

	-j | --jem)
		test_jem
		;;

	*)
		test_code
		test_jem
		;;

esac

