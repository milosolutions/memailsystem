find_package(Qt5 COMPONENTS REQUIRED Core)

set(CMAKE_AUTOMOC ON)

add_executable(example-sendmail main.cpp)

target_link_libraries(example-sendmail memailsystem
  Qt5::Core
)
