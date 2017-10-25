# CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

## set up hunter for deps

set(ENV{HUNTER_ROOT} ${CMAKE_CURRENT_SOURCE_DIR}/.hunter)
string(COMPARE EQUAL ${CMAKE_BUILD_TYPE} "Debug" _is_dbg)
if (_is_dbg)
  set(HUNTER_CONFIGURATION_TYPES Debug)
else()
  set(HUNTER_CONFIGURATION_TYPES Release)
endif()
set(HUNTER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(HUNTER_STATUS_DEBUG 1)
include(cmake/HunterGate.cmake)
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.19.133.tar.gz"
  SHA1 "510315cd43ebfb92c5459c54aa4f31f0e1c9212d")

## info

macro(msg m)
  message("  == [rbtree] ==  ${m}")
endmacro(msg)

## set up project

project(rbtree LANGUAGES C CXX VERSION 0.1.1)

## set up dependencies

include(cmake/deps.cmake)

# eg. set(deps libuv::uv)
set(deps )
build_deps("${deps}")

## set to export explicit exports only

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_STANDARD 11)

## set up RPATHs

if (UNIX)
  if(APPLE)
    set(CMAKE_INSTALL_NAME_DIR "@rpath/")
  else()
    set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
  endif()
endif(UNIX)

## set up sources

msg("Got project source dir: ${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE export_hdr ${PROJECT_SOURCE_DIR}/export/*.h ${PROJECT_SOURCE_DIR}/export/*.hpp)
file(GLOB_RECURSE src_files ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cpp)

msg("Got export hdrs: ${export_hdr}")
msg("Got src files: ${src_files}")

## set up library

add_library(rbtree ${src_files} ${export_hdr})

## setup build flags (public and private)

if (BUILD_SHARED_LIBS)
  set(build_flag BUILDING_RBTREE_SHARED)
  set(pub_build_flag )
  if (APPLE)
    foreach(_dep ${deps})
      get_target_property(_loc ${_dep} LOCATION)
      get_filename_component(_fn ${_loc} NAME)
      add_custom_command(
        TARGET rbtree
        POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -change ${_fn} @rpath/${_fn} $<TARGET_FILE:rbtree>)
    endforeach()
  endif()
else()
  set(build_flag )
  set(pub_build_flag USE_RBTREE_STATIC)
endif()

if (BUILD_SHARED_LIBS)
  set_target_properties(rbtree PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

target_compile_definitions(
  rbtree
  PUBLIC
    ${pub_build_flag}
  PRIVATE
    ${build_flag}
    RBTREE_VER_MAJ=${PROJECT_VERSION_MAJOR}
    RBTREE_VER_MIN=${PROJECT_VERSION_MINOR}
    RBTREE_VER_PATCH=${PROJECT_VERSION_PATCH}
    RBTREE_VER_STRING=${PROJECT_VERSION})

target_include_directories(
  rbtree
  PRIVATE include
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/export>
         $<INSTALL_INTERFACE:include>)

## link

target_link_libraries(rbtree PRIVATE ${deps})

# target_compile_features(
#   rbtree
#   PUBLIC
#   cxx_nullptr
#   cxx_lambdas
#   cxx_auto_type
#   cxx_static_assert)

## configure exports

include(CMakePackageConfigHelpers)

set(cmake_config_dir lib/cmake/rbtree)

configure_package_config_file(
  cmake/rbtreeConfig.cmake.in
  ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/rbtreeConfig.cmake
  INSTALL_DESTINATION ${cmake_config_dir})

set(version_config ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/rbtreeConfigVersion.cmake)
write_basic_package_version_file(
  ${version_config} VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion)

## install configs

install(
  TARGETS rbtree
  EXPORT rbtreeTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include)

install(
  DIRECTORY export/
  DESTINATION include)

install(
  FILES ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/rbtreeConfig.cmake ${version_config}
  DESTINATION ${cmake_config_dir}
  COMPONENT dev)

install(
  EXPORT rbtreeTargets
  NAMESPACE rbtree::
  DESTINATION ${cmake_config_dir}
  COMPONENT dev)

install(
  FILES ${PROJECT_SOURCE_DIR}/LICENSE
  DESTINATION share/rbtree
  RENAME LICENSE.rbtree)

install_deps("${deps}" INCLUDE_HEADERS) # or EXCLUDE_HEADERS

## tests

include(tests.cmake)

## packaging

# set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
# set(CPACK_PACKAGE_VENDOR "")
# set(CPACK_PACKAGE_CONTACT "")
# set(CPACK_STRIP_FILES 1)
# include(CPack)
