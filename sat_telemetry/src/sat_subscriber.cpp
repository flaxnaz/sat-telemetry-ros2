#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sat_telemetry/msg/sat_state.hpp"

class SatTelemetrySubscriber : public rclcpp::Node
{
public:
  SatTelemetrySubscriber() : Node("sat_subscriber"), msg_count_(0)
  {
    subscription_ = this->create_subscription<sat_telemetry::msg::SatState>(
      "sat_telemetry", 10,
      [this](const sat_telemetry::msg::SatState::SharedPtr msg) {
        msg_count_++;
        RCLCPP_INFO(this->get_logger(),
          "[%zu] SEQ:%u ALT:%.2fkm BAT:%.1f%% TEMP:%.1fC PHASE:%s",
          msg_count_, msg->sequence, msg->altitude_km,
          msg->battery_pct, msg->temperature_c, msg->mission_phase.c_str());
      });
    RCLCPP_INFO(this->get_logger(), "Listening on /sat_telemetry...");
  }
private:
  rclcpp::Subscription<sat_telemetry::msg::SatState>::SharedPtr subscription_;
  size_t msg_count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SatTelemetrySubscriber>());
  rclcpp::shutdown();
  return 0;
}
