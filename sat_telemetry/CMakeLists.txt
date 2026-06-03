cmake_minimum_required(VERSION 3.8)
project(sat_telemetry)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SatState.msg"
  "msg/FaultAlert.msg"
  "srv/FaultStatus.srv"
)

ament_export_dependencies(rosidl_default_runtime)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")

add_executable(sat_publisher src/sat_publisher.cpp)
ament_target_dependencies(sat_publisher rclcpp std_msgs)
target_link_libraries(sat_publisher ${cpp_typesupport_target})

add_executable(sat_subscriber src/sat_subscriber.cpp)
ament_target_dependencies(sat_subscriber rclcpp std_msgs)
target_link_libraries(sat_subscriber ${cpp_typesupport_target})

add_executable(fault_detector src/fault_detector.cpp)
ament_target_dependencies(fault_detector rclcpp std_msgs)
target_link_libraries(fault_detector ${cpp_typesupport_target})

install(TARGETS sat_publisher sat_subscriber fault_detector
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch params
  DESTINATION share/${PROJECT_NAME})

ament_package()
