# CMakeLists.txt for install

cmake_minimum_required(VERSION 3.0)

set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
set(CMAKE_CXX_STANDARD 11)

file(GLOB src_files ${TESTS_DIR}/*.c ${TESTS_DIR}/*.cpp)
set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/_source)
file(COPY ${src_files} DESTINATION ${src_dir})

find_package(rbtree)

file(GLOB src_files ${src_dir}/*.c ${src_dir}/*.cpp)

add_executable(tests_install ${src_files})

target_include_directories(
  tests_install
  PRIVATE ${TESTS_DIR})

target_link_libraries(tests_install rbtree::rbtree)

if (WIN32)
  set(path_run PATH="\$PATH$<SEMICOLON>$<TARGET_FILE_DIR:rbtree::rbtree>")
else()
  set(path_run )
endif()

add_custom_target(
  tests_run
  COMMAND ${CMAKE_COMMAND} -E env ${path_run} $<TARGET_FILE:tests_install>
  DEPENDS tests_install)
