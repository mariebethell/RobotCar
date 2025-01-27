cmake_minimum_required(VERSION 3.8)
project(robotcar)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(SDL2_INCLUDE_DIRS /usr/include/SDL2)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

# Find and include pigpio library
find_library(PIGPIO_LIBRARY NAMES pigpiod_if2 PATHS /usr/local/include /usr/lib /usr/local/lib)
if(NOT PIGPIO_LIBRARY)
  message(FATAL_ERROR "pigpio library not found")
endif()

# Find and iclude SDL directory
find_library(SDL_LIBRARY NAMES SDL2 PATHS /usr/include/SDL2 /usr/include /usr/lib /usr/local/lib)
if(NOT SDL_LIBRARY)
  message(FATAL_ERROR "SDL library not found")
endif()


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

add_executable(motorDriverSubscriber src/motorDriverNode.cpp)
ament_target_dependencies(motorDriverSubscriber rclcpp geometry_msgs)
target_link_libraries(motorDriverSubscriber ${PIGPIO_LIBRARY})
target_link_libraries(motorDriverSubscriber "/usr/local/lib/libpigpiod_if2.so")

add_executable(controllerNode src/controllerNode.cpp)
ament_target_dependencies(controllerNode rclcpp geometry_msgs)
target_link_libraries(controllerNode ${SDL_LIBRARY})

add_executable(panTiltSubscriber src/panTiltNode.cpp)
ament_target_dependencies(panTiltSubscriber rclcpp geometry_msgs)

install(
  TARGETS motorDriverSubscriber
  DESTINATION lib/${PROJECT_NAME}
)
install(
  TARGETS controllerNode
  DESTINATION lib/${PROJECT_NAME}
)
install(
  TARGETS panTiltSubscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
