# LIN Communication Usage

This project introduces the LIN communication protocol and provides practical hardware and software design guidelines using the MSPM0C1104 microcontroller and the TLIN1039 LIN transceiver.  

# LIN Frame Structure

![LIN frame](https://ni.scene7.com/is/image/ni/LIN_frame_20090802104146?scl=1)

# Hardware Implementation

<img width="592" height="624" alt="image" src="https://github.com/user-attachments/assets/79f02f89-7344-48bb-88e8-a8f357bc6064" />

## Using the TLIN1039 Transceiver

The TLIN1039 provides all necessary physical-layer functionality:

* Merges UART TX/RX onto a single LIN bus pin
* Translates MCU logic to automotive LIN voltage levels
* Includes bus-stuck-low protection (prevents a node from holding LIN low)
* Implements ESD protection and slew-rate control for EMC compliance
* Provides short-circuit protection on the LIN pin
* Supports low-power modes and fault detection

# Demonstration
As part of demonstration, the LIN bus was selected as the communication protocol for collecting data from ground temperature sensors as part of final thesis "System for temperature control and security surveillance of turtle breeding". The choice was motivated primarily by the bus’s reliable range and simplicity. Since the temperature sensor will be in a small metallic tube, the use of a single data line is advantageous, minimizing wiring complexity and ensuring robust signal transmission.

![Tortoises-Temp-Sensor-LIN-PCB-3](https://github.com/user-attachments/assets/6d22d137-88e7-4538-9ec8-ff45da924118)

## Schematic description
MSPM0C1103 microcontroller was selected for its enhanced UART capabilities, which make LIN implementation easier. An analog temperature sensor was chosen for SW simplicity. For programming and debugging, a Tag‑Connect interface is implemented, providing a compact, solder‑free connection directly to the PCB. A key challenge in the design is that the SWD programming pins are multiplexed with other functional signals, which is an inherent trade‑off of the small package footprint. The hardware reset line is routed together with the SWD interface. For circuit protection, ESD and Schottky diodes were used.

[Tortoises-Temp-Sensor-LIN.pdf](https://github.com/user-attachments/files/23860681/Tortoises-Temp-Sensor-LIN.pdf)

## Software project description

# References

* The ground temperature LIN sensor is part of the final thesis: MUSIL, Pavel. *System for temperature control and security surveillance of turtle breeding.* Online, semestral Thesis. Roman ŠOTNER (supervisor). Brno: Brno University of Technology, Faculty of Electrical Engineering and Communication, 2026. Available at: https://www.vut.cz/en/students/final-thesis/detail/170513. [accessed 2025-12-02].
* The development of the ground temperature LIN sensor was consulted with engineers from egmenergo, in particular with Ing. Ivo Strašil, who is a technical consultant for the thesis.
* MSPM0C1104 Technical Reference Manual (SLAU893C)
* TLIN1039 LIN Transceiver Datasheet
* LIN 2.0/2.1/2.2A Protocol Specifications
