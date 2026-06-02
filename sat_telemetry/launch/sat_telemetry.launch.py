from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='sat_telemetry',
            executable='sat_publisher',
            name='sat_publisher',
            output='screen'
        ),
        Node(
            package='sat_telemetry',
            executable='sat_subscriber',
            name='sat_subscriber',
            output='screen'
        ),
        Node(
            package='sat_telemetry',
            executable='fault_detector',
            name='fault_detector',
            output='screen'
        ),
    ])
