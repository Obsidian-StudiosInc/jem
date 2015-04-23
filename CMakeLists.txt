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

set(VERSION "1.0.1")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

include_directories(lib)

add_library(jem SHARED src/output_formatter.c src/file_parser.c src/vm.c src/package.c src/env_manager.c)
add_executable(jem-cli src/main.c)
add_executable(jem-test EXCLUDE_FROM_ALL tests/test.c)
set_target_properties(jem-cli PROPERTIES OUTPUT_NAME jem)
target_link_libraries(jem-cli jem)
target_link_libraries(jem-test jem)
install(TARGETS jem jem-cli
	RUNTIME DESTINATION sbin
	LIBRARY DESTINATION lib)

# Tests
#enable_testing()
add_custom_command(TARGET jem-test
	COMMAND /usr/bin/valgrind --leak-check=yes --leak-check=full
			--read-var-info=yes --show-reachable=yes
			--track-origins=yes ${CMAKE_BINARY_DIR}/dist/jem-test)
# Package Sources
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_IGNORE_FILES
"${PROJECT_SOURCE_DIR}/CMakeCache.txt"
"${PROJECT_SOURCE_DIR}/CMakeFiles"
"${PROJECT_SOURCE_DIR}/_CPack_Packages/"
"${PROJECT_SOURCE_DIR}/CPackConfig.cmake"
"${PROJECT_SOURCE_DIR}/CPackSourceConfig.cmake"
"${PROJECT_SOURCE_DIR}/Makefile"
"${PROJECT_SOURCE_DIR}/cmake_install.cmake"
"${PROJECT_SOURCE_DIR}/dist/"
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
include(CPack)
