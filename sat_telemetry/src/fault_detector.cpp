#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sat_telemetry/msg/sat_state.hpp"
#include "sat_telemetry/msg/fault_alert.hpp"

class FaultDetector : public rclcpp::Node
{
public:
  FaultDetector() : Node("fault_detector")
  {
    subscription_ = this->create_subscription<sat_telemetry::msg::SatState>(
      "sat_telemetry", 10,
      [this](const sat_telemetry::msg::SatState::SharedPtr msg) {
        check_battery(msg);
        check_temperature(msg);
        check_altitude(msg);
      });

    publisher_ = this->create_publisher<sat_telemetry::msg::FaultAlert>("sat_faults", 10);
    RCLCPP_INFO(this->get_logger(), "Fault detector online. Monitoring /sat_telemetry...");
  }

private:
  rclcpp::Subscription<sat_telemetry::msg::SatState>::SharedPtr subscription_;
  rclcpp::Publisher<sat_telemetry::msg::FaultAlert>::SharedPtr publisher_;

  void raise_fault(uint32_t seq, const std::string& type,
                   const std::string& desc, float value, float threshold, uint8_t severity)
  {
    auto alert = sat_telemetry::msg::FaultAlert();
    alert.sequence    = seq;
    alert.fault_type  = type;
    alert.description = desc;
    alert.value       = value;
    alert.threshold   = threshold;
    alert.severity    = severity;
    publisher_->publish(alert);
    RCLCPP_WARN(this->get_logger(), "[FAULT][SEQ:%u] %s — value:%.2f threshold:%.2f severity:%u",
      seq, type.c_str(), value, threshold, severity);
  }

  void check_battery(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->battery_pct < 85.0f)
      raise_fault(msg->sequence, "LOW_BATTERY",
        "Battery below safe threshold", msg->battery_pct, 85.0f, 2);
    else if (msg->battery_pct < 90.0f)
      raise_fault(msg->sequence, "BATTERY_WARNING",
        "Battery approaching low threshold", msg->battery_pct, 90.0f, 1);
  }

  void check_temperature(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->temperature_c > 30.0f)
      raise_fault(msg->sequence, "OVERTEMP",
        "Temperature exceeds safe limit", msg->temperature_c, 30.0f, 3);
    else if (msg->temperature_c < 10.0f)
      raise_fault(msg->sequence, "UNDERTEMP",
        "Temperature below safe limit", msg->temperature_c, 10.0f, 2);
  }

  void check_altitude(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->altitude_km < 400.0)
      raise_fault(msg->sequence, "LOW_ALTITUDE",
        "Altitude below safe orbit threshold", msg->altitude_km, 400.0f, 3);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FaultDetector>());
  rclcpp::shutdown();
  return 0;
}
