#include <memory>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class SatTelemetrySubscriber : public rclcpp::Node
{
public:
  SatTelemetrySubscriber() : Node("sat_telemetry_subscriber"), msg_count_(0)
  {
    subscription_ = this->create_subscription<std_msgs::msg::String>(
      "sat_telemetry", 10,
      [this](const std_msgs::msg::String::SharedPtr msg) {
        msg_count_++;
        RCLCPP_INFO(this->get_logger(), "[%zu] Received: '%s'",
                    msg_count_, msg->data.c_str());
      });
    RCLCPP_INFO(this->get_logger(), "Listening on /sat_telemetry...");
  }

private:
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  size_t msg_count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SatTelemetrySubscriber>());
  rclcpp::shutdown();
  return 0;
}
