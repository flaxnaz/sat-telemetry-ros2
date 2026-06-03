# sat-telemetry-ros2

A multi-node ROS2 Humble (C++17) satellite telemetry pipeline with custom message types, real-time fault detection, and a single-command launch file.

Built as part of a C++/ROS2 skill sprint targeting flight software roles in the Australian space sector.

## Live Dashboard

https://flaxnaz.github.io/sat-telemetry-ros2

Interactive simulation of the telemetry pipeline — metrics, charts, subscriber log, and fault alerts updating at 1 Hz.

## Architecture

    sat_publisher -> /sat_telemetry (SatState) -> sat_subscriber
                                               -> fault_detector -> /sat_faults (FaultAlert)

## Nodes

| Node | Topic | Role |
|---|---|---|
| sat_publisher | /sat_telemetry | Publishes SatState at 1 Hz with sinusoidal altitude and battery drain |
| sat_subscriber | /sat_telemetry | Subscribes and logs every typed packet with counter |
| fault_detector | /sat_faults | Monitors thresholds, raises severity-graded FaultAlert messages |

## Custom Messages

SatState.msg: sequence, altitude_km, battery_pct, temperature_c, mission_phase

FaultAlert.msg: sequence, fault_type, description, value, threshold, severity

## Fault Thresholds

| Parameter | Warning | Critical |
|---|---|---|
| Battery | < 90% severity 1 | < 85% severity 2 |
| Temperature | < 10C severity 2 | > 30C severity 3 |
| Altitude | < 400km severity 3 | - |

## Build and Run

    source /opt/ros/humble/setup.bash
    cd ~/ros2_ws
    colcon build --packages-select sat_telemetry
    source install/setup.bash
    ros2 launch sat_telemetry sat_telemetry.launch.py

## Environment

- ROS2 Humble | Ubuntu 22.04 WSL2 | C++17 | CMake
- Related: sat-telemetry-logger https://github.com/flaxnaz/sat-telemetry-logger
- Related: nrho-visibility https://github.com/flaxnaz/nrho-visibility
