# LIN

# TODO
- MD - Program komunikace
- PM - **Destička**, Šéfis
- OO - Rešerš ovládání LIN

# About this project
- This project demostrates local interconnect network (LIN) communication
## About LIN Protocol
Local interconnect network or (LIN) is a type of asynchronous communication between devices. It is extesively used in automotive industry for communication between non-critical peripherals. It is a low-cost solution for serial communcation in mass production [[1](https://en.wikipedia.org/wiki/Local_Interconnect_Network)].

![LIN bus](https://canlogger1000.csselectronics.com/img/lin-bus-single-wire-termination-supply-ground.svg)

LIN bus is comprised of supply voltage VBAT, ground GND and LIN signal wire.

In used microcontroller MSPM0C1104 it is featured as an extension for UART module.


- See [Technical Reference Manual](https://www.ti.com/lit/ug/slau893c/slau893c.pdf?ts=1762344977058&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSPM0C1104) of MSPM0C1104 pages 784 through 806 for UART/LIN extesion module.

# Hardware implementation of LIN protocol
## LIN transmission 
- @ Page 791
- LIN counter is clocked by UART clock
- Can be interrupted upon LINCNT counter overflow and is flagged as CPU_INT.IMASK.LINOVF

### Sending BREAK signal
- @ Page 791
- BREAK signal is set by BRK bit in UARTx.LCRH register.
- It is important to set BREAK signal before data is loaded into TXDATA or FIFO register

### Generate LIN responder signals
- @ Page 786.
- To be able to software controll generation if LIN responder signals, the TXD_OUT and
TXD_CTL_EN bit in register UARTx.CTL0 needs to be configured
- If TXD_CTL_EN = '1' then output pin can be controlled by the TXD_OUT bit if the UART transmit is disabled (CTL0.TXE is cleared).

   ```c
   #include <stdio.h>

   int main()
   {
       printf("Hello, world!");
       return 0;
   }
   ```

