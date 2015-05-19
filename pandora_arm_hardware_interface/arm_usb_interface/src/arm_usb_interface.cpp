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
 * Author: Nikos Taras
 *********************************************************************/

#include "arm_usb_interface/arm_usb_interface.h"

namespace pandora_hardware_interface
{
namespace arm
{

  ArmUsbInterface::ArmUsbInterface()
  {
    openUsbPort();
  }


  ArmUsbInterface::~ArmUsbInterface()
  {
    close(fd);
    ROS_INFO("[Head]: USB port closed because of program termination\n");
  }


  //  fcntl(fd, F_SETFL, FNDELAY);    //make read() non-blocking
  //  fcntl(fd, F_SETFL, 0);  //make read() blocking
  void ArmUsbInterface::openUsbPort()
  {
    int timeout = 0;
    // To make read non-blocking use the following:
    // fd = open("/dev/head", O_RDWR | O_NOCTTY | O_NDELAY);
    while ((fd = open("/dev/head", O_RDWR | O_NOCTTY)) == -1)
    {
      ROS_ERROR("[Head]: cannot open USB port\n");
      ROS_ERROR("[Head]: open() failed with error [%s]\n", strerror(errno));

      ros::Duration(0.5).sleep();

      if (timeout > 100)
        throw std::runtime_error("[Head]: cannot open USB port");
      else
        timeout++;
    }

    ROS_INFO("[Head]: USB port successfully opened\n");

    // Needs some time to initialize, even though it opens succesfully.
    // tcflush() didn't work without waiting at least 8 ms
    ros::Duration(0.03).sleep();

    // To save time you can see and change terminal settings in command line with stty command,
    // before implementing in software. Note: stty prefixes disabled flags with a dash.
    // See:  http://man7.org/linux/man-pages/man3/termios.3.html
    struct termios tios;

    if (tcgetattr(fd, &tios) < 0)
      ROS_ERROR("init_serialport: Couldn't get term attributes\n");

    tios.c_lflag &= ~(ICANON | ISIG | ECHO |  IEXTEN | ECHOE | ECHOK);

    tios.c_iflag &= ~(ICRNL | IXON);

    tios.c_oflag &= ~(OPOST);

    tios.c_cc[VTIME] = 1;  // set timeout to 100ms

    if (tcsetattr(fd, TCSANOW, &tios) < 0)
      ROS_ERROR("init_serialport: Couldn't set term attributes\n");
  }

  int ArmUsbInterface::readData(
    int fd, uint8_t bufOut, uint8_t read_bytes, uint8_t* readBuf)
  {
    // flush both data received but not read and data written but not transmitted
    tcflush(fd, TCIOFLUSH);
    
    
    fd_set set;
    struct timeval timeout;
    int rv;
    char buff[100];
    int len = 100;
    int filedesc = open( "dev/ttyS0", O_RDWR );

    FD_ZERO(&set); /* clear the set */
    FD_SET(fd, &set); /* add our file descriptor to the set */

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    
    
    
    
    
    
    
    
    
    
    
    

    int nr;

    nr = write(fd, &bufOut, 1);
    if (nr != 1)
    {
      ROS_ERROR("[Head]: Write Error\n");
      reconnectUsb();
      return WRITE_ERROR;
    }

    //------------- READ NACK -------------
    union
    {
      uint8_t nackBufInUint8[NACK_NBYTES];
      uint16_t nackBufInUint16;
    };

    nr = read(fd, nackBufInUint8, NACK_NBYTES);
    if (nr < 0)
    {
      ROS_ERROR("[Head]: Read Error\n");
      reconnectUsb();
      return READ_ERROR;
    }
    else if (nr != NACK_NBYTES)
    {
      ROS_ERROR("[Head]: Wrong number of bytes read\n");
      reconnectUsb();
      return INCORRECT_NUM_OF_BYTES;
    }

    if (!(nackBufInUint16 == ACK))
    {
      ROS_ERROR("[Head]: Received NACK\n");
      return RECEIVED_NACK;
    }
// ------------------------------------------------------
    rv = select(filedesc + 1, &set, NULL, NULL, &timeout);
    if(rv == -1){
      ROS_ERROR("select error!!!!!!!!!!"); /* an error accured */
      return SELECT_ERROR;
    }
    else if(rv == 0){
      ROS_INFO("timeout!!!!!!!!!!!!!!!!"); /* a timeout occured */
      return READ_TIMEOUT;
    }
    
    nr = read(fd, readBuf, read_bytes);  // blocking
    if (nr < 0)
    {
      ROS_ERROR("[Head]: Read Error\n");
      reconnectUsb();
      return READ_ERROR;
    }
    else if (nr != read_bytes)
    {
      ROS_ERROR("[Head]: Wrong number of bytes read\n");
      reconnectUsb();
      return INCORRECT_NUM_OF_BYTES;
    }
    else
    {
      return NO_ERROR;
    }
  }


