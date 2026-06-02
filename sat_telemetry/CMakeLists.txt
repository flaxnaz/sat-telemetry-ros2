cmake_minimum_required(VERSION 3.8)
project(sat_telemetry)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(sat_publisher src/sat_publisher.cpp)
ament_target_dependencies(sat_publisher rclcpp std_msgs)

add_executable(sat_subscriber src/sat_subscriber.cpp)
ament_target_dependencies(sat_subscriber rclcpp std_msgs)

install(TARGETS sat_publisher sat_subscriber DESTINATION lib/${PROJECT_NAME})

ament_package()
