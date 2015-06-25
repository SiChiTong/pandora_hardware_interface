/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, PAL Robotics, S.L.
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
 *   * Neither the name of the PAL Robotics nor the names of its
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
 *********************************************************************/

/*
 * Author: Bence Magyar
 * Author: Evangelos Apostolidis
 */

#include <tf/transform_datatypes.h>

#include <urdf_parser/urdf_parser.h>

#include <boost/assign.hpp>

#include <motor_controllers/skid_steer_drive_controller.h>

static double euclideanOfVectors(const urdf::Vector3& vec1, const urdf::Vector3& vec2)
{
  return std::sqrt(
    std::pow(vec1.x-vec2.x, 2) +
    std::pow(vec1.y-vec2.y, 2) +
    std::pow(vec1.z-vec2.z, 2));
}

/*
 * \brief Check if the link is modeled as a cylinder
 * \param link Link
 * \return true if the link is modeled as a Cylinder; false otherwise
 */
static bool isCylinder(const boost::shared_ptr<const urdf::Link>& link)
{
  if (!link)
  {
    ROS_ERROR("Link == NULL.");
    return false;
  }

  if (!link->collision)
  {
    ROS_ERROR_STREAM("Link " << link->name <<
      " does not have collision description."
      "Add collision description for link to urdf.");
    return false;
  }

  if (!link->collision->geometry)
  {
    ROS_ERROR_STREAM("Link " << link->name <<
      " does not have collision geometry description."
      "Add collision geometry description for link to urdf.");
    return false;
  }

  if (link->collision->geometry->type != urdf::Geometry::CYLINDER)
  {
    ROS_DEBUG_STREAM("Link " << link->name << " does not have cylinder geometry");
    return false;
  }

  return true;
}

/*
 * \brief Get the wheel radius
 * \param [in]  wheel_link   Wheel link
 * \param [out] wheel_radius Wheel radius [m]
 * \return true if the wheel radius was found; false otherwise
 */
static bool getWheelRadius(const boost::shared_ptr<const urdf::Link>& wheel_link, double& wheel_radius)
{
  if (!isCylinder(wheel_link))
  {
    ROS_DEBUG_STREAM("Wheel link " << wheel_link->name << " is NOT modeled as a cylinder!");
    return false;
  }

  wheel_radius = (static_cast<urdf::Cylinder*>(wheel_link->collision->geometry.get()))->radius;
  return true;
}

namespace pandora_hardware_interface
{
namespace motor
{

  SkidSteerDriveController::SkidSteerDriveController()
    : command_struct_()
    , wheel_separation_(0.0)
    , wheel_radius_(0.0)
    , wheel_separation_multiplier_(1.0)
    , wheel_radius_multiplier_(1.0)
    , cmd_vel_timeout_(0.5)
    , base_frame_id_("base_link")
    , odometry_(2)
  {
  }

