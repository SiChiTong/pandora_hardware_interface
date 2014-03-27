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
#include "pandora_imu_hardware_interface/pandora_imu_hardware_interface.h"

namespace pandora_imu_hardware_interface
{
  PandoraImuHardwareInterface::PandoraImuHardwareInterface(
    ros::NodeHandle nodeHandle)
  :
    nodeHandle_(nodeHandle),
    imuSerialInterface(
      "/dev/ttyUSB0",
      38400,
      100)

  {
    // connect and register imu sensor interface
    imuData_.orientation = new double(4);
    imuData_.orientation[0] = 0;
    imuData_.orientation[1] = 0;
    imuData_.orientation[2] = 0;
    imuData_.orientation[3] = 1;
    imuData_.name="/sensors/imu";  // /sensors might become namespace
    imuData_.frame_id="base_link";
    hardware_interface::ImuSensorHandle imuSensorHandle(imuData_);
    imuSensorInterface_.registerHandle(imuSensorHandle);
    registerInterface(&imuSensorInterface_);
  }

  PandoraImuHardwareInterface::~PandoraImuHardwareInterface()
  {
  }

  void PandoraImuHardwareInterface::read()
  {
    float yaw, pitch, roll;
    imuSerialInterface.read();
    imuSerialInterface.getData(&yaw, &pitch, &roll);

    tf::Quaternion orientation;
    orientation.setEuler(yaw, pitch, roll);
    imuData_.orientation[0] = orientation.getAxis()[0];
    imuData_.orientation[1] = orientation.getAxis()[1];
    imuData_.orientation[2] = orientation.getAxis()[2];
    imuData_.orientation[3] = orientation.getW();
  }

}  // namespace pandora_imu_hardware_interface
