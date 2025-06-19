import launch
from launch_ros.actions import LifecycleNode

def generate_launch_description():
    ld = launch.LaunchDescription()

    joy_vel_convert = LifecycleNode(
        package = 'control_pkg',
        executable = 'joy_vel_converter',
        name = "joy_vel_converter",
        namespace='yasarobo'
    )

    ld.add_action(joy_vel_convert)

    return ld