  bool SkidSteerDriveController::init(hardware_interface::VelocityJointInterface* hw,
            ros::NodeHandle& root_nh,
            ros::NodeHandle &controller_nh)
  {
    const std::string complete_ns = controller_nh.getNamespace();
    std::size_t id = complete_ns.find_last_of("/");
    name_ = complete_ns.substr(id + 1);
    // Get joint names from the parameter server
    std::string left_front_wheel_name, right_front_wheel_name;
    std::string left_rear_wheel_name, right_rear_wheel_name;

    root_nh.param<bool>("sim", sim_, false);

    bool res = controller_nh.hasParam("left_front_wheel");
    if (!res || !controller_nh.getParam("left_front_wheel", left_front_wheel_name))
    {
      ROS_ERROR_NAMED(name_, "Couldn't retrieve left front wheel name from param server.");
      return false;
    }
    res = controller_nh.hasParam("right_front_wheel");
    if (!res || !controller_nh.getParam("right_front_wheel", right_front_wheel_name))
    {
      ROS_ERROR_NAMED(name_, "Couldn't retrieve right front wheel name from param server.");
      return false;
    }
    res = controller_nh.hasParam("left_rear_wheel");
    if (!res || !controller_nh.getParam("left_rear_wheel", left_rear_wheel_name))
    {
      ROS_ERROR_NAMED(name_, "Couldn't retrieve left rear wheel name from param server.");
      return false;
    }
    res = controller_nh.hasParam("right_rear_wheel");
    if (!res || !controller_nh.getParam("right_rear_wheel", right_rear_wheel_name))
    {
      ROS_ERROR_NAMED(name_, "Couldn't retrieve right rear wheel name from param server.");
      return false;
    }

    res = controller_nh.hasParam("wheel_radius");
    if (!res || !controller_nh.getParam("wheel_radius", wheel_radius_))
    {
      ROS_WARN_NAMED(name_, "Couldn't retrieve left front wheel radius from param server.");
      wheel_radius_ = -1;
    }

    double publish_rate;
    controller_nh.param("publish_rate", publish_rate, 50.0);
    ROS_INFO_STREAM_NAMED(name_, "Controller state will be published at "
                          << publish_rate << "Hz.");
    publish_period_ = ros::Duration(1.0 / publish_rate);

    controller_nh.param("wheel_separation_multiplier", wheel_separation_multiplier_, wheel_separation_multiplier_);
    ROS_INFO_STREAM_NAMED(name_, "Wheel separation will be multiplied by "
                          << wheel_separation_multiplier_ << ".");

    controller_nh.param("wheel_radius_multiplier", wheel_radius_multiplier_, wheel_radius_multiplier_);
    ROS_INFO_STREAM_NAMED(name_, "Wheel radius will be multiplied by "
                          << wheel_radius_multiplier_ << ".");

    controller_nh.param("cmd_vel_timeout", cmd_vel_timeout_, cmd_vel_timeout_);
    ROS_INFO_STREAM_NAMED(name_, "Velocity commands will be considered old if they are older than "
                          << cmd_vel_timeout_ << "s.");

    controller_nh.param("base_frame_id", base_frame_id_, base_frame_id_);
    ROS_INFO_STREAM_NAMED(name_, "Base frame_id set to " << base_frame_id_);

    // Velocity and acceleration limits:
    controller_nh.param("linear/x/has_velocity_limits",
      limiter_lin_.has_velocity_limits, limiter_lin_.has_velocity_limits);
    controller_nh.param("linear/x/has_acceleration_limits",
      limiter_lin_.has_acceleration_limits, limiter_lin_.has_acceleration_limits);
    controller_nh.param("linear/x/max_velocity",
      limiter_lin_.max_velocity,  limiter_lin_.max_velocity);
    controller_nh.param("linear/x/min_velocity",
      limiter_lin_.min_velocity, -limiter_lin_.max_velocity);
    controller_nh.param("linear/x/max_acceleration",
      limiter_lin_.max_acceleration,  limiter_lin_.max_acceleration);
    controller_nh.param("linear/x/min_acceleration",
      limiter_lin_.min_acceleration, -limiter_lin_.max_acceleration);

    controller_nh.param("angular/z/has_velocity_limits",
      limiter_ang_.has_velocity_limits, limiter_ang_.has_velocity_limits);
    controller_nh.param("angular/z/has_acceleration_limits",
      limiter_ang_.has_acceleration_limits, limiter_ang_.has_acceleration_limits);
    controller_nh.param("angular/z/max_velocity",
      limiter_ang_.max_velocity,  limiter_ang_.max_velocity);
    controller_nh.param("angular/z/min_velocity",
      limiter_ang_.min_velocity, -limiter_ang_.max_velocity);
    controller_nh.param("angular/z/max_acceleration",
      limiter_ang_.max_acceleration,  limiter_ang_.max_acceleration);
    controller_nh.param("angular/z/min_acceleration",
      limiter_ang_.min_acceleration, -limiter_ang_.max_acceleration);

    controller_nh.param("linear_fit_degree", linearFitDegree_, 5);
    controller_nh.param("angular_fit_degree", angularFitDegree_, 7);

    if (!setOdomParamsFromUrdf(root_nh, left_front_wheel_name, right_front_wheel_name))
      return false;

    setOdomPubFields(root_nh, controller_nh);

    // Get the joint object to use in the realtime loop
    ROS_INFO_STREAM_NAMED(name_,
                          "Adding left front wheel with joint name: " << left_front_wheel_name
                          << ", right front wheel with joint name: " << right_front_wheel_name
                          << ", left rear wheel with joint name: " << left_rear_wheel_name
                          << " and right rear wheel with joint name: " << right_rear_wheel_name);
    left_front_wheel_joint_ = hw->getHandle(left_front_wheel_name);  // throws on failure
    right_front_wheel_joint_ = hw->getHandle(right_front_wheel_name);  // throws on failure
    left_rear_wheel_joint_ = hw->getHandle(left_rear_wheel_name);  // throws on failure
    right_rear_wheel_joint_ = hw->getHandle(right_rear_wheel_name);  // throws on failure

    sub_command_ = controller_nh.subscribe("/cmd_vel", 1, &SkidSteerDriveController::cmdVelCallback, this);

    XmlRpc::XmlRpcValue linearMeasurementsList;
    res = controller_nh.hasParam("measured_linear_velocities");
    if (res && controller_nh.getParam("measured_linear_velocities", linearMeasurementsList))
    {
      ROS_ASSERT(
        linearMeasurementsList.getType() == XmlRpc::XmlRpcValue::TypeArray);

      std::string key;
      for (int ii = 0; ii < linearMeasurementsList.size(); ii++)
      {
        ROS_ASSERT(
          linearMeasurementsList[ii].getType() == XmlRpc::XmlRpcValue::TypeStruct);

        key = "expected";
        ROS_ASSERT(
          linearMeasurementsList[ii][key].getType() == XmlRpc::XmlRpcValue::TypeDouble);
        expectedLinear_.push_back(
          static_cast<double>(linearMeasurementsList[ii][key]));

        key = "actual";
        ROS_ASSERT(
          linearMeasurementsList[ii][key].getType() == XmlRpc::XmlRpcValue::TypeDouble);
        actualLinear_.push_back(
          static_cast<double>(linearMeasurementsList[ii][key]));
      }
    }

    XmlRpc::XmlRpcValue angularMeasurementsList;
    res = controller_nh.hasParam("measured_angular_velocities");
    if (res && controller_nh.getParam("measured_angular_velocities", angularMeasurementsList))
    {
      ROS_ASSERT(
        angularMeasurementsList.getType() == XmlRpc::XmlRpcValue::TypeArray);

      std::string key;
      for (int ii = 0; ii < angularMeasurementsList.size(); ii++)
      {
        ROS_ASSERT(
          angularMeasurementsList[ii].getType() == XmlRpc::XmlRpcValue::TypeStruct);

        key = "expected";
        ROS_ASSERT(
          angularMeasurementsList[ii][key].getType() == XmlRpc::XmlRpcValue::TypeDouble);
        expectedAngular_.push_back(
          static_cast<double>(angularMeasurementsList[ii][key]));

        key = "actual";
        ROS_ASSERT(
          angularMeasurementsList[ii][key].getType() == XmlRpc::XmlRpcValue::TypeDouble);
        actualAngular_.push_back(
          static_cast<double>(angularMeasurementsList[ii][key]));
      }
    }

    linearFitCoefficients_.resize(linearFitDegree_);
    angularFitCoefficients_.resize(angularFitDegree_);

    polynomialFit(linearFitDegree_, actualLinear_, expectedLinear_, linearFitCoefficients_);
    polynomialFit(angularFitDegree_, actualAngular_, expectedAngular_, angularFitCoefficients_);

    return true;
  }

