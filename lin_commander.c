/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


//Responder


//Includes
#include <stdio.h>
#include <string.h>
#include "lin_config.h"
#include "ti_msp_dl_config.h"

//GPIO
#define GPIO_LEDS_USER_LED_11_PIN                                (DL_GPIO_PIN_22)
#define GPIO_LEDS_USER_LED_11_IOMUX                              (IOMUX_PINCM23 )
#define GPIO_LEDS_USER_TEST_PIN_11                               (DL_GPIO_PIN_2 )
#define GPIO_LEDS_USER_TEST_IOMUX_11                             (IOMUX_PINCM3  )

//Definition of n of bytes to be received including checksum
#define DATA_LENGTH (3)

//Definition of number of polls commander does
#define N_OF_POLLS (2)

//Arbitrary threshold value to compare received data to
#define THRSH_VALUE 0x00FF

//Default state for LIN reception FSM
volatile LIN_STATE gStateMachine = LIN_STATE_WAIT_FOR_BREAK;

//Flags for interrupts
//Flag for triggering LIN communication
volatile bool gSendMessage       = false;
volatile bool gDataReceived      = false;

//Buffer, where received data is stored
uint8_t gCommanderRXBuffer[LIN_DATA_MAX_BUFFER_SIZE] = {0};

//Data to transmit
uint8_t gCommanderTXPacket[] = {0x2, 0x4};

//ADC values are written to his variable
uint16_t adcVal = 0;

//Left for debugging purposes
volatile bool test = false;
//This variable is defined for counting n of polled PIDs
uint8_t increment = 0;
//Counting number of bytes received by LIN
uint8_t recvIncr = 0;
//Flag to indicate that 
bool receiveFlag = false;

//Callback function definition
static void LIN_processMessage_Rx1(void);

//Table of PIDs
LIN_table_record_t gCommanderMessageTable[LIN_COMMANDER_NUM_MSGS] = {
    [0] = {0x0A, 2, LIN_processMessage_Rx1}, 
    [1] = {0x02, 2, LIN_processMessage_Rx1},
    {0x03, 2, NULL},
};


int main(void)
{
    //Configure pins based on .syscfg file
    SYSCFG_DL_init();

    //Init GPIO
    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_11_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_TEST_IOMUX_11);

    DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN |
		GPIO_LEDS_USER_TEST_PIN_11);
    DL_GPIO_enableOutput(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN |
		GPIO_LEDS_USER_TEST_PIN_11);

    //Enables transceiver
    //Sets LIN_EN pin high for proper transceiver functionality
    DL_GPIO_setPins(GPIO_LIN_ENABLE_PORT, GPIO_LIN_ENABLE_USER_LIN_ENABLE_PIN);
    //Enable interrupts for LIN
    NVIC_ClearPendingIRQ(LIN_0_INST_INT_IRQN);
    NVIC_EnableIRQ(LIN_0_INST_INT_IRQN);
    NVIC_EnableIRQ(GPIO_SWITCHES1_INT_IRQN);

    //LED signalization
    DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN | GPIO_LEDS_USER_TEST_PIN_11);
    


    while (1) {    
        //Wait for interrupt 
        __WFI();
        //if correct number of bytes of specified packet is received by UART, another PID is polled
        if (recvIncr >= DATA_LENGTH)
        {
            //receiveFlag is raised when correct packet was received
            receiveFlag = true;
            //annulate byte counter
            recvIncr = 0;
        }

            //Poll all PIDs where data should be received from responders
          if (gSendMessage == true && receiveFlag == true) {
            receiveFlag = false;
            //Necessary delay of 1 ms (can be less, but just for good measure) for responder to adjust for next exchange
            delay_cycles(LIN_BREAK_LENGTH);

            if (increment < N_OF_POLLS)
            {
                //Transmit msg
                LIN_Commander_transmitMessage(LIN_0_INST, increment, gCommanderRXBuffer, gCommanderMessageTable);
                //DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN | GPIO_LEDS_USER_TEST_PIN_11);
                increment++;
            }
            else 
            {
                //Poll PIDs where data should be configured at responders
                //LIN_Commander_transmitMessage(LIN_0_INST, increment, gCommanderTXPacket, gCommanderMessageTable);
                //clear flags and increment
                increment = 0;
                gSendMessage = false;
            }
            }

        }
    }


void GPIOA_IRQHandler(void)
{
        gSendMessage = true;
        receiveFlag = true;
}

//Callback function that processes data received from LIN
static void LIN_processMessage_Rx1(void)
{     
    //Simple comparation of received values
    adcVal = (uint16_t) (gCommanderRXBuffer[1] << 8) | gCommanderRXBuffer[0];                                 
    if (adcVal <= THRSH_VALUE)
    {
        DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN | GPIO_LEDS_USER_TEST_PIN_11);
    }
    else
    {
        DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN | GPIO_LEDS_USER_TEST_PIN_11);
    }
}

//Handling interrupt for incoming data
void LIN_0_INST_IRQHandler(void)
{
    //Temp var to write data from UART to
    uint8_t data = 0;
    switch (DL_UART_Extend_getPendingInterrupt(LIN_0_INST)) {
        case DL_UART_EXTEND_IIDX_RX:
            //Process received data from responder
            data = DL_UART_Extend_receiveData(LIN_0_INST);
            LIN_Commander_receiveMessage(LIN_0_INST, data, gCommanderRXBuffer, gCommanderMessageTable);
            recvIncr++;
            break;
        default:
            break;
    }
}

