# LIN

# TODO
MD - Program komunikace
PM - **Destička**, Šéfis
OO - Rešerš ovládání LIN

# About this project
This project demostrates local interconnect network (LIN) communication

See [Technical Reference Manual](https://www.ti.com/lit/ug/slau893c/slau893c.pdf?ts=1762344977058&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSPM0C1104) of MSPM0C1104 pages 784 through 806 for UART/LIN extesion module.

# Hardware implementation of LIN protocol
## LIN transmission 
@ Page 786.
Sending the break signal can be done by setting the BRK bit in UARTx.LCRH register. This bit needs to be set
before the data is written into the FIFO or transmit data register TXDATA.
To generate LIN responder signals such as wake signals, the TX pin can be configured by TXD_OUT and
TXD_CTL_EN bits in register UARTx.CTL0 to be software controlled. By setting TXD_CTL_EN bit to 1, the TX
output pin can be controlled by the TXD_OUT bit if the UART transmit is disabled (CTL0.TXE is cleared).

LIN counter is clocked by UART clock

### Sending BREAK signal
@ Page 791
BREAK signal is set by BRK bit in UARTx.LCRH.
It is important to set BREAK signal before data is loaded into TX register

