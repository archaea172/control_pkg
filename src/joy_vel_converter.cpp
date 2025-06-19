#include <string>
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"

#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/joy.hpp"

using std::placeholders::_1;

using namespace std::chrono_literals;

class JoyVelConverter : public rclcpp_lifecycle::LifecycleNode
{
public:
  // 長い型名の省略
  using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  float max_vx = 2;
  float max_vy = 2;
  float max_omega = 2;

  float max_button_vx = 0.2;
  float max_button_vy = 0.2;

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

    // vel Publisher
    vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
      std::string("cmd_vel"), rclcpp::SystemDefaultsQoS()
    );
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
    // joy subscriber
    joy_subscriber_ = this->create_subscription<sensor_msgs::msg::Joy>(
      std::string("joy"),
      rclcpp::SystemDefaultsQoS(),
      std::bind(&JoyVelConverter::joy_callback, this, _1)
    );
    return CallbackReturn::SUCCESS;
  }

  // deactivating状態のときに呼ばれる関数
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(
      this->get_logger(), "on_deactivate() called. Previous state: %s",
      state.label().c_str());

    // Publisherをdeactivate
    vel_pub_->on_deactivate();
    // subscriberをreset
    joy_subscriber_.reset();
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
    joy_subscriber_.reset();
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

  // コールバック関数
  void joy_callback(const sensor_msgs::msg::Joy::SharedPtr rxdata) const
  {
    // lifecycle publisherがactivateのときのみ、データをpublishする
    if (vel_pub_->is_activated()) {

      geometry_msgs::msg::Twist txdata;
      txdata.linear.x = -rxdata->axes[0] * max_vx - rxdata->axes[6]*max_button_vx;
      txdata.linear.y =  rxdata->axes[1] * max_vy + rxdata->axes[7]*max_button_vy;
      txdata.linear.z = 0;

      txdata.angular.x = 0;
      txdata.angular.y = 0;
      txdata.angular.z = (rxdata->axes[5] - rxdata->axes[2])*max_omega;
      vel_pub_->publish(txdata);
    } else {
      RCLCPP_INFO(this->get_logger(), "controler is NOT activated.");
    }
  }


private:
  // rclcpp::Publisherではなく、ライフサイクル用のpublisherを用いる
  rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::Twist>::SharedPtr vel_pub_;

  // joy subscriber
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_subscriber_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JoyVelConverter>()->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}