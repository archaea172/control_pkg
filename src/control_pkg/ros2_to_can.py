import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool
from geometry_msgs.msg import Twist

import can
from control_pkg.byte_control import ValueBridge

class Ros2Can(Node):
    def __init__(self):
        self.bridge = ValueBridge()
        super().__init__('ros2_to_can')
        self.can0 = can.interface.Bus(
            channel='can0',
            bustype='socketcan',
            bitrate=1000000,
            fd=True,
            data_bitrate=2000000
        )
        self.can_timer = self.create_timer(
            0.1,
            self.callback_timer
        )
        self.vel_subscription = self.create_subscription(
            Twist,
            'cmd_vel',
            self.vel_callback,
            10
        )
        self.vel_txdata = [0, 0, 0]
        self.vel_subscription

    def callback_timer(self):
        self.can_send(self.vel_txdata, 0x300)
        self.get_logger().info(str(self.vel_txdata))

    def can_send(self, txdata_list:list, msg_id:int):
        txdata_byte_list = self.bridge.nhk2025_f32_to_byte(txdata_list)
        txdata_can = can.Message(
            arbitration_id=msg_id,
            is_extended_id=False,
            dlc=12,
            data=txdata_byte_list,
            is_fd=True,
        )
        try:
            self.can_state = True
            self.can0.send(txdata_can)
        except can.CanError:
            self.can_state = False

    def vel_callback(self, rxdata:Twist):
        self.vel_txdata = [rxdata.linear.x, rxdata.linear.y, rxdata.angular.z]

    def destroy_node(self):
        self.can0.shutdown()
        super().destroy_node()
        

def main_ros2_to_can():
    rclpy.init()
    ros2_to_can = Ros2Can()
    try:
        rclpy.spin(ros2_to_can)
    except KeyboardInterrupt:
        pass
    finally:
        ros2_to_can.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()