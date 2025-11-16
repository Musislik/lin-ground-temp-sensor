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

//kod pro Master




#include <stdio.h>
#include <string.h>
#include "lin_config.h"
#include "ti_msp_dl_config.h"

#define GPIO_LEDS_USER_LED_11_PIN                                (DL_GPIO_PIN_22)
#define GPIO_LEDS_USER_LED_11_IOMUX                              (IOMUX_PINCM23 )
#define GPIO_LEDS_USER_TEST_PIN_11                               (DL_GPIO_PIN_2 )
#define GPIO_LEDS_USER_TEST_IOMUX_11                             (IOMUX_PINCM3  )

#define LIN_TABLE_INDEX_PID_00 (0)
#define LIN_TABLE_INDEX_PID_01 (1)

volatile LIN_STATE gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
volatile bool gSendMessage       = false;
volatile bool gDataReceived      = false;

uint8_t gCommanderRXBuffer[LIN_DATA_MAX_BUFFER_SIZE] = {0};
uint8_t gCommanderTXPacket[] = {0x2, 0x4, 0x6, 0x8, 0x0A, 0x0C, 0x0E, 0x10};

volatile bool test = false;

static void LIN_processMessage_Rx(void);

LIN_table_record_t gCommanderMessageTable[LIN_COMMANDER_NUM_MSGS] = {
    [0] = {0x00, 8, NULL},                  //PID pro zápis
    [1] = {0x01, 8, LIN_processMessage_Rx}, //PID pro příjem
};


int main(void)
{
    SYSCFG_DL_init();

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_11_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_TEST_IOMUX_11);

    DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN |
		GPIO_LEDS_USER_TEST_PIN_11);
    DL_GPIO_enableOutput(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN |
		GPIO_LEDS_USER_TEST_PIN_11);

    /* Enable transceiver */
    DL_GPIO_setPins(GPIO_LIN_ENABLE_PORT, GPIO_LIN_ENABLE_USER_LIN_ENABLE_PIN);
    NVIC_ClearPendingIRQ(LIN_0_INST_INT_IRQN);
    NVIC_EnableIRQ(LIN_0_INST_INT_IRQN);
    NVIC_EnableIRQ(GPIO_SWITCHES1_INT_IRQN);

    DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN | GPIO_LEDS_USER_TEST_PIN_11);
    


    while (1) {
        __WFI();

        if (gSendMessage == true) { /* Send LIN message PID_1 */ // stav gSendMessage1 je rizen externim presurenim na GPIO pinu
            gSendMessage = false;
            gDataReceived = false;
            test = !test;   //flag pro debugger
            
            LIN_Commander_transmitMessage(LIN_0_INST, LIN_TABLE_INDEX_PID_01, gCommanderRXBuffer, gCommanderMessageTable); //pošli data
            DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_11_PIN | GPIO_LEDS_USER_TEST_PIN_11);
        }
    }
}

void GPIOA_IRQHandler(void)
{
        gSendMessage = true;
}

static void LIN_processMessage_Rx(void) //Callback pro PID která prijimaji data. Pouze nastaví flag ze jsou data prijata a zmeni stav ledky
{                                       //Vytvorit zvlast funkci pro kazde PID
    /* Message received */
    gDataReceived = true;
    /* Toggle LED with RX packet */
}

void LIN_0_INST_IRQHandler(void) //Preruseni pro zpracovani prijatych dat. Muze byt obecna pro vsechny PID, Pouze sebere data z LIN framu a nacte do bufferu
{
    uint8_t data = 0;
    switch (DL_UART_Extend_getPendingInterrupt(LIN_0_INST)) {
        case DL_UART_EXTEND_IIDX_RX:
            /* Process data received from the Responder */
            data = DL_UART_Extend_receiveData(LIN_0_INST);
            LIN_Commander_receiveMessage(LIN_0_INST, data, gCommanderRXBuffer, gCommanderMessageTable);
            DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
            break;
        default:
            break;
    }
}



