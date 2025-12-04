---

# LIN Communication Usage

This project introduces the LIN communication protocol and provides practical hardware and software design guidelines using the MSPM0C1104 microcontroller and the TLIN1039 LIN transceiver.  
The purpose is not to develop a device strictly compliant with the LIN specification, but rather to demonstrate how LIN communication can be applied flexibly in closed systems without following the exact LIN specification.  
This approach offers the advantage of tailoring the communication exactly to the needs of the application — enabling custom message formats, timing, and system behavior — while still benefiting from robust hardware support such as TI’s extended UART features and the TLIN1039 transceiver.

---

# About the LIN Protocol

LIN (Local Interconnect Network) is an asynchronous, single-wire serial communication bus commonly used in automotive applications for non-safety-critical peripherals. It is a low-cost solution optimized for simple device networks and mass production.

A LIN bus consists of:

* **VBAT** – supply voltage
* **GND** – ground reference
* **LIN** – single-wire communication line

![Schematic](https://github.com/Musislik/lin-ground-temp-sensor/blob/main/imgs/LIN_bus.svg)

---

# LIN Frame Structure

![LIN frame](https://ni.scene7.com/is/image/ni/LIN_frame_20090802104146?scl=1)

A LIN frame consists of the following fields:

### **Break**

Marks the beginning of a new LIN frame. The commander drives the line low for **at least 13 bit periods**, ensuring all responders detect the start of communication.

### **Sync (0x55)**

The commander transmits the byte `0x55`. The alternating `01010101` pattern enables responders to measure the duration of high and low intervals, effectively **calibrating their baud-rate measurement** before interpreting the PID field.

### **ID (PID) Field**

The Protected Identifier consists of 8 bits:

* **Bits 0–6**: 7-bit message identifier
* **Bit 7**: parity bit

The PID defines the response structure and whether a responder should answer.

### **Data Field**

Depending on the PID definition, a LIN message may require:
* the commander to send up to **8 data bytes** (with no responder reply), or
* the responder to send up to **8 data bytes**, or
* no response at all.


### **Checksum**

Ensures data integrity.

Two variants exist:

* **Classic checksum**: covers only the data bytes
* **Enhanced checksum**: covers PID + data bytes (LIN 2.0+)

Example of checksum implementation:
```
checksum = 0xFF - ((sum of covered bytes) mod 256)
```

If the responder responds, it generates the checksum.

---

# Hardware Implementation

<img width="592" height="624" alt="image" src="https://github.com/user-attachments/assets/79f02f89-7344-48bb-88e8-a8f357bc6064" />

### Pull-Up Requirements

* The **commander** implements the dominant pull-up resistor: **1 kΩ** to VBAT through the LIN transceiver.
* **Responders** only include a weak pull-up of approximately **47 kΩ**, provided internally by TLIN1039.

### Using the TLIN1039 Transceiver

The TLIN1039 provides all necessary physical-layer functionality:

* Merges UART TX/RX onto a single LIN bus pin
* Translates MCU logic to automotive LIN voltage levels
* Includes bus-stuck-low protection (prevents a node from holding LIN low)
* Implements ESD protection and slew-rate control for EMC compliance
* Provides short-circuit protection on the LIN pin
* Supports low-power modes and fault detection

---

# LIN Commander Guide

The commander (master) initiates all communication on the bus.

### Sending the BREAK Field

The MSPM0C1104 LIN UART extension allows BREAK generation via `BRK` bit in `UARTx.LCRH`.

```c
// Generate BREAK condition
UARTx.LCRH |= (1 << BRK);
```

### Sending Sync, PID, and Data

After BREAK:

1. Write `0x55` (SYNC) to UART TX FIFO.
2. Write PID (1 parity bit computed from 7-bit ID).
3. (Optional) Write 0–8 data bytes.
4. (Optional) Write the computed checksum.

All these bytes are transmitted using the standard UART framing mechanism, without special handling.

### Checksum
Could be computed as:
```
checksum = 0xFF - ((sum of applicable bytes) mod 256)
```

---

# LIN Responder Guide

Responders never initiate frames.

### Break Detection

The MSPM0C1104 LIN UART module detects break conditions using hardware logic. When the line is held in the dominant low state beyond the duration of 13 bits, it triggers a break interrupt, signaling frame start. The duration is verified using the timer that measures the dominant state interval.

### Baud-Rate Synchronization Using SYNC

The extended hardware support of the MSPM0C110 includes two measurement timers:

* One measures **dominant** bit durations
* One measures **recessive** bit durations

During reception of `0x55` (b01010101), software uses these measurements to compute the bit period and update the UART baud rate.

### PID Handling Using a PID Table

Responders typically implement a PID table containing:

* Supported PIDs
* Expected data lengths
* A callback function

### Receiving or Transmitting Data

Depending on the PID definition, the responder:

* Processes received data bytes, or
* Transmits 0–8 data bytes and the checksum

### Checksum Handling

If the responder transmits data, it computes the required checksum.

---

# Demonstration
As part of demonstration, the LIN bus was selected as the communication protocol for collecting data from ground temperature sensors as part of final thesis "System for temperature control and security surveillance of turtle breeding". The choice was motivated primarily by the bus’s reliable range and simplicity. Since the temperature sensor will be in a small metallic tube, the use of a single data line is advantageous, minimizing wiring complexity and ensuring robust signal transmission.

## Schematic description
MSPM0C1103 microcontroller was selected for its enhanced UART capabilities, which make LIN implementation easier. An analog temperature sensor was chosen for SW simplicity. For programming and debugging, a Tag‑Connect interface is implemented, providing a compact, solder‑free connection directly to the PCB. A key challenge in the design is that the SWD programming pins are multiplexed with other functional signals, which is an inherent trade‑off of the small package footprint. The hardware reset line is routed together with the SWD interface. For circuit protection, ESD and Schottky diodes were used.

[Tortoises-Temp-Sensor-LIN.pdf](https://github.com/user-attachments/files/23860681/Tortoises-Temp-Sensor-LIN.pdf)

## Software Development
Due to a delay in the development of the custom board, it was not possible to develop and deploy the software directly on the target hardware within the project timeframe. As a workaround, the required functionality was tested and validated on development boards that had been purchased prior to the start of the project. This approach allowed us to verify data harvesting, ensuring that the software could later be fitted to the final hardware without major modifications.

## Hardware Setup 
The experimental setup consisted of two LP‑MSPM0C1104 development boards connected via UART. To emulate the behavior of the temperature sensor, a resistive divider was used as a simple analog source. This configuration provided a practical environment for testing the communication protocol before integrating the actual sensor hardware.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/imgs/1764706711210.jpg" />

## Software project description
LIN responder mainly polls PIDs within the LIN bus to obtain data from back from them. This conception is closest to final application in semestral thesis where data is polled from temperature sensors. 

### LIN commander
It has by default defined LIN enable signal to high to enable LIN transceiver. LIN communication by itself is triggered by button (GPIO) as interrupt where flag for sending message and receive data are raised.In while loop first PID from message table is sent then receive data flag is raised down, now commander is waiting for data to be received from responder. The reception is hadled as UART interrupt where this interrupt lasts for 1 byte of received data thus it needs to be counted because number of bytes communication is set 3 bytes (Checksum included). When complete packet is received the receive flag is raised and data is written into Rx buffer.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/imgs/received_data.png" />
In the picture above you can see received data written to rx buffer (debugger)

Commander then moves to poll another PID until number of needed polls is reached. Commander then waits for another communication trigger by button.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/imgs/polling_multiple_PIDs.png" />
In the picture above you can see polling PIDs that contain same value that is received.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/diagrams/diag_main_loop-commander_main.drawio.svg" />
In the picture above you can see main while loop of Commander where polling occurs.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/diagrams/diag_main_loop-incoming_data_interp.drawio.svg" />
In the picture above you can see how incomming data interrupt is handled.

Commander features simple callback where data received from responder is compared against some arbitrary threshold value.
#### Interpreting received data as LIN
Receiving data from UART is not enough, checksum at the commander side needs to be computed and compared with received one. The receive message function is modeled as a sort of state machine where first state, data is put to data and checksum buffers respectively. When required data length has been receiver state gets flipped to cheksum, there a proper checksum is calculated and callback is triggered. By default case LIN state stays in LIN_DATA.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/diagrams/diag_main_loop-recv_msg.drawio.svg" />

#### Transmiting data in LIN protocol
First break signal is sent by enabling break bit in LCHR register this stays for 1 ms then break bit is cleared. Then Sync and PID frame is transmitted directly by UART transmit functions. If there is callback present in message table i.e. commander does not transmit data, then checksum is not calculated. 

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/diagrams/diag_main_loop-transm_msg.drawio.svg" />

## LIN responder
Reception of LIN protocol to responder also works on basis of state machine. Data reception is handled via UART interrupt. When it gets to PID a function setLINResponderRXMessage() is called where PID is checked for its validity and then from switch for valid PIDs it is decided how to deal with received message. For our case data is sampled from AD converters and sent back.

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/diagrams/diag_main_loop-fix-Responder.drawio.svg" />

Data is either transmitted back or just received as seen on two pictures below

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/imgs/LIN_mod_program_request_na_8B.png" />

<img width="592" height="624" alt="image" src="https://github.com/Musislik/lin-ground-temp-sensor/blob/main/imgs/LIN_Texas_example_PID_Send_No_Response.png" />

---
## Demonstration Videos
[Demo 1](https://youtu.be/wHVMJd_wKBM), [Demo 2](https://youtu.be/HMDH7teU0-Q)

---

# References

* The ground temperature LIN sensor is part of the final thesis: MUSIL, Pavel. *System for temperature control and security surveillance of turtle breeding.* Online, semestral Thesis. Roman ŠOTNER (supervisor). Brno: Brno University of Technology, Faculty of Electrical Engineering and Communication, 2026. Available at: https://www.vut.cz/en/students/final-thesis/detail/170513. [accessed 2025-12-02].
* The development of the ground temperature LIN sensor was consulted with engineers from egmenergo, in particular with Ing. Ivo Strašil, who is a technical consultant for the thesis.
* [MSPM0C1104 Technical Reference Manual (SLAU893C)](https://www.ti.com/lit/ug/slau893c/slau893c.pdf?ts=1764767323160) (pg785)
* TLIN1039 LIN Transceiver Datasheet
* LIN 2.0/2.1/2.2A Protocol Specifications
* TI Sysconfig
* OpenAI. ChatGPT (GPT-5) was used for formulating phrases.

---
