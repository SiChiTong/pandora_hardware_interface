/***************************************************************************
*   Copyright (C) 2010-2011 by Charalampos Serenis <info@devel.serenis.gr>*
*   Author: Charalampos Serenis <info@devel.serenis.gr>                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,  *
*   MA 02110-1301, USA.                                                   *
***************************************************************************/
#ifndef EPOS_HANDLER_ABSTRACT_EPOS_HANDLER_H
#define EPOS_HANDLER_ABSTRACT_EPOS_HANDLER_H

#include "ros/ros.h"
#include <boost/scoped_ptr.hpp>
#include <epos_gateway/epos_serial_gateway.h>
#include <stdint.h>

namespace pandora_hardware_interface
{
namespace motor
{
  class Error
  {
   public:
    uint32_t leftRear;
    uint32_t rightRear;
    uint32_t leftFront;
    uint32_t rightFront;

    Error();
    ~Error();
  };

  class AbstractEposHandler
  {
   public:
    AbstractEposHandler();
    virtual ~AbstractEposHandler();

    virtual void getRPM(
      int* leftRearRpm, int* leftFrontRpm,
      int* rightRearRpm, int* rightFrontRpm) = 0;

    virtual void getCurrent(
      int* leftRearCurrent, int* leftFrontCurrent,
      int* rightRearCurrent, int* rightFrontCurrent) = 0;

    virtual Error getError() = 0;
    virtual uint16_t  writeRPM(const int leftRpm, const int rightRpm) = 0;

    virtual void getTorque(
        double* leftRearTorque,
        double* leftFrontTorque,
        double* rightRearTorque,
        double* rightFrontTorque);

   protected:
    uint32_t encodeToControlWord(const int& left, const int& right);

   protected:
    boost::scoped_ptr<AbstractEposGateway> gatewayImpl_;
  };

}  // namespace motor
}  // namespace pandora_hardware_interface

#endif  // EPOS_HANDLER_ABSTRACT_EPOS_HANDLER_H
