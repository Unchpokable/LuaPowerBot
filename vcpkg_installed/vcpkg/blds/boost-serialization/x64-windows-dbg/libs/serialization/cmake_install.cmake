# Install script for directory: C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/src/ost-1.86.0-0fd03110d6.clean/libs/serialization

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/pkgs/boost-serialization_x64-windows/debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/src/ost-1.86.0-0fd03110d6.clean/libs/serialization/include/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/stage/lib/boost_serialization-vc143-mt-gd-x64-1_86.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/stage/bin/boost_serialization-vc143-mt-gd-x64-1_86.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/stage/bin/boost_serialization-vc143-mt-gd-x64-1_86.pdb")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0/boost_serialization-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0/boost_serialization-targets.cmake"
         "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/CMakeFiles/Export/44e258b2cbae920ebd8456fa7c47bb3e/boost_serialization-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0/boost_serialization-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0/boost_serialization-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/CMakeFiles/Export/44e258b2cbae920ebd8456fa7c47bb3e/boost_serialization-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/CMakeFiles/Export/44e258b2cbae920ebd8456fa7c47bb3e/boost_serialization-targets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/tmpinst/boost_serialization-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_serialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/tmpinst/boost_serialization-config-version.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/stage/lib/boost_wserialization-vc143-mt-gd-x64-1_86.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/stage/bin/boost_wserialization-vc143-mt-gd-x64-1_86.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/stage/bin/boost_wserialization-vc143-mt-gd-x64-1_86.pdb")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0/boost_wserialization-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0/boost_wserialization-targets.cmake"
         "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/CMakeFiles/Export/f05e39b3467168c88da21e87a9b8868a/boost_wserialization-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0/boost_wserialization-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0/boost_wserialization-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/CMakeFiles/Export/f05e39b3467168c88da21e87a9b8868a/boost_wserialization-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/CMakeFiles/Export/f05e39b3467168c88da21e87a9b8868a/boost_wserialization-targets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/tmpinst/boost_wserialization-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/boost_wserialization-1.86.0" TYPE FILE FILES "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/tmpinst/boost_wserialization-config-version.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/DEACMM-PC/source/repos/LuaPowerBot/vcpkg_installed/vcpkg/blds/boost-serialization/x64-windows-dbg/libs/serialization/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
