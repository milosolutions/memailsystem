find_package(Qt5 COMPONENTS REQUIRED Core)

set(CMAKE_AUTOMOC ON)

add_executable(example-helloworld main.cpp)

target_link_libraries(example-helloworld memailsystem
  Qt5::Core
)
