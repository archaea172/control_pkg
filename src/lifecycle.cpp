#include <string>
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class LifecycleTalker : public rclcpp_lifecycle::LifecycleNode
{
public:
  // 長い型名の省略
  using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  LifecycleTalker()
  : rclcpp_lifecycle::LifecycleNode(std::string("lifecycle_talker"))
  {
    RCLCPP_INFO(this->get_logger(), "LifecycleTalker Constructor.");
  }

  ~LifecycleTalker()
  {
    RCLCPP_INFO(this->get_logger(), "LifecycleTalker Destructor.");
  }

  // configuring状態のときに呼ばれる関数
  CallbackReturn on_configure(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_configure() called. Previous state: %s",
      state.label().c_str());

    // Publisherとタイマを定義する
    lifecycle_pub_ = this->create_publisher<std_msgs::msg::String>(
      std::string(
        "lc_chatter"), rclcpp::SystemDefaultsQoS());
    timer_ = this->create_wall_timer(1s, std::bind(&LifecycleTalker::timer_callback, this));
    return CallbackReturn::SUCCESS;
  }

  // activating状態のときに呼ばれる関数
  CallbackReturn on_activate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_activate() called. Previous state: %s",
      state.label().c_str());

    // Publisherをactivateする
    lifecycle_pub_->on_activate();
    return CallbackReturn::SUCCESS;
  }

  // deactivating状態のときに呼ばれる関数
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_deactivate() called. Previous state: %s",
      state.label().c_str());

    // Publisherをdeactivateし、タイマを止める
    lifecycle_pub_->on_deactivate();
    timer_->cancel();
    return CallbackReturn::SUCCESS;
  }

  // cleaning up状態のときに呼ばれる関数
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_cleanup() called. Previous state: %s",
      state.label().c_str());

    // 次にconfigureするときのために、shared_ptrで所有しているリソースを解放する
    lifecycle_pub_.reset();
    timer_.reset();
    return CallbackReturn::SUCCESS;
  }

  // error processing状態のときに呼ばれる関数
  CallbackReturn on_error(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(this->get_logger(), "on_error() called. Previous state: %s", state.label().c_str());
    return CallbackReturn::SUCCESS;
  }

  // shutting down状態のときに呼ばれる関数
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_shutdown() called. Previous state: %s",
      state.label().c_str());
    return CallbackReturn::SUCCESS;
  }

  // タイマーのコールバック関数
  void timer_callback()
  {
    std_msgs::msg::String msg;
    msg.data = "hello, world!";

    // lifecycle publisherがactivateのときのみ、データをpublishする
    if (lifecycle_pub_->is_activated()) {
      RCLCPP_INFO(this->get_logger(), "Lifecycle publisher is activated.");
      RCLCPP_INFO(this->get_logger(), "publish: %s", msg.data.c_str());
      lifecycle_pub_->publish(msg);
    } else {
      RCLCPP_INFO(this->get_logger(), "Lifecycle publisher is NOT activated.");
    }
  }

private:
  // rclcpp::Publisherではなく、ライフサイクル用のpublisherを用いる
  rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>::SharedPtr lifecycle_pub_;

  // 周期的な処理を行うためのタイマ
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LifecycleTalker>()->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}