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
* Author: George Kouros
*********************************************************************/

#ifndef PANDORA_IMU_HARDWARE_INTERFACE_ABSTRACT_IMU_SERIAL_INTERFACE_H
#define PANDORA_IMU_HARDWARE_INTERFACE_ABSTRACT_IMU_SERIAL_INTERFACE_H

#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <stdexcept>

#include <ros/ros.h>
#include <serial/serial.h>

namespace pandora_hardware_interface
{
namespace imu
{
  /**
   @class AbstractImuSerialInterface
   @brief Abstract Serial Interface class inherited by Compass OS-4000 
   serial interface and Trax AHRS serial interface 
  **/
  class AbstractImuSerialInterface : private boost::noncopyable
  {
   public:
    /**
     @brief Default Constructor
     @param device [std::string &] : IMU device com port name
     @param speed [int] : Serial communication speed (baud rate)
     @param timeout [int] : Connection response timeout
    **/
    AbstractImuSerialInterface(
       const std::string& device,
        int speed,
        int timeout)
     :
      device_(device),
      speed_(speed),
      timeout_(timeout),
      yaw_(0),
      pitch_(0),
      roll_(0),
      serialPtr_(NULL)
    {
    }


    /**
     @brief Establishes serial communication
     @return void
    **/
    virtual void init() = 0;

    /**
     @brief Reads raw data from the IMU and calculates yaw, pitch and roll 
     @details Init must be called first to establish serial communication
     @return void
    **/
    virtual void read() = 0;

    /**
     @brief Get roll value
     @return float roll
    **/
    inline float getRoll() const
    {
      return roll_;
    }

    /**
     @brief Get pitch latest meausurement
     @return float pitch
    **/
    inline float getPitch() const
    {
      return pitch_;
    }

    /**
     @brief Get yaw latest measurement
     @return float yaw
    **/
    inline float getYaw() const
    {
      return yaw_;
    }

    /**
     @brief Get yaw, pitch and roll
     @param yaw [float*] : used to return latest yaw measurement
     @param pitch [float*] : used to return latest pitch measurement
     @param roll [float*] : used to return latest roll measurement
     @return void 
    **/
    inline void getData(
      float* yaw,
      float* pitch,
      float* roll) const
    {
      *yaw = yaw_;
      *pitch = pitch_;
      *roll = roll_;
    }

   protected:
    /**
     @brief Transform raw IMU data to yaw, pitch and roll meausurements
     @param packet [std::string&] : packet containing the raw imu data
     @return void
    **/
    virtual void parse(const std::string& packet) = 0;

    /**
     @brief Check size of latest received IMU data packet
     @return bool
    **/
    virtual bool check(const std::string& packet, int crc) = 0;

   protected:
    float yaw_;  //!< latest yaw measurement
    float pitch_;  //!< latest pitch measurement
    float roll_;  //!< latest roll measurement

    const std::string device_;  //!< IMU device com port name
    const int speed_;  //!< serial communication speed (baud rate)
    const int timeout_;  //!< Connection response timeout

    boost::scoped_ptr<serial::Serial> serialPtr_;  //!< serial communication class instance
  };
}  // namespace imu
}  // namespace pandora_hardware_interface

#endif  // PANDORA_IMU_HARDWARE_INTERFACE_ABSTRACT_IMU_SERIAL_INTERFACE_H
