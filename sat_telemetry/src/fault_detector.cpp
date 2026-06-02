#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sat_telemetry/msg/sat_state.hpp"
#include "sat_telemetry/msg/fault_alert.hpp"

class FaultDetector : public rclcpp::Node
{
public:
  FaultDetector() : Node("fault_detector")
  {
    // Declare parameters with defaults
    this->declare_parameter("battery_warning", 90.0);
    this->declare_parameter("battery_critical", 85.0);
    this->declare_parameter("temp_max", 30.0);
    this->declare_parameter("temp_min", 10.0);
    this->declare_parameter("altitude_min", 400.0);

    // Read parameters
    bat_warn_  = this->get_parameter("battery_warning").as_double();
    bat_crit_  = this->get_parameter("battery_critical").as_double();
    temp_max_  = this->get_parameter("temp_max").as_double();
    temp_min_  = this->get_parameter("temp_min").as_double();
    alt_min_   = this->get_parameter("altitude_min").as_double();

    RCLCPP_INFO(this->get_logger(),
      "Thresholds — BAT warn:%.1f crit:%.1f TEMP min:%.1f max:%.1f ALT min:%.1f",
      bat_warn_, bat_crit_, temp_min_, temp_max_, alt_min_);

    publisher_ = this->create_publisher<sat_telemetry::msg::FaultAlert>("sat_faults", 10);

    subscription_ = this->create_subscription<sat_telemetry::msg::SatState>(
      "sat_telemetry", 10,
      [this](const sat_telemetry::msg::SatState::SharedPtr msg) {
        check_battery(msg);
        check_temperature(msg);
        check_altitude(msg);
      });

    RCLCPP_INFO(this->get_logger(), "Fault detector online. Monitoring /sat_telemetry...");
  }

private:
  rclcpp::Subscription<sat_telemetry::msg::SatState>::SharedPtr subscription_;
  rclcpp::Publisher<sat_telemetry::msg::FaultAlert>::SharedPtr publisher_;
  double bat_warn_, bat_crit_, temp_max_, temp_min_, alt_min_;

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
    RCLCPP_WARN(this->get_logger(),
      "[FAULT][SEQ:%u] %s — value:%.2f threshold:%.2f severity:%u",
      seq, type.c_str(), value, threshold, severity);
  }

  void check_battery(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->battery_pct < bat_crit_)
      raise_fault(msg->sequence, "LOW_BATTERY",
        "Battery below safe threshold", msg->battery_pct, bat_crit_, 2);
    else if (msg->battery_pct < bat_warn_)
      raise_fault(msg->sequence, "BATTERY_WARNING",
        "Battery approaching low threshold", msg->battery_pct, bat_warn_, 1);
  }

  void check_temperature(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->temperature_c > temp_max_)
      raise_fault(msg->sequence, "OVERTEMP",
        "Temperature exceeds safe limit", msg->temperature_c, temp_max_, 3);
    else if (msg->temperature_c < temp_min_)
      raise_fault(msg->sequence, "UNDERTEMP",
        "Temperature below safe limit", msg->temperature_c, temp_min_, 2);
  }

  void check_altitude(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->altitude_km < alt_min_)
      raise_fault(msg->sequence, "LOW_ALTITUDE",
        "Altitude below safe orbit threshold", msg->altitude_km, alt_min_, 3);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FaultDetector>());
  rclcpp::shutdown();
  return 0;
}
