/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, PAL Robotics, S.L.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the PAL Robotics nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/*
 * Author: Konstantinos Panayiotou   <klpanagi@gmail.com>
 * Author: Konstantinos Zisis        <zisikons@gmail.com>
 * Author: Elisabet Papadopoulou     <papaelisabet@gmail.com >
 */

#ifndef MOTOR_CONTROLLERS_SKID_STEER_TORQUE_CONTROLLER_H
#define MOTOR_CONTROLLERS_SKID_STEER_TORQUE_CONTROLLER_H

// Include ros_control specific headers
#include <controller_interface/controller.h>
#include <hardware_interface/joint_command_interface.h>
#include <pluginlib/class_list_macros.h>
#include <pandora_sensor_msgs/TorqueMsg.h>

// Include ROS
#include <ros/ros.h>

namespace pandora_hardware_interface
{
namespace motor
{
  class SkidSteerTorqueController
      : public controller_interface::Controller<hardware_interface::EffortJointInterface>
  {
    public:
      // Controller Functionz
      SkidSteerTorqueController() {}

      /**
      * \brief Initialize controller
      * \param hw            Effort joint interface for the wheels
      * \param nh            ROS Node handle
      */
      bool init(hardware_interface::EffortJointInterface* hw,
                ros::NodeHandle &nh);

      void update(const ros::Time& time, const ros::Duration& period);
      void starting(const ros::Time& time) { }
      void stopping(const ros::Time& time) { }
      void commandCallback(const pandora_sensor_msgs::TorqueMsg& command);

    private:
      /// Hardware joint handles:
      hardware_interface::JointHandle left_front_wheel_joint_;
      hardware_interface::JointHandle right_front_wheel_joint_;
      hardware_interface::JointHandle left_rear_wheel_joint_;
      hardware_interface::JointHandle right_rear_wheel_joint_;

      // ROS subscriber :
      ros::Subscriber command_listener_;

      // Velocity command related struct
      struct TorqueCommands
      {
        double left_rear_wheel_torque;
        double left_front_wheel_torque;
        double right_rear_wheel_torque;
        double right_front_wheel_torque;
        ros::Time stamp;

        TorqueCommands():left_rear_wheel_torque(0.0),
                        left_front_wheel_torque(0.0),
                        right_rear_wheel_torque(0.0),
                       right_front_wheel_torque(0.0),
                                          stamp(0.0) {}
      };
      TorqueCommands command_struct_;
  };

  PLUGINLIB_EXPORT_CLASS(
    pandora_hardware_interface::motor::SkidSteerTorqueController,
    controller_interface::ControllerBase);

}  //  namespace motor
}  //  namespace pandora_hardware_interface

#endif  // MOTOR_CONTROLLERS_SKID_STEER_TORQUE_CONTROLLER_H
