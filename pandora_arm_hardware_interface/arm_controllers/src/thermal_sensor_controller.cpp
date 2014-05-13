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
#include "arm_controllers/thermal_sensor_controller.h"

namespace pandora_hardware_interface
{
namespace arm
{
  bool ThermalSensorController::init(
    ThermalSensorInterface*
      thermalSensorInterface,
    ros::NodeHandle& rootNodeHandle,
    ros::NodeHandle& controllerNodeHandle)
  {
    rootNodeHandle_ = &rootNodeHandle;
    thermalSensorInterface_ = thermalSensorInterface;
    // Get sensor names from interface
    const std::vector<std::string>& thermalSensorNames =
      thermalSensorInterface_->getNames();

    // Read publish rate from yaml
    if (!controllerNodeHandle.getParam("publish_rate", publishRate_))
    {
      publishRate_ = 50;
      ROS_ERROR(
        "Parameter 'publish_rate' not set in yaml, using default 50Hz");
    }

    for (int ii = 0; ii < thermalSensorNames.size(); ii++)
    {
      // Get sensor handles from interface
      sensorHandles_.push_back(
        thermalSensorInterface_->getHandle(thermalSensorNames[ii]));

      // Create publisher for each controller
      ImageRealtimePublisher publisher(
        new realtime_tools::RealtimePublisher<sensor_msgs::Image>(
          *rootNodeHandle_, thermalSensorNames[ii], 4));
      realtimePublishers_.push_back(publisher);
    }

    // resize last times published
    lastTimePublished_.resize(thermalSensorNames.size());
    return true;
  }

  void ThermalSensorController::starting(const ros::Time& time)
  {
    // Initialize last time published
    for (int ii = 0; ii < lastTimePublished_.size(); ii++)
    {
      lastTimePublished_[ii] = time;
    }
  }

  void ThermalSensorController::update(
    const ros::Time& time, const ros::Duration& period)
  {
    // Publish messages
    for (int ii = 0; ii < realtimePublishers_.size(); ii++)
    {
      if (lastTimePublished_[ii] + ros::Duration(1.0/publishRate_) < time)
      {
        if (realtimePublishers_[ii]->trylock())
        {
          lastTimePublished_[ii] =
            lastTimePublished_[ii] + ros::Duration(1.0/publishRate_);

          realtimePublishers_[ii]->msg_.header.stamp = time;
          realtimePublishers_[ii]->msg_.header.frame_id =
            sensorHandles_[ii].getFrameId();
          realtimePublishers_[ii]->msg_.encoding =
            sensor_msgs::image_encodings::MONO8;

          int height, width;
          height = *sensorHandles_[ii].getHeight();
          realtimePublishers_[ii]->msg_.height = height;
          width= *sensorHandles_[ii].getWidth();
          realtimePublishers_[ii]->msg_.width = width;
          realtimePublishers_[ii]->msg_.step = *sensorHandles_[ii].getStep();

          realtimePublishers_[ii]->msg_.data.clear();
          uint8_t* data = sensorHandles_[ii].getData();
          for (int jj = 0; jj < height * width; jj++)
          {
            realtimePublishers_[ii]->msg_.data.push_back(data[jj]);
          }

          realtimePublishers_[ii]->unlockAndPublish();
        }
      }
    }
  }

  void ThermalSensorController::stopping(const ros::Time& time)
  {
  }

  ThermalSensorController::ThermalSensorController()
  {
  }

  ThermalSensorController::~ThermalSensorController()
  {
  }
}  // namespace arm
}  // namespace pandora_hardware_interface

PLUGINLIB_EXPORT_CLASS(
  pandora_hardware_interface::arm::ThermalSensorController,
  controller_interface::ControllerBase)
