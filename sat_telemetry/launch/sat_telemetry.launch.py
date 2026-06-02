import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    params_file = os.path.join(
        get_package_share_directory('sat_telemetry'),
        'params', 'thresholds.yaml'
    )
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
            output='screen',
            parameters=[params_file]
        ),
    ])
