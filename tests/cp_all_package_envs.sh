#!/bin/bash
#
# Copy all package.env files from a system running to testing dir
# run from tests dir in jem sources

PKG_ENVS=$(ls /usr/share/*/package.env)

for pkg_env in $PKG_ENVS; do

	DIR="../samples$(dirname ${pkg_env})"
	[[ ! -d ${DIR} ]] && mkdir -p ${DIR}
	[[ ! -f "../samples${pkg_env}" ]] && cp -v ${pkg_env} ${DIR}

done
