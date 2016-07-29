#!/bin/bash
# runs each command of jem through valgrind as well as the tests binary

VG="/usr/bin/valgrind --leak-check=yes --leak-check=full --read-var-info=yes"
VG="${VG} --show-reachable=yes --track-origins=yes"

cd ../dist

${VG} jem-test

ARGS="c f j J l L o O r t v ? V"

for arg in ${ARGS}; do
	${VG} jem -${arg}
	rc=$?
	[[ ${rc} -ne 0 ]] && exit ${rc}
done

${VG} jem -g LDPATH
