#include <string>
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class JoyVelConverter : public rclcpp_lifecycle::LifecycleNode
{
public:
  // 長い型名の省略
  using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  // コンストラクタ
  JoyVelConverter()
  : rclcpp_lifecycle::LifecycleNode(std::string("joy_vel_converter"))
  {
    RCLCPP_INFO(this->get_logger(), "joy_vel_converter Constructor.");
  }

  //ディスコントラクタ
  ~JoyVelConverter()
  {
    RCLCPP_INFO(this->get_logger(), "joy_vel_converter Destructor.");
  }

  // configuring状態のときに呼ばれる関数
  CallbackReturn on_configure(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_configure() called. Previous state: %s",
      state.label().c_str());

    // Publisherとタイマを定義する
    vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
      std::string(
        "cmd_vel"), rclcpp::SystemDefaultsQoS());
    timer_ = this->create_wall_timer(1s, std::bind(&JoyVelConverter::timer_callback, this));
    return CallbackReturn::SUCCESS;
  }

  // activating状態のときに呼ばれる関数
  CallbackReturn on_activate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_activate() called. Previous state: %s",
      state.label().c_str());

    // Publisherをactivateする
    vel_pub_->on_activate();
    return CallbackReturn::SUCCESS;
  }

  // deactivating状態のときに呼ばれる関数
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_deactivate() called. Previous state: %s",
      state.label().c_str());

    // Publisherをdeactivateし、タイマを止める
    vel_pub_->on_deactivate();
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
    vel_pub_.reset();
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
    geometry_msgs::msg::Twist txdata;

    txdata.linear.x = 10;
    txdata.linear.y = 10;
    txdata.linear.z = 10;

    txdata.angular.x = 0;
    txdata.angular.y = 0;
    txdata.angular.z = 3;

    // lifecycle publisherがactivateのときのみ、データをpublishする
    if (vel_pub_->is_activated()) {
      RCLCPP_INFO(this->get_logger(), "publish");
      vel_pub_->publish(txdata);
    } else {
      RCLCPP_INFO(this->get_logger(), "Lifecycle publisher is NOT activated.");
    }
  }

private:
  // rclcpp::Publisherではなく、ライフサイクル用のpublisherを用いる
  rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::Twist>::SharedPtr vel_pub_;

  // 周期的な処理を行うためのタイマ
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JoyVelConverter>()->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}