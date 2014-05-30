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
#ifndef ARM_HARDWARE_INTERFACE_THERMAL_SENSOR_INTERFACE_H
#define ARM_HARDWARE_INTERFACE_THERMAL_SENSOR_INTERFACE_H

#include <hardware_interface/internal/hardware_resource_manager.h>
#include <string>

namespace pandora_hardware_interface
{
namespace arm
{
  class ThermalSensorHandle
  {
  public:
    struct Data
    {
      Data()
      {
      }

      std::string name;
      std::string frameId;
      int* height;
      int* width;
      int* step;
      std::vector<int>* data;
    };

    ThermalSensorHandle(const Data& data = Data())
      : name_(data.name),
        frameId_(data.frameId),
        height_(data.height),
        width_(data.width),
        step_(data.step),
        data_(data.data)
    {
    }

    inline std::string getName() const
    {
      return name_;
    }
    inline std::string getFrameId() const
    {
      return frameId_;
    }
    inline const int* getHeight() const
    {
      return height_;
    }
    inline const int* getWidth() const
    {
      return width_;
    }
    inline const int* getStep() const
    {
      return step_;
    }
    inline const std::vector<int>* getData() const
    {
      return data_;
    }

  private:
    std::string name_;
    std::string frameId_;
    int* height_;
    int* width_;
    int* step_;
    std::vector<int>* data_;
  };

  class ThermalSensorInterface :
    public hardware_interface::HardwareResourceManager<ThermalSensorHandle>
  {
  };
}  // namespace arm
}  // namespace pandora_hardware_interface

#endif  // ARM_HARDWARE_INTERFACE_THERMAL_SENSOR_INTERFACE_H
