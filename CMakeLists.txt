cmake_minimum_required(VERSION 3.3)
project(GeospaceServer)
set(PROJECT_VERSION 0.1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_CXX_COMPILER g++-7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

# Export a JSON file with information about our build for irony-mode
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(PROJECT_SOURCE_DIR, "src/")
set(PROJECT_INCLUDE_DIR, "include/")

set(HEADERS
  include/utility.hpp
  include/handlers.hpp
)

set(SOURCES
  src/handlers.cpp
)


# include(${CMAKE_BINARY_DIR}/../conanbuildinfo.cmake)
# conan_basic_setup(TARGETS)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
ExternalProject_Add (
  PocoHttpRouterProject
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/include/PocoHttpRouter
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  GIT_REPOSITORY "git@github.com:kennymalac/PocoHttpRouter.git"
  GIT_TAG "master"
  UPDATE_COMMAND ""
  INSTALL_COMMAND "")

ExternalProject_Get_Property(PocoHttpRouterProject install_dir)

include_directories(include ${CMAKE_CURRENT_BINARY_DIR}/include/PocoHttpRouter/src)

add_library(GeospaceServer_Base ${HEADERS} ${SOURCES})
target_link_libraries(GeospaceServer_Base PocoRedis)
add_executable(GeospaceServer src/main.cpp)

add_dependencies(GeospaceServer PocoHttpRouterProject)
target_link_libraries(GeospaceServer PocoFoundation
    PocoNetSSL
    PocoUtil
    PocoJSON
    PocoCrypto
    PocoNet
    PocoRedis "${CMAKE_CURRENT_BINARY_DIR}/include/PocoHttpRouter/src/PocoHttpRouterProject-build/libPocoHttpRouter.a" pthread GeospaceServer_Base)
# target_link_libraries(GeospaceServer PocoRedis CONAN_PKG::Poco pthread)
