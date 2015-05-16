#!/bin/bash

PKGS=$(ls /usr/share/*/package.env | cut -d '/' -f 4)

JEMO=/tmp/jem_output
JCO=/tmp/java-config_output

echo "Comparing"
echo "jem -d -p pkg"
echo "java-config -d -p pkg"
echo ""

for pkg in ${PKGS}; do

	jem -d -p ${pkg} | tr ':' '\n' | sort > ${JEMO}
	java-config -d -p ${pkg} | tr ':' '\n' | sort > ${JCO}

	output=$(diff -Naur ${JCO} ${JEMO})
	if [[ -n ${output} ]] ; then

		echo -e "${pkg} \x1B[0;31mFailed\x1B[0m"
		echo "${output}"

	else
		echo -e "${pkg} \x1B[0;32mPassed\x1B[0m"

	fi

#	echo ""

done
