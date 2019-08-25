[![Showcase Video](/Deliverables/ThumbnailVideo.jpg)](https://www.youtube.com/watch?v=AWg9zco_IYE)
# CareBot
This repository contains all the code used in this project. We used a BrickPi programmed in C++ to be a smart assistant for use in hospitals. It has Computer Vision to avoid obstacles (feet, table legs, walls, etc.) using openCV. It also has a remote based on a ESP32 with a TFT-screen and joystick. The joystick is used to control the robot, turn a 433mhz connected light on/off and select operation mode which can be one of the following.
- Follow the red dot (which can be tied to a leg including obstacle avoidance)
- Follow the line (including calibration and obstacle avoidance)
- Manual control using the remote
- Automatic navigation 

The robot also has a microphone and speaker connected to it. These are used for the Google Assistant library. This makes the following possible.
- "Hey google, drive towards me"
- "Hey google, follow the line"
- "Hey google, turn the light on/off"
- "Hey google, start manual control"
