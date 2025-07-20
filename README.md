# Real-Time Displacement Tracker

## Overview 

This project involves the design of a device used to track the displacement of a  moving building under seismic activity. It was developed as part of the final project for the Multivariable Control Systems course in the MSc program in Control Systems and Robotics. The primary goal of the device is to monitor and provide real-time state feedback for controllers applied to seismically controlled buildings.

## The Idea

Consider a building equipped with static control mechanisms, such as elastic dampers installed at its base. These dampers mitigate high-frequency vibrations caused by seismic activity and help protect the building. However, not all seismic waves are fully absorbed—residual vibrations can still propagate and cause structural damage.

The concept proposed in the paper *Robust control of base-isolated structures: Theory and experiments* (-*Kelly, Leitmann and Soldatos*), suggests that that a robust controller applied at the base of an elastomeric-isolated structure can effectively suppress superstructure motion. Their analysis provides critical design constraints for the sensor unit, including the need for high-fidelity base displacement and velocity measurements, low latency, and sufficient dynamic range to capture large isolator displacements and velocities. This project focuses on designing the sensing component that would support such a control system by providing accurate, real-time motion data.

## Hardware

* MPU6050 IMU – Inertial Measurement Unit for retrieving acceleration and gyroscope data.
* FireBeetle ESP32-C6 Mini – Microcontroller used for data acquisition, processing, and communication.
* 3D-Printed Casing – Protective enclosure for the sensor and electronic components.
* USB Cable – Used for both power supply and serial communication with a host computer.

The MPU6050 IMU communicates with the ESP32-C6 Mini microcontroller via the I2C protocol. Use the following wiring configuration to connect the sensor to the microcontroller.

<div align="center">

<table>
  <thead>
    <tr>
      <th><strong>MPU6050 Pin</strong></th>
      <th><strong>ESP32-C6 Mini Pin</strong></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td align="center">SCL</td>
      <td align="center">IO19</td>
    </tr>
    <tr>
      <td align="center">SDA</td>
      <td align="center">IO20</td>
    </tr>
    <tr>
      <td align="center">VCC</td>
      <td align="center">3V3</td>
    </tr>
    <tr>
      <td align="center">GND</td>
      <td align="center">GND</td>
    </tr>
  </tbody>
</table>

</div>

The ESP32-C6 Mini connects to a host computer over USB for data transfer and power. To secure the electronic components, use the provided 3D model casing (`case.stl`). **Caution**: Ensure that the sensor is oriented correctly within the case—align the axis arrows on the top of the casing with the direction arrows on the sensor's PCB.

## Software 

- ESP-IDF – Software development and hardware configuration framework for ESP32-based SoC's.
- Embedded source code – Firmware for data acquisition and processing as provided in this repository.
- Python scripts – For data visualization and post-processing.

To install the ESP-IDF software please follow the guide provided by the official [webpage](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html). Once the ESP-IDF tools are installed successfully, make a new project by adding this repository.  

## Programming the Device 

To program the ESP32-C6 device with the provided source code, use the ESP-IDF tools:

```shell
cp -r RTimu_displacement_tracker/esp32_rtdt_app ~/esp
cd ~/esp
. ./esp-idf/export.sh
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyACM0 flash
```

## Build the UI

To build the executable application from the provided Python script, run the following commands on a Linux terminal:

```shell
cd disp_monitor_ui
chmod +x build.sh
./build.sh
```

For windows users use the `build.bat` script simillarly. 
