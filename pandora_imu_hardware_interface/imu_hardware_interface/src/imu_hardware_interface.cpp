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
* Author:  George Kouros
*********************************************************************/
#include "imu_hardware_interface/imu_hardware_interface.h"

namespace pandora_hardware_interface
{
namespace imu
{
  ImuHardwareInterface::ImuHardwareInterface(
    ros::NodeHandle nodeHandle)
  :
    nodeHandle_(nodeHandle)
  {
    std::string device;
    if (nodeHandle_.getParam("device", device))
    {
      ROS_INFO("Selected Device: %s", device.c_str());

      if (device == "compass")
        comInterface_ = new ImuComInterface("/dev/compass", 38400, 100);
      else if (device == "trax")
        comInterface_ = new AhrsComInterface("/dev/trax", 38400, 100);
      else
      {
        ROS_FATAL(
          "device not set correctly in parameter server.");
        exit(-1);
      }
    }
    else
    {
      ROS_FATAL("device not set in parameter server.");
      exit(-1);
    }

    comInterface_->init();

    // initialize imu interface
    for (int ii = 0; ii < 3; ii++)
    {
      imuOrientation_[ii] = 0;
      imuAngularVelocity_[ii] = 0;
      imuLinearAcceleration_[ii] = 0;
    }
    imuOrientation_[3] = 1;

    imuData_.orientation = imuOrientation_;
    imuData_.angular_velocity = imuAngularVelocity_;
    imuData_.linear_acceleration = imuLinearAcceleration_;
    imuData_.name = "/sensors/imu";
    imuData_.frame_id = "base_link";

    hardware_interface::ImuSensorHandle imuSensorHandle(imuData_);
    imuSensorInterface_.registerHandle(imuSensorHandle);
    registerInterface(&imuSensorInterface_);

    imuRoll_ = new double;
    imuPitch_ = new double;
    imuYaw_ = new double;

    *imuRoll_ = 0;
    *imuPitch_ = 0;
    *imuYaw_ = 0;

    imuRPYData_.roll = imuRoll_;
    imuRPYData_.pitch = imuPitch_;
    imuRPYData_.yaw = imuYaw_;
    imuRPYData_.name = "/sensors/imu_rpy";
    imuRPYData_.frame_id = "base_link";

    pandora_hardware_interface::imu::ImuRPYHandle imuRPYHandle(imuRPYData_);
    imuRPYInterface_.registerHandle(imuRPYHandle);
    registerInterface(&imuRPYInterface_);

    // initialize dynamic reconfigure
    server_.setCallback(boost::bind(
      &pandora_hardware_interface::imu::ImuHardwareInterface::dynamicReconfigureCallback,
      this,
      _1,
      _2));

    // initialize diagnostics
    updater_.setHardwareID(device);
    updater_.add("Roll Upper Bound Check", this, &ImuHardwareInterface::rollUpperBoundDiagnostic);
    updater_.add("Roll Lower Bound Check", this, &ImuHardwareInterface::rollLowerBoundDiagnostic);
    updater_.add("Pitch Upper Bound Check", this, &ImuHardwareInterface::pitchUpperBoundDiagnostic);
    updater_.add("Pitch Lower Bound Check", this, &ImuHardwareInterface::pitchLowerBoundDiagnostic);
    updater_.add("Yaw Upper Bound Check", this, &ImuHardwareInterface::yawUpperBoundDiagnostic);
    updater_.add("Yaw Lower Bound Check", this, &ImuHardwareInterface::yawLowerBoundDiagnostic);
  }

  ImuHardwareInterface::~ImuHardwareInterface()
  {
    delete comInterface_;
  }

  void ImuHardwareInterface::read()
  {
    float yaw, pitch, roll, aV[3], lA[3];

    comInterface_->read();
    comInterface_->getData(
      &yaw,
      &pitch,
      &roll,
      aV,
      lA);

    // apply offsets to pitch, roll and yaw
    roll = roll + rollOffset_;
    pitch = pitch + pitchOffset_;
    yaw = yaw + yawOffset_;

    *imuYaw_ = static_cast<double>(yaw);
    *imuPitch_ = static_cast<double>(pitch);
    *imuRoll_ = static_cast<double>(roll);

    yaw = yaw * (2 * boost::math::constants::pi<double>()) / 360;
    pitch = pitch * (2 * boost::math::constants::pi<double>()) / 360;
    roll = roll * (2 * boost::math::constants::pi<double>()) / 360;

    geometry_msgs::Quaternion orientation;
    orientation = tf::createQuaternionMsgFromRollPitchYaw(roll, pitch, yaw);
    imuOrientation_[0] = orientation.x;
    imuOrientation_[1] = orientation.y;
    imuOrientation_[2] = orientation.z;
    imuOrientation_[3] = orientation.w;

    for (int ii = 0; ii < 3; ii++)
    {
      imuAngularVelocity_[ii] = static_cast<double>(aV[ii]);
      imuLinearAcceleration_[ii] = static_cast<double>(lA[ii]);
    }
  }

  void ImuHardwareInterface::dynamicReconfigureCallback(
    const pandora_imu_hardware_interface::ImuHardwareInterfaceConfig& config,
    uint32_t level)
  {
    rollOffset_ = config.roll_offset;
    pitchOffset_ = config.pitch_offset;
    yawOffset_ = config.yaw_offset;
  }

  void ImuHardwareInterface::rollUpperBoundDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat)
  {
    stat.add("Diagnostic Name", "Roll upper bound check");
    // roll bounds check
    if (*imuRoll_ > 180.0)
      stat.summary(diagnostic_msgs::DiagnosticStatus::ERROR, "ERROR");
    else
      stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
  }

  void ImuHardwareInterface::rollLowerBoundDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat)
  {
    stat.add("Diagnostics Name", "Roll lower bound check");
    if (*imuRoll_ < -180.0)
      stat.summary(diagnostic_msgs::DiagnosticStatus::ERROR, "ERROR");
    else
      stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
  }

  void ImuHardwareInterface::pitchUpperBoundDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat)
  {
    stat.add("Diagnostics Name", "Pitch upper bound check");
    if (*imuPitch_ > 90.0)
      stat.summary(diagnostic_msgs::DiagnosticStatus::ERROR, "ERROR");
    else
      stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
  }

  void ImuHardwareInterface::pitchLowerBoundDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat)
  {
    stat.add("Diagnostics Name", "Pitch lower bound check");
    if (*imuPitch_ < -90.0)
      stat.summary(diagnostic_msgs::DiagnosticStatus::ERROR, "ERROR");
    else
      stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
  }

  void ImuHardwareInterface::yawUpperBoundDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat)
  {
    stat.add("Diagnostics Name", "Yaw upper bound check");
    if (*imuYaw_ > 180.0)
      stat.summary(diagnostic_msgs::DiagnosticStatus::ERROR, "ERROR");
    else
      stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
  }

  void ImuHardwareInterface::yawLowerBoundDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat)
  {
    stat.add("Diagnostics Name", "Yaw lower bound check");
    if (*imuYaw_ < -180.0)
      stat.summary(diagnostic_msgs::DiagnosticStatus::ERROR, "ERROR");
    else
      stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
  }
}  // namespace imu
}  // namespace pandora_hardware_interface
