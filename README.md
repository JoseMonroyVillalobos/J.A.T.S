##Robot Tracking and Control System

-This repository contains the code and documentation for a robot tracking and control system. The project uses Ultra-Wideband (UWB) technology for precise localization and communication protocols like ESPNOW and Bluetooth to control the robot’s movement.

##Project Overview

#Objective

-The robot is designed to continuously track a tag the user carries and adjust its position to ensure it always faces the tag and follows user. Additionally, an alternative control mode allows for direct manual operation of the robot using a Bluetooth-enabled application.

##Features

-Ultra-Wideband Tracking: Uses three anchors on the robot to calculate the distance to a tag for real-time trilateration.

-Autonomous Movement: Calculates and executes the required movement to keep the robot oriented towards the tag.

-ESPNOW Communication: Sends movement instructions from the main ESP32 microcontroller to the drivetrain ESP32 microcontroller.

-Bluetooth Control: An alternate version using the Dabble library enables manual control of the robot via Bluetooth.

System Components

Hardware

Anchors: Three UWB modules positioned on the robot.

Tag: A UWB module carried by the user.

Microcontrollers: ESP32 boards for localization, processing, and drivetrain control.

Drivetrain: Motors and motor drivers controlled by the drivetrain ESP32 microcontroller.

Software

Localization Algorithm: Implements trilateration to calculate the tag's position relative to the robot.

Movement Algorithm: Determines the robot's movement to maintain alignment with the tag.

ESPNOW Communication: Facilitates wireless communication between the main and drivetrain microcontrollers.

Bluetooth Control: Uses the Dabble library for direct manual control of the drivetrain ESP32.

Project Versions

AutonomousESPNOWVersion

This version focuses on autonomous operation:

Continuously calculates the tag's position using distances from the three anchors.

Sends movement instructions via ESPNOW to the drivetrain ESP32.

Suitable for autonomous tracking scenarios.

Bluetooth Control

This version allows manual operation:

Uses the Dabble library for Bluetooth communication.

Sends movement commands directly to the drivetrain ESP32 from a mobile device.

Ideal for scenarios requiring direct manual control.

Why Separate Versions?

Due to program space limitations on the drivetrain ESP32, the ESPNOW-based tracking system and the Bluetooth control system are maintained as separate versions.

Getting Started

Prerequisites

ESP32 microcontrollers.

ESP32 UWB modules from makerfabs

Motor driver boards TB6612FNG and motors for the drivetrain.

Dabble app for Bluetooth control (optional).

Installation

Install the necessary libraries:

DW3000

DabbleESP32 (for Bluetooth version).

Upload the respective code to the ESP32 microcontrollers.


Usage

UWB Tracking

Power on the robot and the tag.

The robot will automatically calculate the tag's position and adjust its orientation.

Bluetooth Control

Pair your mobile device with the drivetrain ESP32 via Bluetooth.

Open the Dabble app and use the control interface to operate the robot manually.

Contributing

Contributions are welcome! Please create a pull request or open an issue to suggest changes or report bugs.

License

This project is licensed under the MIT License. 

Acknowledgments

Special thanks to the developers of the Dabble library and ESPNOW communication protocol for enabling this project.

Feel free to reach out with questions or suggestions by opening an issue on this repository.