  void SkidSteerDriveController::update(const ros::Time& time, const ros::Duration& period)
  {
    // COMPUTE AND PUBLISH ODOMETRY
    // Estimate linear and angular velocity using joint information
    odometry_.update(left_front_wheel_joint_.getPosition(), right_front_wheel_joint_.getPosition(), time, 1);

    // Publish odometry message
    if (last_state_publish_time_ + publish_period_ < time)
    {
      last_state_publish_time_ += publish_period_;
      // Compute and store orientation info
      const geometry_msgs::Quaternion orientation(
            tf::createQuaternionMsgFromYaw(odometry_.getHeading()));

      // Populate odom message and publish
      if (odom_pub_->trylock())
      {
        odom_pub_->msg_.header.stamp = time;
        odom_pub_->msg_.pose.pose.position.x = odometry_.getX();
        odom_pub_->msg_.pose.pose.position.y = odometry_.getY();
        odom_pub_->msg_.pose.pose.orientation = orientation;
        odom_pub_->msg_.twist.twist.linear.x  = odometry_.getLinearEstimated();
        odom_pub_->msg_.twist.twist.angular.z = odometry_.getAngularEstimated();
        odom_pub_->unlockAndPublish();
      }
    }

    // MOVE ROBOT
    // Retreive current velocity command and time step:
    Commands curr_cmd = *(command_.readFromRT());
    const double dt = (time - curr_cmd.stamp).toSec();

    // Brake if cmd_vel has timeout:
    if (dt > cmd_vel_timeout_)
    {
      curr_cmd.lin = 0.0;
      curr_cmd.ang = 0.0;
    }

    // Limit velocities and accelerations:
    double cmd_dt = period.toSec();
    limiter_lin_.limit(curr_cmd.lin, last_cmd_.lin, cmd_dt);
    limiter_ang_.limit(curr_cmd.ang, last_cmd_.ang, cmd_dt);
    last_cmd_ = curr_cmd;

    // Apply multipliers:
    double ws = wheel_separation_multiplier_ * wheel_separation_;
    const double wr = wheel_radius_multiplier_ * wheel_radius_;

    double newLinearVel = curr_cmd.lin;
    double newAngularVel = curr_cmd.ang;

    if (!sim_)
    {
      // Remap velocities according to the polynomial function
      remapVelocities(newLinearVel, newAngularVel);
    }

    // Compute wheels velocities:
    const double vel_left  = (newLinearVel - newAngularVel * ws / 2.0)/wr;
    const double vel_right = (newLinearVel + newAngularVel * ws / 2.0)/wr;

    // Set wheels velocities:
    left_front_wheel_joint_.setCommand(vel_left);
    right_front_wheel_joint_.setCommand(vel_right);
    left_rear_wheel_joint_.setCommand(vel_left);
    right_rear_wheel_joint_.setCommand(vel_right);
  }

