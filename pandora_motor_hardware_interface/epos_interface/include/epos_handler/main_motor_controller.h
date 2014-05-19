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

#ifndef MAINMOTORCONTROLLER_H
#define MAINMOTORCONTROLLER_H

#include "ros/ros.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <epos_gateway/epos_serial_gateway.h>
#include <fstream>
#include "epos_handler/serial_epos_handler.h"
#include "epos_handler/fake_epos_handler.h"
#include "diagnostic_updater/diagnostic_updater.h"

using namespace pandora_hardware_interface::motor;


class MainMotorController
{

  std::ofstream outfile;
  AbstractEposHandler *motors;
  ros::Publisher main_motor_measurements;
  ros::NodeHandle m_handle;
  epos::CommandStatus motorStatus;
  ros::Timer timer;
  bool softBusy;
  boost::mutex guard;
  //For Diagnostics about motor status and setVehicleSpeed Calls
  ros::Time lastSetSpeedCall;
  bool falseSpeedCall;
  ros::Duration timeBetweenCalls;

  /**
  * The diagnostic updater
  */
  diagnostic_updater::Updater _updater;



public:



  MainMotorController(std::string dev,
                      int speed,
                      int timeout,
                      ros::NodeHandle &handle,
                      float period);
  ~MainMotorController();
  epos::CommandStatus setVelocity(float linear);
  void getVelocity(float linear);



  /**
   * \brief post the controller status for health monitoring
   *
   * callback function for ros timer. Post the controller status in topic
   * /mainMotorControl/status
   * \param[in] event a ros::TimerEvent used to call the callback function
   */
  void postStatus(const ros::TimerEvent& event);




  /**
   * \brief unknown function
   *
   * functions added by diagnostic team, dont know what they do!
   *
   */
  void speedDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat);
  /**
   * \brief unknown function
   *
   * functions added by diagnostic team, dont know what they do!
   *
   */
  void statusDiagnostic(diagnostic_updater::DiagnosticStatusWrapper &stat);

  /**
  * \brief convert epos::CommandStatus to a human readable string
  *
  * \param[in] status the status code that we wish convert to string
  * \return a human readable string describing the error
  */
  std::string convertStatusToString(epos::CommandStatus status);

  bool calledTooOften();


};



#endif