  int ArmUsbInterface::readGrideyeValues(
    const char& grideyeSelect, uint8_t* values)
  {
    int nr;
    uint8_t bufOut;

    switch (grideyeSelect)
    {
      case 'C':
        bufOut = COMMAND_GEYE_CENTER;
        break;
      case 'L':
        bufOut = COMMAND_GEYE_LEFT;
        break;
      case 'R':
        bufOut = COMMAND_GEYE_RIGHT;
        break;
      default:
        ROS_ERROR(
          "Undefined Grideye Selection! Center Grideye selected by  default.");
        bufOut = COMMAND_GEYE_CENTER;
        break;
    }

    int ret = readData(fd, bufOut, GEYE_NBYTES, values);
    return ret;
  }


  int ArmUsbInterface::readSonarValues(
    const char& sonarSelect, uint16_t* value)
  {
    union
    {
      uint8_t sonarBufIn_uint8[SONAR_NBYTES];
      uint16_t sonarBufIn_uint16;
    };

    int nr;
    uint8_t bufOut;

    switch (sonarSelect)
    {
      case 'L':
        bufOut = COMMAND_SONAR_LEFT;
        break;
      case 'R':
        bufOut = COMMAND_SONAR_RIGHT;
        break;
      default:
        bufOut = COMMAND_SONAR_LEFT;
        ROS_ERROR("Undefined Sonar Selection! Left Sonar selected by default.");
        break;
    }

    int ret = readData(fd, bufOut, SONAR_NBYTES, sonarBufIn_uint8);
    *value = sonarBufIn_uint16;
    return ret;
  }


  int ArmUsbInterface::readCo2Value(float* value)
  {
    union
    {
      uint8_t CO2BufInUint8[CO2_NBYTES];
      float CO2BufInFloat;
    };

    int nr;
    uint8_t bufOut = COMMAND_CO2;
    int ret = readData(fd, bufOut, CO2_NBYTES, CO2BufInUint8);
    *value = CO2BufInFloat;
    return ret;
  }

  int ArmUsbInterface::readEncoderValue(uint16_t* value)
  {
    union
    {
      uint8_t encoderBufInUint8[ENCODER_NBYTES];
      uint16_t encoderBufInUint16;
    };

    int nr;
    uint8_t bufOut = COMMAND_ENCODER;
    int ret = readData(fd, bufOut, ENCODER_NBYTES, encoderBufInUint8);
    *value = encoderBufInUint16;
    return ret;
  }


  int ArmUsbInterface::readBatteryValues(
    const char& batterySelect, uint16_t* value)
  {
    union
    {
      uint8_t batteryBufInUint8[BATTERY_NBYTES];
      uint16_t batteryBufInUint16;
    };

    int nr;
    uint8_t bufOut;

    switch (batterySelect)
    {
      case 'M':
        bufOut = COMMAND_BATTERY_MOTOR;
        break;
      case 'S':
        bufOut = COMMAND_BATTERY_SUPPLY;
        break;
      default:
        ROS_ERROR(
          "Undefined Battery selection. Motor battery selected by default");
        bufOut = COMMAND_BATTERY_MOTOR;  // shouldn't get in there
        break;
    }

    int ret = readData(fd, bufOut, BATTERY_NBYTES, batteryBufInUint8);
    *value = batteryBufInUint16;
    return ret;
  }

  void ArmUsbInterface::reconnectUsb()
  {
    // reconnectUsb() should be called until communication is restored.
    close(fd);
    ROS_INFO("[Head]: USB port closed\n");
    ros::Duration(1.5).sleep();

    openUsbPort();

    ROS_INFO("[Head]: USB port reconnected successfully");
  }

}  // namespace arm
}  // namespace pandora_hardware_interface