  void SkidSteerDriveController::starting(const ros::Time& time)
  {
    brake();

    // Register starting time used to keep fixed rate
    last_state_publish_time_ = time;
  }

  void SkidSteerDriveController::stopping(const ros::Time& time)
  {
    brake();
  }

  void SkidSteerDriveController::brake()
  {
    const double vel = 0.0;
    left_front_wheel_joint_.setCommand(vel);
    right_front_wheel_joint_.setCommand(vel);
    left_rear_wheel_joint_.setCommand(vel);
    right_rear_wheel_joint_.setCommand(vel);
  }

  void SkidSteerDriveController::cmdVelCallback(const geometry_msgs::Twist& command)
  {
    if (isRunning())
    {
      command_struct_.ang   = command.angular.z;
      command_struct_.lin   = command.linear.x;
      command_struct_.stamp = ros::Time::now();
      command_.writeFromNonRT(command_struct_);
      ROS_DEBUG_STREAM_NAMED(name_,
                             "Added values to command. "
                             << "Ang: "   << command_struct_.ang << ", "
                             << "Lin: "   << command_struct_.lin << ", "
                             << "Stamp: " << command_struct_.stamp);
    }
    else
    {
      ROS_ERROR_NAMED(name_, "Can't accept new commands. Controller is not running.");
    }
  }

