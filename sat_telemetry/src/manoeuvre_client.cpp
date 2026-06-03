#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "sat_telemetry/action/manoeuvre.hpp"

using Manoeuvre = sat_telemetry::action::Manoeuvre;

class ManoeuvreClient : public rclcpp::Node
{
public:
  ManoeuvreClient() : Node("manoeuvre_client")
  {
    client_ = rclcpp_action::create_client<Manoeuvre>(this, "execute_manoeuvre");

    // Wait for server
    if (!client_->wait_for_action_server(std::chrono::seconds(5))) {
      RCLCPP_ERROR(this->get_logger(), "Manoeuvre server not available");
      return;
    }

    // Send goal
    auto goal = Manoeuvre::Goal();
    goal.delta_v_ms = 25.0f;
    goal.manoeuvre_type = "apogee_raise";

    RCLCPP_INFO(this->get_logger(),
      "Sending goal: %s — %.2f m/s", goal.manoeuvre_type.c_str(), goal.delta_v_ms);

    auto send_goal_options = rclcpp_action::Client<Manoeuvre>::SendGoalOptions();

    // Feedback callback
    send_goal_options.feedback_callback =
      [this](rclcpp_action::ClientGoalHandle<Manoeuvre>::SharedPtr,
             const std::shared_ptr<const Manoeuvre::Feedback> feedback) {
        RCLCPP_INFO(this->get_logger(),
          "[FEEDBACK] dv_remaining: %.2f m/s | thrust: %.1f%% | phase: %s | elapsed: %.0fs",
          feedback->delta_v_remaining_ms, feedback->thrust_pct,
          feedback->phase.c_str(), feedback->elapsed_seconds);
      };

    // Result callback
    send_goal_options.result_callback =
      [this](const rclcpp_action::ClientGoalHandle<Manoeuvre>::WrappedResult& result) {
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_INFO(this->get_logger(), "[RESULT] SUCCESS: %s | actual_dv: %.2f m/s",
            result.result->message.c_str(), result.result->actual_delta_v_ms);
        } else {
          RCLCPP_WARN(this->get_logger(), "[RESULT] FAILED/CANCELLED: %s",
            result.result->message.c_str());
        }
        rclcpp::shutdown();
      };

    client_->async_send_goal(goal, send_goal_options);
  }

private:
  rclcpp_action::Client<Manoeuvre>::SharedPtr client_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ManoeuvreClient>());
  rclcpp::shutdown();
  return 0;
}
