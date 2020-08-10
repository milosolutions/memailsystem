if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif ()

cmake_minimum_required(VERSION 3.5)

project(memailsystem
  VERSION 0.0.1
  DESCRIPTION "Milo Mail System"
  LANGUAGES CXX
)

find_package(Qt5 COMPONENTS REQUIRED Core Network)

set(CMAKE_AUTOMOC ON)
set(mconfig-editors ON)
set(mconfig-crypto ON)

add_subdirectory(../mcrypto mcrypto)
add_subdirectory(../mconfig mconfig)

set(SOURCES emailconfig.h memailsystem.h memailsystem.cpp)

set(OTHER_FILES README.md memailsystem.doxyfile)

add_library(memailsystem STATIC ${SOURCES} ${OTHER_FILES})

# Defines
target_compile_definitions(memailsystem PUBLIC MEMAILSYSTEM_LIB)
target_compile_definitions(memailsystem PUBLIC MCRYPTO_LIB)

target_include_directories(memailsystem
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(memailsystem mcrypto mconfig
  Qt5::Core
  Qt5::Network
)

message("MEmailSystem: missing unit test!")
#add_subdirectory(tst_memailstystem)
add_subdirectory(example/helloworld)
add_subdirectory(example/sendmail)
