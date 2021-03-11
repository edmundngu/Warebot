cmake_minimum_required(VERSION 3.0.2)
project(warebot)

  add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  actionlib
  actionlib_msgs
  message_generation
  roscpp
  rospy
  std_msgs
)

find_package( OpenCV REQUIRED ) 

catkin_metapackage()

 generate_messages(
   DEPENDENCIES
   std_msgs  # Or other packages containing msgs
 )

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES tutorial_pkg
#  CATKIN_DEPENDS roscpp
#  DEPENDS system_lib
  CATKIN_DEPENDS
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_executable(warebot_autowork src/warebot_autowork.cpp)
add_dependencies(warebot_autowork warebot_generate_messages_cpp)
target_link_libraries(warebot_autowork ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

