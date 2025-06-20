import launch
from launch_ros.actions import LifecycleNode
from launch_ros.events.lifecycle import ChangeState
import lifecycle_msgs
import lifecycle_msgs.msg

def generate_launch_description():
    ld = launch.LaunchDescription()

    joy_vel_convert = LifecycleNode(
        package = 'control_pkg',
        executable = 'joy_vel_converter',
        name = "joy_vel_converter",
        namespace='yasarobo'
    )

    to_configure = launch.actions.EmitEvent(
        event = ChangeState(
            lifecycle_node_matcher=launch.events.matches_action(joy_vel_convert),
            transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
        )
    )


    ld.add_action(joy_vel_convert)
    ld.add_action(to_configure)

    return ld