/*********************************************************************
*
* Software License Agreement (BSD License)
*
* Copyright (c) 2014, P.A.N.D.O.R.A. Team.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided
* with the distribution.
* * Neither the name of the P.A.N.D.O.R.A. Team nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Author: George Kouros
*********************************************************************/

#include "leddar_usb_interface/leddar_usb_interface.h"
#include "pandora_leddar_hardware_interface/LeddarMsg.h"
#include <iostream>

using pandora_hardware_interface::leddar::LeddarUSBInterface;

LeddarHandle LeddarUSBInterface::leddarHandle_ = NULL;
LdDetection* LeddarUSBInterface::measurements_ = NULL;
int LeddarUSBInterface::leddarDetectionCount_;

int main(int argc, char** argv)
{
  ros::init(argc, argv, "leddar_usb_demo");
  ros::NodeHandle nodeHandle;
  
  ROS_INFO("Leddar USB demo node launched");  
  
  LeddarUSBInterface leddarUSBInterface("leddar", "AF44010"); 
  
  leddarUSBInterface.init();  

  ros::Duration(0.1).sleep();

  // create publisher to publish all the readings from the leddar
  ros::Publisher leddarPub = nodeHandle.advertise<
    pandora_leddar_hardware_interface::LeddarMsg>("leddar", 100);
  // create the msg to be published
  pandora_leddar_hardware_interface::LeddarMsg msg; 
  
  while (ros::ok())
  {
    // If a live connection is active we need to ping it periodically.
    leddarUSBInterface.ping();
    
    // clear the msg
    msg.leddar_distances.clear();
    msg.leddar_detection_count = 16;
    
    // fill the msg with the detections
    for (int ii=0; ii<LeddarUSBInterface::leddarDetectionCount_; ii++)
    {
      msg.leddar_distances.push_back(LeddarUSBInterface::measurements_[ii].mDistance);
    }

    // publish the msg
    leddarPub.publish(msg);

    //sleep for 0.5 seconds
    ros::Duration(0.5).sleep();
  }
  
  return 0;
}
