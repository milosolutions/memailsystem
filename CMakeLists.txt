
project(memailsystem
  VERSION 0.0.1
  DESCRIPTION "Milo Mail System"
  LANGUAGES CXX
)

find_package(Qt5 COMPONENTS REQUIRED Core Network)

set(SOURCES emailconfig.h memailsystem.h memailsystem.cpp)

set(OTHER_FILES README.md memailsystem.doxyfile)

add_library(memailsystem OBJECT ${SOURCES} ${OTHER_FILES})

# Defines
target_compile_definitions(memailsystem PUBLIC MEMAILSYSTEM_LIB)

target_include_directories(memailsystem
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(memailsystem
  Qt5::Core
  Qt5::Network
)

message("MEmailSystem: missing unit test!")
#add_subdirectory(tst_memailstystem)
