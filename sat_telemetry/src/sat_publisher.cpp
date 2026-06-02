#include <chrono>
#include <memory>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class SatTelemetryPublisher : public rclcpp::Node
{
public:
  SatTelemetryPublisher() : Node("sat_telemetry_publisher"), count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>("sat_telemetry", 10);
    timer_ = this->create_wall_timer(
      1000ms, [this]() {
        auto msg = std_msgs::msg::String();
        msg.data = "SEQ:" + std::to_string(count_++) +
                   " ALT:408.2km BAT:94.3% TEMP:22.1C";
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
        publisher_->publish(msg);
      });
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SatTelemetryPublisher>());
  rclcpp::shutdown();
  return 0;
}
