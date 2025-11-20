# LIN

# TODO
- MD - Program komunikace
- PM - **Destička**, Šéfis
- OO - Rešerš ovládání LIN![Uploading image.png…]()


# About this project
- This project demostrates local interconnect network (LIN) communication
## About LIN Protocol
Local interconnect network or (LIN) is a type of asynchronous communication between devices. It is extesively used in automotive industry for communication between non-critical peripherals. It is a low-cost solution for serial communcation in mass production [[1](https://en.wikipedia.org/wiki/Local_Interconnect_Network)].

![LIN bus](https://canlogger1000.csselectronics.com/img/lin-bus-single-wire-termination-supply-ground.svg)

LIN bus is comprised of supply voltage VBAT, ground GND and LIN signal wire.

## LIN frame
![LIN frame](https://ni.scene7.com/is/image/ni/LIN_frame_20090802104146?scl=1)


- **Break sequence**: Serves as start notice for all nodes in the bus. When BREAK field begins the LIN line is pulled low for time period of 13 bits from commander's side to give sufficient time for responders to notice.
- **Sync sequence**: Commander will send character `0x55` into the bus, so that all responders will synchroze and set their baurate according to it
- **ID sequence**: In this sequence of 8 bits, commander will provide task (recieve or send) and address of responder in a bus. First 6 bits containing responder address is sent, then 2 bits of parity is determines following task to responder: "01"... ignore data that will be sent, "10"... listen for data, "11"... send data.
- **Data**: 8 bits of data is sent/recieved
- **Checksum sequence**: It is transmitted as a last field frame to verify that transmission was successful

- In used microcontroller MSPM0C1104 it is featured as an extension for UART module.
- See [Technical Reference Manual](https://www.ti.com/lit/ug/slau893c/slau893c.pdf?ts=1762344977058&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSPM0C1104) of MSPM0C1104 pages 784 through 806 for UART/LIN extesion module.

# Hardware implementation of LIN protocol
<img width="592" height="624" alt="image" src="https://github.com/user-attachments/assets/79f02f89-7344-48bb-88e8-a8f357bc6064" />

Differences between commander and responder implementation [[2]](https://www.ti.com/lit/ds/symlink/tlin1039-q1.pdf?ts=1763575962774).

## LIN transmission 
- @ Page 791
- LIN counter is clocked by UART clock
- Can be interrupted upon LINCNT counter overflow and is flagged as CPU_INT.IMASK.LINOVF

### Sending BREAK signal
- @ Page 791
- BREAK signal is set by BRK bit in UARTx.LCRH register.
- It is important to set BREAK signal before data is loaded into TXDATA or FIFO register
  ```c
   //Control BREAK 
   UARTx.LCRH = UARTx.LCRH | (1<<BRK );
   //Code continues with loading data
   //...
   ```

### Generate LIN responder signals
- @ Page 786
- To be able to software control generation of LIN responder signals, the TXD_OUT and
TXD_CTL_EN bit in register UARTx.CTL0 needs to be configured
- If TXD_CTL_EN = '1' then output pin can be controlled by the TXD_OUT bit if the UART transmit is disabled (CTL0.TXE is cleared).

   ```c
   //Enable LIN responder signals generation
   UARTx.CTL0 = UARTx.CTL0 | (1<<TXD_CTL_EN );
   ```
## LIN recieve
- @ Page 792
- To detect BREAK and SYNC thus enabling reception, these features need to be software configured:

#### A) BREAK Detection

1. Initialize LIN counter to 0 `(UARTx.LINCNT = 0)`
2. Enable counter compare match mode `(UARTx.LINCTL.LINC0_MATCH = 1)`
3. Load `UARTx.LINC0` (counter capture 0 register) with counter value corresponding to $9.5 \cdot T_{bit}$
4. Enable LINC0 match interrupt `(CPU_INT.IMASK.LINC0 = 1)`
5. Setup LIN count control `(UARTx.LINCTL)`:
- Enable count while low signal on RXD `(LINCTL.CNTRXLOW = 1)`
- Enable LIN counter clearing on RXD falling edge `(LINCTL.ZERONE = 1)`
- Enable LIN counter `(LINCTL.CTRENA = 1)`
- Optionally a timeout can be added if BREAK field fails to pull itself high withing specific time (see page 792)

#### B) SYNC Detection & edge detection

- The following flow describes a possible LIN sync field validation procedure:
1. Initialize LIN counter to 0 `(UARTx.LINCNT = 0)` after detecting a valid break field.
2. Enable interrupt on RX falling edge `(CPU_INT.IMASK.RXNE = 1)`
3. Setup LIN count control `(LINCTL)`:
• Enable LIN counter capture on raising RX edge `(LINCTL.LINC1CAP = 1)`
• Enable LIN counter capture on falling RX edge  `(LINCTL.LINC0CAP = 1)`
• Enable LIN counter clearing on RX falling edge `(LINCTL.ZERONE = 1)`

#### C) Edge detection during SYNC field

- LIN extension features so called Capture Registers that will detect either rising or falling edge or Rx/Tx
1. LIN counter is set to 0 and start counting on the falling RX edge. (LINCTL.ZERONE = 1)
2. RX falling edge interrupt trigger `(RXNE)`:
- Read capture register `LINC0` (falling edge) and `LINC1` (rising edge) values
- Verify bit times
3. RX falling edge interrupt trigger `(RXNE)`:
- Read capture register `LINC0` (falling edge) and `LINC1` (rising edge) values
- Verify bit times
4. RX falling edge interrupt trigger `(RXNE)`:
- Read capture register `LINC0` (falling edge) and `LINC1` (rising edge) values
- Verify bit times
5. RX falling edge interrupt trigger `(RXNE)`:
- Read capture register `LINC0` (falling edge) and `LINC1` (rising edge) values
- Verify bit times
- Calculate the proper baud rate to set. Software must set the baud rate before the start bit of the PID field
after sync field.

- *TODO: Implement to code*

![Schematic](https://github.com/Musislik/lin-ground-temp-sensor/blob/main/LIN_bus.svg)
