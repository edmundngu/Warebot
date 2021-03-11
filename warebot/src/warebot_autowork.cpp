/** WAREBOT V1.0

AUTHOR : EDMUND NGU JAN PIEW

**/

// Set C/C++ library
#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Int32MultiArray.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <opencv2/opencv.hpp>
#include <termios.h>

//define Task ID
#define RECEIPT 1
#define TAKE_ORDER 2
#define PLACE_ORDER 3

using namespace std;

std::string path_to_sounds;

// Fucntion declaration
bool moveToGoal(double goalXcoor, double goalYcoor);
void objectCallback();
int getch();

/** Declare coordinates of warehouse */
//WAREBOT BASE
double init_xcoor = 3.48; 
double init_ycoor = 5.27;

// RECEIPT
double warehouse1_xcoor = 6.27;
double warehouse1_ycoor = 0.679;

// RACK_LOAD
double warehouse2_xcoor = -2.62;
double warehouse2_ycoor = -3.68;


// UNLOAD BASE
double warehouse3_xcoor = 6.44;
double warehouse3_ycoor = -4.65;

/*********************************/

// Variable initialize
bool goalReached = false;
int id=0;
int initmove=0;
char mission;
double duration = 10;
bool valid = false;
ros::Publisher action_pub;
geometry_msgs::Twist set_vel;
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void objectCallback(const std_msgs::Float32MultiArrayPtr &object)
{
  if(initmove==0){
    ROS_INFO("Press s to start: ");

    while(!valid){
      mission = getch();
      if(mission == 's' ){
	valid = true;
      }
      else{
	ROS_INFO("Invalid mission! Please try again !");
      }
    }

    if(mission == 's'){
      goalReached = moveToGoal(warehouse1_xcoor, warehouse1_ycoor);
    }
    initmove++;
  }

  if (object->data.size() > 0)
  {
    id = object->data[0];
   
    switch(id)
    {
      case RECEIPT:
        ROS_INFO("GOODS RECEIPT Received! Navigate to collect list item! (Task ID : 1)");
        goalReached = moveToGoal(warehouse2_xcoor, warehouse2_ycoor);
        set_vel.angular.z = -3.142*2/4/duration;
        action_pub.publish(set_vel);
        ros::Duration(duration).sleep();

	if(!goalReached){
	  ROS_INFO("Fail to receive goods receipt!");
        }

	break;

      case TAKE_ORDER:
	ROS_INFO("Pickup the item! Navigate to unloading location! (Task ID : 2)");
        goalReached = moveToGoal(warehouse3_xcoor, warehouse3_ycoor);

	if(!goalReached){
	  ROS_INFO("Fail to pickup or load items!");
        }
        break;

      case PLACE_ORDER:
	ROS_INFO("Unload Items! Jom,Next Receipt! (Task ID : 3)");
	//goalReached = moveToGoal(init_xcoor, init_ycoor);
        goalReached = moveToGoal(warehouse1_xcoor, warehouse1_ycoor);
        //initmove = 0;

        if(!goalReached){
          ROS_INFO("Fail to unload items!");
        }
        break;

      default:
	ROS_INFO("ERROR: Invalid Task ID!\nPlease try again!");
    }
  }

  else{
    ROS_INFO("No Task ID Detected!\nPlease try again!");
  }
}

int main(int argc, char** argv){
  ros::init(argc, argv, "map_navigation_node");
  ros::NodeHandle n;
  ros::Rate loop_rate(50);
  ros::Subscriber sub = n.subscribe("/objects", 1, objectCallback);
  action_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 1);

  while(ros::ok())
  { 
    ROS_INFO("Scan my Task ID");
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}

bool moveToGoal(double goalXcoor, double goalYcoor){
  
  //define a client for to send goal requests to the move_base server through a SimpleActionClient
  //actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> ac("move_base", true);
  MoveBaseClient ac("move_base", true);

  //wait for the action server to come up
  while(!ac.waitForServer(ros::Duration(5.0))){
    ROS_INFO("Waiting for the move_base action server to come up");
  }

  move_base_msgs::MoveBaseGoal goal;

  //set up the frame parameters
  goal.target_pose.header.frame_id = "map";
  //goal.target_pose.header.frame_id = "base_link";
  goal.target_pose.header.stamp = ros::Time::now();

  /** move 180 degree (turn around) */
  set_vel.linear.x = 0;
  set_vel.linear.y = 0;
  set_vel.linear.z = 0;
  set_vel.angular.z = 3.142*2/4/duration;
  action_pub.publish(set_vel);
  
  ros::Duration(duration).sleep();

  set_vel.angular.z = 0;
  action_pub.publish(set_vel);

  /* moving towards the goal*/
  goal.target_pose.pose.position.x =  goalXcoor;
  goal.target_pose.pose.position.y =  goalYcoor;
  goal.target_pose.pose.position.z =  0.0;
  goal.target_pose.pose.orientation.x = 0.0;
  goal.target_pose.pose.orientation.y = 0.0;
  goal.target_pose.pose.orientation.z = 0.0;
  goal.target_pose.pose.orientation.w = 1.0;

  ROS_INFO("Navigating to target location ...");
  ac.sendGoal(goal);

  ac.waitForResult();

  if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
     ROS_INFO("Arrived at target location!");
     return true;
  }
  else{
     ROS_INFO("Fail to arrive at target location!");
     //goalReached = moveToGoal(init_xcoor, init_ycoor);
     return false;
  }
}

int getch(){	// get the input from terminal
  static struct termios oldt, newt;
  tcgetattr( STDIN_FILENO, &oldt);           // save old settings
  newt = oldt;
  newt.c_lflag &= ~(ICANON);                 // disable buffering      
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);  // apply new settings

  int c = getchar();  // read character (non-blocking)

  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);  // restore old settings
  return c;
}


