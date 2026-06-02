#include <chrono>
#include <memory>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "sat_telemetry/msg/sat_state.hpp"

using namespace std::chrono_literals;

class SatTelemetryPublisher : public rclcpp::Node
{
public:
  SatTelemetryPublisher() : Node("sat_publisher"), seq_(0)
  {
    publisher_ = this->create_publisher<sat_telemetry::msg::SatState>("sat_telemetry", 10);
    timer_ = this->create_wall_timer(1000ms, [this]() {
      auto msg = sat_telemetry::msg::SatState();
      msg.sequence      = seq_++;
      msg.altitude_km   = 408.2 + 0.5 * std::sin(seq_ * 0.1);
      msg.battery_pct   = 94.3f - seq_ * 0.2f;
      msg.temperature_c = 22.1f + 0.3f * std::sin(seq_ * 0.2f);
      msg.mission_phase = seq_ < 30 ? "nominal" : (seq_ < 60 ? "eclipse" : "nominal");
      RCLCPP_INFO(this->get_logger(), "[SEQ:%u] ALT:%.2fkm BAT:%.1f%% TEMP:%.1fC PHASE:%s",
        msg.sequence, msg.altitude_km, msg.battery_pct, msg.temperature_c, msg.mission_phase.c_str());
      publisher_->publish(msg);
    });
  }
private:
  rclcpp::Publisher<sat_telemetry::msg::SatState>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  uint32_t seq_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SatTelemetryPublisher>());
  rclcpp::shutdown();
  return 0;
}
