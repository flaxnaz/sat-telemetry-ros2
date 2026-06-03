#include <chrono>
#include <memory>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "sat_telemetry/action/manoeuvre.hpp"

using namespace std::chrono_literals;
using Manoeuvre = sat_telemetry::action::Manoeuvre;
using GoalHandle = rclcpp_action::ServerGoalHandle<Manoeuvre>;

class ManoeuvreServer : public rclcpp::Node
{
public:
  ManoeuvreServer() : Node("manoeuvre_server")
  {
    action_server_ = rclcpp_action::create_server<Manoeuvre>(
      this, "execute_manoeuvre",
      // Goal callback — accept or reject
      [this](const rclcpp_action::GoalUUID& uuid,
             std::shared_ptr<const Manoeuvre::Goal> goal) {
        (void)uuid;
        RCLCPP_INFO(this->get_logger(),
          "Received manoeuvre request: %s — delta_v: %.2f m/s",
          goal->manoeuvre_type.c_str(), goal->delta_v_ms);
        if (goal->delta_v_ms <= 0.0f || goal->delta_v_ms > 500.0f) {
          RCLCPP_WARN(this->get_logger(), "Rejected — delta_v out of range");
          return rclcpp_action::GoalResponse::REJECT;
        }
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
      },
      // Cancel callback
      [this](const std::shared_ptr<GoalHandle> goal_handle) {
        (void)goal_handle;
        RCLCPP_WARN(this->get_logger(), "Manoeuvre cancel requested");
        return rclcpp_action::CancelResponse::ACCEPT;
      },
      // Accepted callback — runs the manoeuvre
      [this](const std::shared_ptr<GoalHandle> goal_handle) {
        std::thread([this, goal_handle]() {
          execute(goal_handle);
        }).detach();
      });

    RCLCPP_INFO(this->get_logger(), "Manoeuvre server ready on /execute_manoeuvre");
  }

private:
  rclcpp_action::Server<Manoeuvre>::SharedPtr action_server_;

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<Manoeuvre::Feedback>();
    auto result = std::make_shared<Manoeuvre::Result>();

    float total_dv = goal->delta_v_ms;
    float remaining = total_dv;
    float elapsed = 0.0f;
    float burn_rate = total_dv / 10.0f;  // complete in ~10 steps

    RCLCPP_INFO(this->get_logger(), "Executing %s manoeuvre — %.2f m/s",
      goal->manoeuvre_type.c_str(), total_dv);

    while (remaining > 0.0f && rclcpp::ok()) {
      // Check for cancellation
      if (goal_handle->is_canceling()) {
        result->success = false;
        result->actual_delta_v_ms = total_dv - remaining;
        result->message = "Manoeuvre cancelled at " + std::to_string(elapsed) + "s";
        goal_handle->canceled(result);
        RCLCPP_WARN(this->get_logger(), "Manoeuvre cancelled");
        return;
      }

      // Burn step
      float this_step = std::min(burn_rate, remaining);
      remaining -= this_step;
      elapsed += 1.0f;

      // Publish feedback
      feedback->delta_v_remaining_ms = remaining;
      feedback->thrust_pct = (remaining > 0.0f) ? 95.0f : 0.0f;
      feedback->elapsed_seconds = elapsed;
      feedback->phase = (remaining > total_dv * 0.5f) ? "burn_start" :
                        (remaining > 0.0f) ? "burn_mid" : "burn_complete";
      goal_handle->publish_feedback(feedback);

      RCLCPP_INFO(this->get_logger(),
        "[%.0fs] dv_remaining: %.2f m/s | thrust: %.1f%% | phase: %s",
        elapsed, remaining, feedback->thrust_pct, feedback->phase.c_str());

      std::this_thread::sleep_for(1000ms);
    }

    // Success
    result->success = true;
    result->actual_delta_v_ms = total_dv;
    result->message = "Manoeuvre complete — " + goal->manoeuvre_type +
                      " executed successfully in " + std::to_string((int)elapsed) + "s";
    goal_handle->succeed(result);
    RCLCPP_INFO(this->get_logger(), "Manoeuvre complete: %s", result->message.c_str());
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ManoeuvreServer>());
  rclcpp::shutdown();
  return 0;
}
