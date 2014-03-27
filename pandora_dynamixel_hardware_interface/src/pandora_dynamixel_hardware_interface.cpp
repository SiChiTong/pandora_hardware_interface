/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2014, P.A.N.D.O.R.A. Team.
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
*   * Neither the name of the P.A.N.D.O.R.A. Team nor the names of its
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
*
* Author:  Evangelos Apostolidis
*********************************************************************/
#include "pandora_dynamixel_hardware_interface/pandora_dynamixel_hardware_interface.h"

namespace pandora_dynamixel_hardware_interface
{
  PandoraDynamixelHardwareInterface::PandoraDynamixelHardwareInterface(
    ros::NodeHandle nodeHandle)
    : nodeHandle_(nodeHandle)
  {
    std::vector<std::string> jointNames = getJointNameFromParamServer();
    // connect and register the joint state interface
    for (int ii = 0; ii < jointNames.size(); ii++)
    {
      hardware_interface::JointStateHandle jointStateHandle(
        jointNames[ii],
        &position[ii],
        &velocity[ii],
        &effort[ii]);
      jointStateInterface_.registerHandle(jointStateHandle);
    }
    registerInterface(&jointStateInterface_);

    // connect and register the joint position interface
    for (int ii = 0; ii < jointNames.size(); ii++)
    {
      hardware_interface::JointHandle jointPositionHandle(
        jointStateInterface_.getHandle(jointNames[ii]),
        &command[ii]);
      positionJointInterface_.registerHandle(jointPositionHandle);
    }
    registerInterface(&positionJointInterface_);
  }

  PandoraDynamixelHardwareInterface::~PandoraDynamixelHardwareInterface()
  {
  }

  void PandoraDynamixelHardwareInterface::read()
  {
  }

  void PandoraDynamixelHardwareInterface::write()
  {
  }

  std::vector<std::string>
    PandoraDynamixelHardwareInterface::getJointNameFromParamServer()
  {
    std::vector<std::string> jointNames;
    std::string name;
    nodeHandle_.getParam(
      "motor_joints/stabilizer_joints/pitch_joint",
      name);
    jointNames.push_back(name);
    nodeHandle_.getParam(
      "motor_joints/stabilizer_joints/roll_joint",
      name);
    jointNames.push_back(name);
    nodeHandle_.getParam(
      "motor_joints/kinect_orientation_joints/pitch_joint",
      name);
    jointNames.push_back(name);
    nodeHandle_.getParam(
      "motor_joints/kinect_orientation_joints/yaw_joint",
      name);
    jointNames.push_back(name);

    return jointNames;
  }
}  // namespace pandora_dynamixel_hardware_interface
