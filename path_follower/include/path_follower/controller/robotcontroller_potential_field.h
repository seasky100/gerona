#ifndef ROBOTCONTROLLER_POTENTIAL_FIELD_H
#define ROBOTCONTROLLER_POTENTIAL_FIELD_H

/// THIRD PARTY
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <geometry_msgs/PoseStamped.h>

/// SYSTEM
#include <pcl_ros/point_cloud.h>
#include <Eigen/Core>
#include <Eigen/Dense>


/// PROJECT
#include <path_follower/controller/robotcontroller_interpolation.h>
#include <path_follower/utils/parameters.h>


/// The Potential_Field class
class RobotController_Potential_Field : public RobotController_Interpolation
{
public:
    RobotController_Potential_Field();

    //! Immediately stop any motion.
    virtual void stopMotion();

    /// Sets the speed of the robot.
    static void setRobotSpeed(double speed);

protected:

    /// Sets the goal position.
    virtual void setGoalPosition();

    virtual MoveCommandStatus computeMoveCommand(MoveCommand* cmd);
    virtual void publishMoveCommand(const MoveCommand &cmd) const;
    virtual void initialize();

    void calculateMovingDirection();
    void initializeMarkers();
    void visualizeMarkers();

    // x and y component of the attractive force vector
    double FAttX, FAttY;
    // x and y component of the resulting force vector
    double FResX, FResY;
    // nominal robot velocity
    double vn_;
    // index of the orthogonal projection
    unsigned int proj_ind_;
    // error coordinates
    double xe_, ye_, theta_e_;

    // nearest obstacle for each segment, split in x and y component
    double obstacles[2];
    // repulsive force for each segment, split in x and y component
    Eigen::Vector2d FRep;

    // update the current potential field
    void update(double newFAttX, double newFAttY);
    // dind the nearest obstacle for each segment
    void findObstacles();
    // compute the repulsive forces
    void computeFReps();
    // compute the resulting force acting on the robot
    void computeFRes();

    double mGoalPosX;
    double mGoalPosY;

    // some markers for visualizing the force vectors
    // for FAtt
    visualization_msgs::Marker marker;
    // for FReps
    visualization_msgs::Marker FrepMarker;
    // for FRes
    visualization_msgs::Marker FResMarker;
    // MarkerArray for publishing
    visualization_msgs::MarkerArray FMarkers;
    // marker publisher
    ros::Publisher F_pub;



    struct ControllerParameters : public RobotController_Interpolation::InterpolationParameters
    {
        P<double> kAtt;
        P<double> kRep;
        P<double> max_angular_velocity;

        ControllerParameters():
            kAtt(this, "~kAtt", 0.2, ""),
            kRep(this, "~kRep", 0.5, ""),
            max_angular_velocity(this, "~max_angular_velocity", 0.8, "")
        {}
    } opt_;

    const RobotController_Interpolation::InterpolationParameters& getParameters() const
    {
        return opt_;
    }

    struct Command
    {
        RobotController_Potential_Field *parent_;

        //! Speed of the movement.
        float speed;
        //! Direction of movement as angle to the current robot orientation.
        float direction_angle;
        //! rotational velocity.
        float rotation;


        // initialize all values to zero
        Command(RobotController_Potential_Field *parent):
            parent_(parent),
            speed(0.0f), direction_angle(0.0f), rotation(0.0f)
        {}

        operator MoveCommand()
        {
            MoveCommand mcmd(true);
            mcmd.setDirection(direction_angle);
            mcmd.setVelocity(speed);
            mcmd.setRotationalVelocity(rotation);
            return mcmd;
        }

        bool isValid()
        {
            if ( isnan(speed) || isinf(speed)
                 || isnan(direction_angle) || isinf(direction_angle)
                 || isnan(rotation) || isinf(rotation) )
            {
                ROS_FATAL("Non-numerical values in command: %d,%d,%d,%d,%d,%d",
                          isnan(speed), isinf(speed),
                          isnan(direction_angle), isinf(direction_angle),
                          isnan(rotation), isinf(rotation));
                // fix this instantly, to avoid further problems.
                speed = 0.0;
                direction_angle = 0.0;
                rotation = 0.0;

                return false;
            } else {
                return true;
            }
        }
    };

    Command cmd_;

    void reset();
    void setPath(Path::Ptr path);


};
#endif //ROBOTCONTROLLER_POTENTIAL_FIELD_H
