# Real-Time Displacement Tracker

## Overview 

This project involves the design of a device used to track the displacement of a  moving building under seismic activity. It was developed as part of the final project for the Multivariable Control Systems course in the MSc program in Control Systems and Robotics. The primary goal of the device is to monitor and provide real-time state feedback for controllers applied to seismically controlled buildings.

## The Idea

Consider a building equipped with static control mechanisms, such as elastic dampers installed at its base. These dampers mitigate high-frequency vibrations caused by seismic activity and help protect the building. However, not all seismic waves are fully absorbed—residual vibrations can still propagate and cause structural damage.

The concept proposed in the paper (**Kelly, Soldatos**) suggests that, while preserving the benefits of elastic damping, we can introduce a dynamic control system to compensate for the remaining movement. This project focuses on designing the sensing component that would support such a control system by providing accurate, real-time motion data.

## Hardware

- MPU6050 IMU – Inertial measurement unit for acceleration data retreival.
- FireBeetle ESP32-C6 Mini – Microcontroller for data acquisition, procesing and communication
- 3D-Printed Casing – Protective housing for sensor and electronics
- USB Cable – For power and serial communication

## Software 

- ESP-IDF – Software development and hardware configuration framework for ESP32-based SoC's 
- Embedded source code – Firmware for data acquisition and processing as provided in this repository
- Python scripts – For data visualization and post-processing

## Configuration

The configuration consists of at least two sensors, **A** and **B**, positioned as follows:

* **Sensor A** is mounted at the base of the elastic dampers (i.e., on the ground).
* **Sensor B** is mounted at the base of the building, directly on top of the dampers.

Both sensors continuously monitor acceleration. At a given timestamp $t_i$:

* Sensor A records acceleration $a_A(t_i)$
* Sensor B records acceleration $a_B(t_i)$

This data is transmitted to a central processing unit, where the relative acceleration is computed as:

$$
    a_{\text{diff}}(t_i) = a_A(t_i) - a_B(t_i)
$$

This differential acceleration reflects the movement of the building relative to the ground. It can be used to infer structural displacement and to provide real-time feedback for control algorithms.