#!/bin/bash
# runs each command of jem through valgrind as well as the tests binary
# run from root project directory, or will error

VG="/usr/bin/valgrind --leak-check=yes --leak-check=full --read-var-info=yes"
VG="${VG} --show-reachable=yes --track-origins=yes --error-exitcode=1"
VM="oraclejdk-7"

check_rc() {
	[[ ${1} -ne 0 ]] && exit "${1}"
}

test_code() {
	pwd
	ls
	${VG} ./dist/jem-test
	check_rc $?
}

test_jem() {
	${VG} ./dist/jem -S 1
	check_rc $?

	ARGS="c f j J l L o O r t v ? V"

	for arg in ${ARGS}; do
		${VG} ./dist/jem -"${arg}"
		check_rc $?
	done

	${VG} ./dist/jem -a ${VM} -g JAVA_HOME
	check_rc $?

	${VG} ./dist/jem -e javac
	check_rc $?

	${VG} ./dist/jem -g LDPATH
	check_rc $?

	${VG} ./dist/jem -i jna
	check_rc $?

	${VG} ./dist/jem -dp tomcat-server
	check_rc $?

	${VG} ./dist/jem --package tomcat-server -q DEPEND
	check_rc $?

	${VG} ./dist/jem -P ${VM}
	check_rc $?
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

exit 0
