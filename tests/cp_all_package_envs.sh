#!/bin/bash
#
# Copy all package.env files from a system running to testing dir
# run from tests dir in jem sources

PKG_ENVS=$(ls /usr/share/*/package.env)

for pkg_env in $PKG_ENVS; do

	DIR="../samples$(dirname ${pkg_env})"
	[[ ! -d ${DIR} ]] && mkdir -p ${DIR}
	[[ ! -d ${DIR}/lib ]] && mkdir -p ${DIR}/lib
	[[ ! -f "../samples${pkg_env}" ]] && cp -v ${pkg_env} ${DIR}

	# create fake jars, just need names not the contents for tests
	pushd /usr/share/$(basename ${DIR})/lib/ > /dev/null
	JARS=$(ls *.jar)
	popd > /dev/null
	for jar in ${JARS}; do
		[[ ! -f "${DIR}/lib/${jar}" ]] && touch ${DIR}/lib/${jar}
	done

done

VDIR="../samples/etc/jem/virtuals.d"
[[ ! -d ${VDIR} ]] && mkdir -p ${VDIR}

VPATH="/etc/jem/virtuals.d/"
VIRTUALS=$(ls ${VPATH})

for virtual in ${VIRTUALS}; do
	[[ ! -f ${VPATH}/${virtual} ]] && cp -v ${VPATH}/${virtual} ${VDIR}
done
