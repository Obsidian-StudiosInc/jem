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

set(JAVA_BIN_TOOLS
	"appletviewer"
	"apt"
	"ControlPanel"
	"extcheck"
	"HtmlConverter"
	"idlj"
	"jar"
	"jarsigner"
	"java"
	"javac"
	"javadoc"
	"javah"
	"javap"
	"javaws"
	"jconsole"
	"jcontrol"
	"jdb"
	"jhat"
	"jinfo"
	"jmap"
	"jps"
	"jrunscript"
	"jsadebugd"
	"jstack"
	"jstat"
	"jstatd"
	"jvisualvm"
	"keytool"
	"native2ascii"
	"orbd"
	"pack200"
	"policytool"
	"rmic"
	"rmid"
	"rmiregistry"
	"schemagen"
	"serialver"
	"servertool"
	"tnameserv"
	"wsgen"
	"wsimport"
	"xjc"
)

foreach(T ${JAVA_BIN_TOOLS})
	set(MY_PREFFIX $ENV{DESTDIR})
	if(NOT MY_PREFFIX)
		set(MY_PREFFIX ${CMAKE_INSTALL_PREFIX})
	endif()
	execute_process(COMMAND ln -sfv run-java-tool.bash ${T}
			WORKING_DIRECTORY ${MY_PREFFIX}/usr/bin)
endforeach()
