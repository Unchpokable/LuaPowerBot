# Install script for directory: C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/src/ost-1.86.0-75da820bf0.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/pkgs/boost-chrono_x64-windows")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/libs/chrono/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/src/ost-1.86.0-75da820bf0.clean/libs/chrono/include/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/stage/lib/boost_chrono-vc143-mt-x64-1_86.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/stage/bin/boost_chrono-vc143-mt-x64-1_86.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/stage/bin/boost_chrono-vc143-mt-x64-1_86.pdb")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0/boost_chrono-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0/boost_chrono-targets.cmake"
         "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/CMakeFiles/Export/dde5267b9c38b234140512de2dd70afc/boost_chrono-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0/boost_chrono-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0/boost_chrono-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/CMakeFiles/Export/dde5267b9c38b234140512de2dd70afc/boost_chrono-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/CMakeFiles/Export/dde5267b9c38b234140512de2dd70afc/boost_chrono-targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/tmpinst/boost_chrono-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_chrono-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/tmpinst/boost_chrono-config-version.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-chrono/x64-windows-rel/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
