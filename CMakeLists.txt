cmake_minimum_required (VERSION 3.1)
project (lingo)

set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig)
pkg_check_modules(yaml-cpp yaml-cpp REQUIRED)
pkg_check_modules(dpp dpp REQUIRED)
pkg_check_modules(GraphicsMagick GraphicsMagick++ REQUIRED)

add_subdirectory(vendor/curlcpp)
add_subdirectory(vendor/verbly)

include_directories(
  ${dpp_INCLUDE_DIRS}
  vendor/verbly/lib
  ${yaml-cpp_INCLUDE_DIRS}
  ${CURLCPP_SOURCE_DIR}/include
  vendor/json
  ${GraphicsMagick_INCLUDE_DIRS})

link_directories(
  ${dpp_LIBRARY_DIRS}
  ${yaml-cpp_LIBRARY_DIRS}
  ${GraphicsMagick_LIBRARY_DIRS})

add_executable(lingo lingo.cpp imagenet.cpp)
set_property(TARGET lingo PROPERTY CXX_STANDARD 17)
set_property(TARGET lingo PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(lingo verbly ${dpp_LIBRARIES} ${yaml-cpp_LIBRARIES} curlcpp curl ${GraphicsMagick_LIBRARIES})
