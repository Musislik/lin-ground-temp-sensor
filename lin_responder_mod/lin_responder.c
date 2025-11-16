#include <stdio.h>
#include <string.h>
#include "lin_config.h"
#include "ti_msp_dl_config.h"

volatile LIN_STATE gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
volatile uint16_t gNumCycles     = 0;
volatile LIN_Sync_Bits gBitTimes[LIN_RESPONDER_SYNC_CYCLES];
uint8_t gResponderRXBuffer[LIN_DATA_MAX_BUFFER_SIZE]      = {0};
uint8_t gResponderTXMessageData[LIN_DATA_MAX_BUFFER_SIZE] = {0};

/* Variables for autobaud feature */

volatile uint8_t gNumSyncErrors        = 0;
volatile bool gFirstSyncBit            = true;
volatile uint16_t gTotalBitTime        = 0;
volatile uint16_t gLinResponseLapseVar = LIN_RESPONSE_LAPSE;
volatile bool gAutoBaudUsed            = false;
volatile uint32_t gLin0TbitWidthVar    = LIN_0_TBIT_WIDTH;
volatile uint16_t gPrevBaudRate        = LIN_0_BAUD_RATE;
volatile uint16_t gCurrBaudRate        = LIN_0_BAUD_RATE;

uint8_t SensorData[LIN_DATA_MAX_BUFFER_SIZE] = {0x2, 0x4, 0x6, 0x8, 0x0A, 0x0C, 0x0E, 0x10};


void LIN_Message_1_Handler(void) //callback pro nektera PID, (zmena stavu ledky pri prijmu packetu)
{
    memcpy(&gResponderTXMessageData[0], &gResponderRXBuffer[0], 0x08); //zkopiruj data(8 bajtu) z gResponderRXBuffer do gResponderTXMessageData 
    DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
}

LIN_table_record_t responderMessageTable[LIN_RESPONDER_NUM_MSGS] = {
    /* Important, connection number matches the order of this table */
    {0x00, /* PID 0x39 */                               //Přijmi data do bufferu
        8, LIN_Message_1_Handler},
    {
        0x01, //PID adresa                                  //Odesli odpoved
        8,    //Pocet prijatych bajtu
    },
};




void setLINResponderRXMessage(UART_Regs *uart, uint8_t data, volatile LIN_STATE *gStateMachine){

    static uint8_t msgTableIndex = 0;
    static uint8_t byteCounter   = 0;
    static LIN_word_t tempChksum;
    static LIN_RX_STATE LIN_state = LIN_RX_STATE_ID;
    volatile uint8_t checksum;
    uint8_t rxChecksum;
    LIN_function_ptr_t callbackFunction;

    switch (LIN_state) {
        case LIN_RX_STATE_ID:
            /* Find ID message */
            for (msgTableIndex = 0; msgTableIndex < LIN_RESPONDER_NUM_MSGS;
                 msgTableIndex++) {
                if (responderMessageTable[msgTableIndex].msgID == data) {
                    break;
                }
            }

            if (msgTableIndex >= LIN_RESPONDER_NUM_MSGS) {
                data = LIN_MESSAGE_NOT_FOUND;
            }

            switch (data) {
                case 0x00: /* ID 0x39 */   //Pri detekci techto PID zpracuj data a proved callback
                    tempChksum.word = data;
                    byteCounter     = 0;
                    LIN_state       = LIN_RX_STATE_DATA;
                    *gStateMachine  = LIN_STATE_DATA_FIELD;
                    break;

                case 0x01: /* ID 0x08 */    //Pri detekci techto PID pouze odesli data
                    /*
                     * Delay is used to ensure STOP bit after PID is completely
                     * received before data transmitted.
                     */
                    delay_cycles(LIN_RESPONSE_LAPSE);
                    sendLINResponderTXMessage(uart, msgTableIndex, SensorData, responderMessageTable);     //Posli gResponderTXMessageData
                    /* Toggle LED1 with TX packet */
                    DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
                    *gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
                    LIN_state      = LIN_RX_STATE_ID;
                    break;
                default:
                    *gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
                    break;
            }
            break;

        case LIN_RX_STATE_DATA:                         //secti data a nastav stav checksum
            gResponderRXBuffer[byteCounter] = data;
            tempChksum.word += gResponderRXBuffer[byteCounter];
            byteCounter++;

            if (byteCounter >= responderMessageTable[msgTableIndex].msgSize) {
                LIN_state = LIN_RX_STATE_CHECKSUM;
            }
            break;

        case LIN_RX_STATE_CHECKSUM:
            rxChecksum = data;

            tempChksum.word = tempChksum.byte[0] + tempChksum.byte[1];
            checksum        = tempChksum.byte[0];
            checksum += tempChksum.byte[1];
            checksum = 0xFF - checksum;

            if (rxChecksum == checksum) {               //pri pozitivnim vysledku vyvolej callback funkci
                callbackFunction = responderMessageTable[msgTableIndex].callbackFunction;
                callbackFunction();
            }

            /* Clear break field and sync byte flags */
            *gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
            LIN_state      = LIN_RX_STATE_ID;
            break;
        default:
            LIN_state = LIN_RX_STATE_ID;
            break;
    }
}

