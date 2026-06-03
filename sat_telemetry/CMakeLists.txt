cmake_minimum_required(VERSION 3.8)
project(sat_telemetry)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SatState.msg"
  "msg/FaultAlert.msg"
  "srv/FaultStatus.srv"
  "action/Manoeuvre.action"
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

add_executable(manoeuvre_server src/manoeuvre_server.cpp)
ament_target_dependencies(manoeuvre_server rclcpp rclcpp_action std_msgs)
target_link_libraries(manoeuvre_server ${cpp_typesupport_target})

add_executable(manoeuvre_client src/manoeuvre_client.cpp)
ament_target_dependencies(manoeuvre_client rclcpp rclcpp_action std_msgs)
target_link_libraries(manoeuvre_client ${cpp_typesupport_target})

install(TARGETS sat_publisher sat_subscriber fault_detector manoeuvre_server manoeuvre_client
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch params
  DESTINATION share/${PROJECT_NAME})

ament_package()
