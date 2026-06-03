#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sat_telemetry/msg/sat_state.hpp"
#include "sat_telemetry/msg/fault_alert.hpp"
#include "sat_telemetry/srv/fault_status.hpp"

class FaultDetector : public rclcpp::Node
{
public:
  FaultDetector() : Node("fault_detector"),
    total_faults_(0), battery_faults_(0),
    temperature_faults_(0), altitude_faults_(0),
    last_battery_(0.0f)
  {
    this->declare_parameter("battery_warning", 90.0);
    this->declare_parameter("battery_critical", 85.0);
    this->declare_parameter("temp_max", 30.0);
    this->declare_parameter("temp_min", 10.0);
    this->declare_parameter("altitude_min", 400.0);

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
        last_battery_ = msg->battery_pct;
        check_battery(msg);
        check_temperature(msg);
        check_altitude(msg);
      });

    // Service server — responds to fault status queries
    service_ = this->create_service<sat_telemetry::srv::FaultStatus>(
      "get_fault_status",
      [this](const sat_telemetry::srv::FaultStatus::Request::SharedPtr /*req*/,
             sat_telemetry::srv::FaultStatus::Response::SharedPtr res) {
        res->total_faults       = total_faults_;
        res->battery_faults     = battery_faults_;
        res->temperature_faults = temperature_faults_;
        res->altitude_faults    = altitude_faults_;
        res->last_battery_value = last_battery_;
        res->status = total_faults_ == 0 ? "NOMINAL" :
                     (battery_faults_ > 0 ? "BATTERY_ISSUE" : "FAULT_ACTIVE");
        RCLCPP_INFO(this->get_logger(),
          "Status queried — total:%u bat:%u temp:%u alt:%u",
          total_faults_, battery_faults_, temperature_faults_, altitude_faults_);
      });

    RCLCPP_INFO(this->get_logger(), "Fault detector online. Service: /get_fault_status");
  }

private:
  rclcpp::Subscription<sat_telemetry::msg::SatState>::SharedPtr subscription_;
  rclcpp::Publisher<sat_telemetry::msg::FaultAlert>::SharedPtr publisher_;
  rclcpp::Service<sat_telemetry::srv::FaultStatus>::SharedPtr service_;

  double bat_warn_, bat_crit_, temp_max_, temp_min_, alt_min_;
  uint32_t total_faults_, battery_faults_, temperature_faults_, altitude_faults_;
  float last_battery_;

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
    total_faults_++;
    RCLCPP_WARN(this->get_logger(),
      "[FAULT][SEQ:%u] %s — value:%.2f threshold:%.2f severity:%u",
      seq, type.c_str(), value, threshold, severity);
  }

  void check_battery(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->battery_pct < bat_crit_) {
      raise_fault(msg->sequence, "LOW_BATTERY",
        "Battery below safe threshold", msg->battery_pct, bat_crit_, 2);
      battery_faults_++;
    } else if (msg->battery_pct < bat_warn_) {
      raise_fault(msg->sequence, "BATTERY_WARNING",
        "Battery approaching low threshold", msg->battery_pct, bat_warn_, 1);
      battery_faults_++;
    }
  }

  void check_temperature(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->temperature_c > temp_max_) {
      raise_fault(msg->sequence, "OVERTEMP",
        "Temperature exceeds safe limit", msg->temperature_c, temp_max_, 3);
      temperature_faults_++;
    } else if (msg->temperature_c < temp_min_) {
      raise_fault(msg->sequence, "UNDERTEMP",
        "Temperature below safe limit", msg->temperature_c, temp_min_, 2);
      temperature_faults_++;
    }
  }

  void check_altitude(const sat_telemetry::msg::SatState::SharedPtr msg) {
    if (msg->altitude_km < alt_min_) {
      raise_fault(msg->sequence, "LOW_ALTITUDE",
        "Altitude below safe orbit threshold", msg->altitude_km, alt_min_, 3);
      altitude_faults_++;
    }
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FaultDetector>());
  rclcpp::shutdown();
  return 0;
}