int main(void)
{
    SYSCFG_DL_init();

    /* Enable transceiver */
    DL_GPIO_setPins(GPIO_LIN_ENABLE_PORT, GPIO_LIN_ENABLE_USER_LIN_ENABLE_PIN);

    NVIC_ClearPendingIRQ(LIN_0_INST_INT_IRQN);
    NVIC_EnableIRQ(LIN_0_INST_INT_IRQN);
    DL_SYSCTL_enableSleepOnExit();

    while (1) {
        //Hlavní smyčka: Pouze reaguje na interrupt
        __WFI();
    }
}

void LIN_0_INST_IRQHandler(void)
{
    uint16_t counterVal       = 0;
    uint8_t data              = 0;
    uint16_t averageBitTime   = 0;
    uint16_t measuredBaudRate = 0;
    
    //Stavový automat:
    // WAIT_FOR_BREAK → Čeká na začátek LIN rámce

    // BREAK_FIELD → Zpracovává break pole

    // SYNC_FIELD_NEG_EDGE → Sestupná hrana sync pole
    // SYNC_FIELD_POS_EDGE → Vzestupná hrana sync pole

    // PID_FIELD → Příjem Protected ID

    // DATA_FIELD → Příjem dat


    //Sekvence LIN ramce
    // [BREAK] → [SYNC 0x55] → [PID] → [DATA] → [CHECKSUM]
    //     ↓           ↓          ↓        ↓         ↓
    //     FALLING_EDGE → NEG_EDGE → RX → DATA_FIELD → CHECKSUM
    //                 POS_EDGE

    switch (DL_UART_Extend_getPendingInterrupt(LIN_0_INST)) {
        /* LIN Minimum Break Field Width Interrupt. */
        case DL_UART_EXTEND_IIDX_LIN_FALLING_EDGE:                              //Detekce začátku Break pole (Break pole = log 0 po dobu ≥13 bitů = start komunikačního rámce)
            /* Signals the start of the break field. */
            if (gStateMachine == LIN_STATE_WAIT_FOR_BREAK) {
                gStateMachine = LIN_STATE_BREAK_FIELD;

                counterVal = DL_UART_Extend_getLINCounterValue(LIN_0_INST);
                /* Validation check of the length of the break field. */
                if (counterVal < (gLin0TbitWidthVar * 13.5) && counterVal > (gLin0TbitWidthVar * LIN_0_TBIT_COUNTER_COEFFICIENT)) //Pri spravnem poctu low bitu detekuj sync falling edge
                {
                    gStateMachine = LIN_STATE_SYNC_FIELD_NEG_EDGE;
                    DL_UART_Extend_enableLINCounterClearOnFallingEdge(LIN_0_INST);
                    DL_UART_Extend_enableInterrupt(LIN_0_INST, DL_UART_EXTEND_INTERRUPT_RXD_NEG_EDGE);
                    DL_UART_Extend_enableLINSyncFieldValidationCounterControl(LIN_0_INST);
                } else {
                    gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
                }

                DL_UART_Extend_disableInterrupt(
                    LIN_0_INST, DL_UART_EXTEND_INTERRUPT_RX);
            }
            break;
        /* Rising Edge Detection Interrupt on UARTxRXD. */
        case DL_UART_EXTEND_IIDX_RXD_POS_EDGE:                                  //Kalibrace Baud rate podle sync field
            /* Signals the positive edge of a sync field segment. */
            if (gStateMachine == LIN_STATE_SYNC_FIELD_POS_EDGE) {
                gBitTimes[gNumCycles].posEdge = DL_UART_Extend_getLINRisingEdgeCaptureValue(LIN_0_INST);
                /* Validation check of the timing of the sync field segment.
                 * Finding an invalid sync bit stores each bit time to
                 * calculate new baud rate */
                if (gBitTimes[gNumCycles].posEdge > ((gLin0TbitWidthVar * 95) / 100) 
                && gBitTimes[gNumCycles].posEdge < ((gLin0TbitWidthVar * 105) / 100)) 
                {
                    gNumCycles++;
                } 
                else if (!gFirstSyncBit) {
                    gTotalBitTime =
                        gTotalBitTime + gBitTimes[gNumCycles].posEdge;
                    gNumSyncErrors++;
                } 
                else {
                    gFirstSyncBit = false;
                }
                /* Only 5 segments of a sync field. */
                if ((gNumSyncErrors + gNumCycles) ==
                    LIN_RESPONDER_SYNC_CYCLES) {
                    DL_UART_Extend_enableInterrupt(
                        LIN_0_INST, DL_UART_EXTEND_INTERRUPT_RX);
                    DL_UART_Extend_disableInterrupt(
                        LIN_0_INST, DL_UART_EXTEND_INTERRUPT_RXD_NEG_EDGE);

                    /* Track new and previous baud rate for validation when
                     * increasing baud rate. Ensures that resets to deal with
                     * overrun errors happen on the appropriate frame.
                     * Reset all variables relevant to sync field */
                    if (gNumCycles == LIN_RESPONDER_SYNC_CYCLES) {
                        gPrevBaudRate = gCurrBaudRate;
                        gAutoBaudUsed = false;
                    }
                    gNumCycles     = 0;
                    gNumSyncErrors = 0;
                    gTotalBitTime  = 0;
                    gFirstSyncBit  = true;

                    /* If 4 sync errors are detected, update baud rate given
                 * autobaud is enabled*/

                } else if ((gNumSyncErrors == AUTO_BAUD_THRESHOLD) &&
                           AUTO_BAUD_ENABLED) {
                    averageBitTime   = gTotalBitTime / gNumSyncErrors;
                    measuredBaudRate = LIN_0_INST_FREQUENCY / averageBitTime;

                    DL_UART_disable(LIN_0_INST);
                    delay_cycles(AUTO_BAUD_CONFIG_DELAY);
                    DL_UART_disableFIFOs(LIN_0_INST);
                    gLinResponseLapseVar =
                        LIN_0_INST_FREQUENCY / (2 * measuredBaudRate);
                    gLin0TbitWidthVar = averageBitTime;
                    DL_UART_configBaudRate(LIN_0_INST, LIN_0_INST_FREQUENCY, measuredBaudRate);

                    DL_UART_Extend_setLINCounterCompareValue(LIN_0_INST,
                        gLin0TbitWidthVar * LIN_0_TBIT_COUNTER_COEFFICIENT);
                    DL_UART_Extend_enable(LIN_0_INST);
                    gPrevBaudRate = gCurrBaudRate;
                    gCurrBaudRate = measuredBaudRate;

                    gAutoBaudUsed = true;
                    gStateMachine = LIN_STATE_SYNC_FIELD_NEG_EDGE;
                }

                else {
                    gStateMachine = LIN_STATE_SYNC_FIELD_NEG_EDGE;
                }
            }
            break;
        /* Counter Overflow Interrupt. */
        case DL_UART_EXTEND_IIDX_LIN_COUNTER_OVERFLOW:
            gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
            break;
        /* Negative Edge Detection Interrupt on UARTxRXD. */
        case DL_UART_EXTEND_IIDX_RXD_NEG_EDGE:
            /* Signals the negative edge of a sync field segment. */
            if (gStateMachine == LIN_STATE_SYNC_FIELD_NEG_EDGE) {
                /* If flag for overrun has gone off and previous
                 * baud rate < current baud rate, turn counter off and on. */
                if ((gCurrBaudRate > gPrevBaudRate) &&
                    DL_UART_getErrorStatus(
                        LIN_0_INST, DL_UART_ERROR_OVERRUN)) {
                    DL_UART_disableLINCounter(LIN_0_INST);
                    DL_UART_enableLINCounter(LIN_0_INST);
                }
                gBitTimes[gNumCycles].negEdge =
                    DL_UART_Extend_getLINFallingEdgeCaptureValue(LIN_0_INST);
                gStateMachine = LIN_STATE_SYNC_FIELD_POS_EDGE;
            }
            break;
        /* UARTxRX Data Detection Interrupt. */
        case DL_UART_EXTEND_IIDX_RX:                                        //Příjem dat LIN rámce
            /* Determine if SYNC byte was received */
            if (gStateMachine == LIN_STATE_SYNC_FIELD_POS_EDGE) {
                data = DL_UART_Extend_receiveData(LIN_0_INST);
                if (data == LIN_SYNC_BYTE) {
                    /* First received byte is SYNC. Ignore it and wait for PID */
                    gStateMachine = LIN_STATE_PID_FIELD;
                } else if ((data != LIN_SYNC_BYTE) && gAutoBaudUsed) {
                    /* Incorrect baud rate can cause an incorrect sync byte to be received.
                     * If autobaud was used and the sync byte was incorrect, the state machine
                     * proceeds as usual.*/
                    gStateMachine = LIN_STATE_PID_FIELD;
                } else {
                    /* Unexpected byte, return to idle status */
                    gStateMachine = LIN_STATE_WAIT_FOR_BREAK;
                }
            } else if (gStateMachine == LIN_STATE_PID_FIELD) {                        //zjisti PID
                data = DL_UART_Extend_receiveData(LIN_0_INST);

                /* Process the PID. The state machine will be updated */
                setLINResponderRXMessage(LIN_0_INST, data, &gStateMachine);
            }
            /* Signals the data field */
            else if (gStateMachine == LIN_STATE_DATA_FIELD) {                         //Zpracuj data
                data = DL_UART_Extend_receiveData(LIN_0_INST);

                /*
                 *  Process data byte.
                 *  If data byte matches ID byte from "responderMessageTable"
                 *  then performs the matching function. Otherwise does nothing.
                 */
                setLINResponderRXMessage(LIN_0_INST, data, &gStateMachine);
            }
            /* Flushes any data that comes after an invalid PID*/
            else if (gStateMachine == LIN_STATE_WAIT_FOR_BREAK) {
                data = DL_UART_Extend_receiveData(LIN_0_INST);
            }
        default:
            break;
    }
}