  bool SkidSteerDriveController::setOdomParamsFromUrdf(ros::NodeHandle& root_nh,
                             const std::string& left_front_wheel_name,
                             const std::string& right_front_wheel_name)
  {
    // Parse robot description
    const std::string model_param_name = "/robot_description";
    bool res = root_nh.hasParam(model_param_name);
    std::string robot_model_str="";
    if (!res || !root_nh.getParam(model_param_name, robot_model_str))
    {
      ROS_ERROR_NAMED(name_, "Robot descripion couldn't be retrieved from param server.");
      return false;
    }

    boost::shared_ptr<urdf::ModelInterface> model(urdf::parseURDF(robot_model_str));

    // Get wheel separation
    boost::shared_ptr<const urdf::Joint>
      left_front_wheel_joint(model->getJoint(left_front_wheel_name));
    if (!left_front_wheel_joint)
    {
      ROS_ERROR_STREAM_NAMED(name_, left_front_wheel_name
                             << " couldn't be retrieved from model description");
      return false;
    }
    boost::shared_ptr<const urdf::Joint> left_current_joint =
      left_front_wheel_joint;
    urdf::Vector3 left_position =
      left_front_wheel_joint->parent_to_joint_origin_transform.position;
    while (left_current_joint->parent_link_name != base_frame_id_)
    {
      boost::shared_ptr<const urdf::Joint>
        parent_joint(model->getLink(
          left_front_wheel_joint->parent_link_name)->parent_joint);
      if (!parent_joint)
      {
        ROS_ERROR_STREAM_NAMED(name_, left_front_wheel_joint->parent_link_name
                               << " couldn't be retrieved from model description");
        return false;
      }
      left_current_joint = parent_joint;
      left_position = left_position +
      left_current_joint->parent_to_joint_origin_transform.position;
    }

    boost::shared_ptr<const urdf::Joint> right_front_wheel_joint(model->getJoint(right_front_wheel_name));
    if (!right_front_wheel_joint)
    {
      ROS_ERROR_STREAM_NAMED(name_, right_front_wheel_name
                             << " couldn't be retrieved from model description");
      return false;
    }
    boost::shared_ptr<const urdf::Joint> right_current_joint =
      right_front_wheel_joint;
    urdf::Vector3 right_position =
      right_front_wheel_joint->parent_to_joint_origin_transform.position;
    while (right_current_joint->parent_link_name != base_frame_id_)
    {
      boost::shared_ptr<const urdf::Joint>
        parent_joint(model->getLink(
          right_front_wheel_joint->parent_link_name)->parent_joint);
      if (!parent_joint)
      {
        ROS_ERROR_STREAM_NAMED(name_, right_front_wheel_joint->parent_link_name
                               << " couldn't be retrieved from model description");
        return false;
      }
      right_current_joint = parent_joint;
      right_position = right_position +
      right_current_joint->parent_to_joint_origin_transform.position;
    }

    ROS_INFO_STREAM("left front wheel to origin: " << left_position.x << ","
                    << left_position.y << ", "
                    << left_position.z);
    ROS_INFO_STREAM("right front wheel to origin: " << right_position.x << ","
                    << right_position.y << ", "
                    << right_position.z);

    wheel_separation_ = euclideanOfVectors(left_position,
                                           right_position);

    // Get wheel radius
    if (!getWheelRadius(model->getLink(left_front_wheel_joint->child_link_name),
      wheel_radius_)
      && wheel_radius_ == -1)
    {
      ROS_ERROR_STREAM_NAMED(name_, "Couldn't retrieve " << left_front_wheel_name << " wheel radius");
      return false;
    }

    // Set wheel params for the odometry computation
    const double ws = wheel_separation_multiplier_ * wheel_separation_;
    const double wr = wheel_radius_multiplier_     * wheel_radius_;
    odometry_.setWheelParams(ws, wr);
    ROS_INFO_STREAM_NAMED(name_,
                          "Odometry params : wheel separation " << ws
                          << ", wheel radius " << wr);
    return true;
  }

