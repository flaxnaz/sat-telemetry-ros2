# sat-telemetry-ros2

ROS2 Humble (C++) publisher/subscriber nodes for simulated satellite telemetry.

## Overview
A ROS2 package demonstrating real-time telemetry streaming between spacecraft subsystem nodes over a named topic. Built as part of a C++/ROS2 skill development sprint targeting flight software roles.

## Nodes
- **sat_publisher** — publishes simulated satellite state (sequence, altitude, battery, temperature) at 1Hz
- **sat_subscriber** — subscribes to `/sat_telemetry` and logs each message with a counter

## Build & Run
```bash
source /opt/ros/humble/setup.bash
colcon build --packages-select sat_telemetry
source install/setup.bash

# Terminal 1
ros2 run sat_telemetry sat_publisher

# Terminal 2
ros2 run sat_telemetry sat_subscriber
```

## Environment
- ROS2 Humble | Ubuntu 22.04 | C++17
- Related: [sat-telemetry-logger](https://github.com/flaxnaz/sat-telemetry-logger) (Python pipeline)
