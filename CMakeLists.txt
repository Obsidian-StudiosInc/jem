#  This file is part of jem.
#
#  jem is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  jem is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with jem.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required (VERSION 2.6)
project (jem)

file(STRINGS "${PROJECT_SOURCE_DIR}/include/version.h"
	JEM_BUILD_SPEC REGEX "^[ \t]*#define[ \t]+JEM_VERSION_[A-Z]+[ \t]+[0-9]+.*$")

if(JEM_BUILD_SPEC)
	message(STATUS "${JEM_BUILD_SPEC}")
	foreach(item IN ITEMS MAJOR MINOR PATCH)
		string(REGEX REPLACE ".*#define[ \t]+JEM_VERSION_${item}[ \t]+([0-9]+).*"
			"\\1" XYZ ${JEM_BUILD_SPEC})
		set("JEM_VERSION_${item}" ${XYZ} CACHE STRING "Jem version")
	endforeach()
else()
	message(FATAL_ERROR "Version unavilable")
endif()

set(VERSION "${JEM_VERSION_MAJOR}.${JEM_VERSION_MINOR}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

set(CMAKE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY})

set(CMAKE_INSTALL_PREFIX "/usr")

include_directories(include)

add_library(jem SHARED src/output_formatter.c src/file_parser.c src/vm.c src/package.c src/env_manager.c)
add_executable(jem-cli src/main.c)
add_executable(jem-test EXCLUDE_FROM_ALL tests/test.c)
set_target_properties(jem-cli PROPERTIES OUTPUT_NAME jem)
target_link_libraries(jem-cli jem)
target_link_libraries(jem-test jem)
install(TARGETS jem jem-cli
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib)

# add a target to generate man page documentation with help2man
find_program(HELP2MAN "help2man")
if(HELP2MAN)
	add_custom_target(man ALL
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/docs/man/man1
			COMMAND ${HELP2MAN} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/jem -o
				${CMAKE_CURRENT_BINARY_DIR}/docs/man/man1/jem.1
			DEPENDS jem-cli)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/docs/man/man1/jem.1
		DESTINATION share/man/man1)
endif()

# add a target to generate API documentation with Doxygen
if(BUILD_DOC)
	find_package(Doxygen)
	if(DOXYGEN_FOUND)
#		configure_file(${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_SOURCE_DIR}/Doxyfile @ONLY)
		add_custom_target(doc ALL
			${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			DEPENDS man)
		install(FILES ${PROJECT_SOURCE_DIR}/docs/man/man3/
			DESTINATION share/man/man3)
		install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/html
			DESTINATION share/doc/jem-${VERSION}/)
	endif()
endif()

# Tests
#enable_testing()
add_custom_command(TARGET jem-test
	COMMAND /usr/bin/valgrind --leak-check=yes --leak-check=full
			--read-var-info=yes --show-reachable=yes
			--track-origins=yes ${CMAKE_BINARY_DIR}/dist/jem-test)
# Package Sources
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_IGNORE_FILES
"${PROJECT_SOURCE_DIR}/.git/"
"${PROJECT_SOURCE_DIR}/.dep.inc"
"${PROJECT_SOURCE_DIR}/.gitignore"
"${PROJECT_SOURCE_DIR}/CMakeCache.txt"
"${PROJECT_SOURCE_DIR}/CMakeFiles"
"${PROJECT_SOURCE_DIR}/_CPack_Packages/"
"${PROJECT_SOURCE_DIR}/CPackConfig.cmake"
"${PROJECT_SOURCE_DIR}/CPackSourceConfig.cmake"
"${PROJECT_SOURCE_DIR}/Makefile"
"${PROJECT_SOURCE_DIR}/cmake_install.cmake"
"${PROJECT_SOURCE_DIR}/nbproject/"
"${PROJECT_SOURCE_DIR}/dist/"
"${PROJECT_SOURCE_DIR}/docs/"
"${PROJECT_SOURCE_DIR}/install_manifest.txt"
)

# Create RPM
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "DEB;RPM;TBZ2")
set(CPACK_PACKAGE_NAME "jem")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "William L. Thomson Jr.")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "William L. Thomson Jr.")
set(CPACK_PACKAGE_VENDOR "Obsidian-Studios, Inc.")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_OUTPUT_FILE_PREFIX dist)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")
include(CPack)