  void SkidSteerDriveController::setOdomPubFields(ros::NodeHandle& root_nh, ros::NodeHandle& controller_nh)
  {
    // Get and check params for covariances
    XmlRpc::XmlRpcValue pose_cov_list;
    controller_nh.getParam("pose_covariance_diagonal", pose_cov_list);
    ROS_ASSERT(pose_cov_list.getType() == XmlRpc::XmlRpcValue::TypeArray);
    ROS_ASSERT(pose_cov_list.size() == 6);
    for (int i = 0; i < pose_cov_list.size(); ++i)
      ROS_ASSERT(pose_cov_list[i].getType() == XmlRpc::XmlRpcValue::TypeDouble);

    XmlRpc::XmlRpcValue twist_cov_list;
    controller_nh.getParam("twist_covariance_diagonal", twist_cov_list);
    ROS_ASSERT(twist_cov_list.getType() == XmlRpc::XmlRpcValue::TypeArray);
    ROS_ASSERT(twist_cov_list.size() == 6);
    for (int i = 0; i < twist_cov_list.size(); ++i)
      ROS_ASSERT(twist_cov_list[i].getType() == XmlRpc::XmlRpcValue::TypeDouble);

    // Setup odometry realtime publisher + odom message constant fields
    odom_pub_.reset(new realtime_tools::RealtimePublisher<nav_msgs::Odometry>(controller_nh, "/odom", 100));
    odom_pub_->msg_.header.frame_id = "odom";
    odom_pub_->msg_.child_frame_id = base_frame_id_;
    odom_pub_->msg_.pose.pose.position.z = 0;
    odom_pub_->msg_.pose.covariance = boost::assign::list_of
        (static_cast<double>(pose_cov_list[0])) (0)   (0)  (0)  (0)  (0)
        (0) (static_cast<double>(pose_cov_list[1]))  (0)  (0)  (0)  (0)
        (0)   (0)  (static_cast<double>(pose_cov_list[2])) (0)  (0)  (0)
        (0)   (0)   (0) (static_cast<double>(pose_cov_list[3])) (0)  (0)
        (0)   (0)   (0)  (0) (static_cast<double>(pose_cov_list[4])) (0)
        (0)   (0)   (0)  (0)  (0)  (static_cast<double>(pose_cov_list[5]));
    odom_pub_->msg_.twist.twist.linear.y  = 0;
    odom_pub_->msg_.twist.twist.linear.z  = 0;
    odom_pub_->msg_.twist.twist.angular.x = 0;
    odom_pub_->msg_.twist.twist.angular.y = 0;
    odom_pub_->msg_.twist.covariance = boost::assign::list_of
        (static_cast<double>(twist_cov_list[0])) (0)   (0)  (0)  (0)  (0)
        (0) (static_cast<double>(twist_cov_list[1]))  (0)  (0)  (0)  (0)
        (0)   (0)  (static_cast<double>(twist_cov_list[2])) (0)  (0)  (0)
        (0)   (0)   (0) (static_cast<double>(twist_cov_list[3])) (0)  (0)
        (0)   (0)   (0)  (0) (static_cast<double>(twist_cov_list[4])) (0)
        (0)   (0)   (0)  (0)  (0)  (static_cast<double>(twist_cov_list[5]));
    tf_odom_pub_.reset(new realtime_tools::RealtimePublisher<tf::tfMessage>(root_nh, "/tf", 100));
    tf_odom_pub_->msg_.transforms.resize(1);
    odom_frame_.transform.translation.z = 0.0;
    odom_frame_.child_frame_id = base_frame_id_;
    odom_frame_.header.frame_id = "odom";
  }

  void SkidSteerDriveController::remapVelocities(
      double& linear,
      double& angular)
  {
    double new_linear = 0;
    for (int i = 0; i <= linearFitDegree_; i++)
    {
      new_linear += linearFitCoefficients_[i] * pow(linear, i);
    }

    double new_angular = 0;
    for (int i = 0; i <= angularFitDegree_; i++)
    {
      new_angular += angularFitCoefficients_[i] * pow(angular, i);
    }

    linear = new_linear;
    angular = new_angular;
  }

  void SkidSteerDriveController::polynomialFit(
      const int& degree,
      const std::vector<double>& actualValues,
      const std::vector<double>& expectedValues,
      std::vector<double>& coefficients)
  {
    int obs = actualValues.size();

    gsl_multifit_linear_workspace *ws;
    gsl_matrix *cov, *X;
    gsl_vector *y, *c;
    double chisq;

    X = gsl_matrix_alloc(obs, degree);
    y = gsl_vector_alloc(obs);
    c = gsl_vector_alloc(degree);
    cov = gsl_matrix_alloc(degree, degree);

    for (int i = 0; i < obs; i++)
    {
      gsl_matrix_set(X, i, 0, 1.0);
      for (int j = 0; j < degree; j++)
      {
        gsl_matrix_set(X, i, j, pow(actualValues[i], j));
      }
      gsl_vector_set(y, i, expectedValues[i]);
    }

    ws = gsl_multifit_linear_alloc(obs, degree);
    gsl_multifit_linear(X, y, c, cov, &chisq, ws);

    for (int i = 0; i < degree; i++)
    {
      coefficients[i] = gsl_vector_get(c, i);
    }

    gsl_multifit_linear_free(ws);
    gsl_matrix_free(X);
    gsl_matrix_free(cov);
    gsl_vector_free(y);
    gsl_vector_free(c);
  }
}  // namespace motor
}  // namespace pandora_hardware_interface
