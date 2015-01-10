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
 * Author: Orestis Zachariadis
 *********************************************************************/

#ifndef ARM_USB_INTERFACE_ARM_USB_INTERFACE_H
#define ARM_USB_INTERFACE_ARM_USB_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <termios.h>

#include <ros/ros.h>

namespace pandora_hardware_interface
{
namespace arm
{

/**Command to send if you want encoder data*/
#define COMMAND_ENCODER 1
/**Command to send if you want left sonar's data*/
#define COMMAND_SONAR_LEFT 2
/**Command to send if you want right sonar's data*/
#define COMMAND_SONAR_RIGHT 3
/**Command to send if you want CO2 data*/
#define COMMAND_CO2 4
/**Command to send if you want motor battery data*/
#define COMMAND_BATTERY_MOTOR 5
/**Command to send if you want supply battery data*/
#define COMMAND_BATTERY_SUPPLY 6

#define CO2_NBYTES 4         ///< Number of bytes of incoming CO2 data
#define SONAR_NBYTES 2       ///< Number of bytes of incoming Sonar data
#define ENCODER_NBYTES 2     ///< Number of bytes of incoming Encoder data
#define BATTERY_NBYTES 2     ///< Number of bytes of incoming Battery data

#define COMMAND_NBYTES 1     ///< Number of bytes of outgoing command

//-----------------THE CODE BELOW IS USED ONLY FOR COMPILE--------------------

/**Command to send if you want center grideye's data*/
#define COMMAND_GEYE_CENTER 7
/**Command to send if you want left grideye's data*/
#define COMMAND_GEYE_LEFT 8
/**Command to send if you want right grideye's data*/
#define COMMAND_GEYE_RIGHT 9

#define GEYE_NBYTES 64       ///< Number of bytes of incoming GridEYE data



//-----------------THE CODE ABOVE IS USED ONLY FOR COMPILE-------------------

class ArmUSBInterface : private boost::noncopyable
{
public:
  ArmUSBInterface();

  ~ArmUSBInterface();

  void openUSBPort();

  /**
   * @attention If the uController detects a malfunction in a sensor it returns
   * zeros in place of its readings.
   * @note Although the uController can return the most recent reading whenever
   * it is asked, there is no point asking for data much more frequently than
   * two times the sensor speed. This sensor's frequency is 10 Hz.
   * @param[in] grideyeSelect 'C' for Center GridEYE, 'L' for left,
   *  'R' for right
   * @param[in,out] values pointer to an uint8[64] array
   * @returns 1 for a successful read, -1 for write error, -2 for read error,
   *  -3 for incorrect number of bytes read
   */
   
//-------THIS CODE IS NEEDED ONLY FOR COMPILE----------------------------
  int grideyeValuesGet(const char& grideyeSelect, uint8_t * values);
//--------------------------------------------------------------------------

  uint16_t sonarValuesRead(const char& sonarSelect);

  /**
   * @attention If the uController detects a malfunction in a sensor it returns
   * zeros in place of its readings.
   * @note Although the uController can return the most recent reading whenever
   * it is asked, there is no point asking for data much more frequently than
   * two times the sensor speed. This sensor's frequency is 2 Hz.
   * @returns Gas reading in percent volume if read was successful,
   *  -1 for write error, -2 for read error,
   *  -3 for incorrect number of bytes read
   */
  float co2ValueGet();     //Should be renamed to co2ValueRead();

  uint16_t encoderValueRead();

  uint16_t batteryValuesRead(const char& batterySelect);

private:
  void reconnectUSB();
  int fd;  ///< File Descriptor
};

}  // namespace arm
}  // namespace pandora_hardware_interface

#endif  // ARM_USB_INTERFACE_ARM_USB_INTERFACE_H
