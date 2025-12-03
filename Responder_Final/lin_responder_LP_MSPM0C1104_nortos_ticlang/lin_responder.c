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

uint8_t SensorData1[2] = {0xFF, 0xFF};
uint8_t SensorData2[2] = {0xFF, 0xFF};

uint16_t adcResult;


void LIN_Message_1_Handler(void) //callback pro nektera PID, (zmena stavu ledky pri prijmu packetu)
{
    memcpy(&gResponderTXMessageData, &gResponderRXBuffer, 0x08); //zkopiruj data z gResponderRXBuffer do gResponderTXMessageData 
    DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
}

LIN_table_record_t responderMessageTable[LIN_RESPONDER_NUM_MSGS] = {
    /* Important, connection number matches the order of this table */
    {0x0A, //PID adresa
        2,  //Pocet prijatych bajtu
        NULL //NULL znamena zadna callback funkce
        },   
    {   
        0x02, 
        2,    
        NULL
    },
        {
        0x03,      
        2,  
        LIN_Message_1_Handler //callback funkce
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
            for (msgTableIndex = 0; msgTableIndex < LIN_RESPONDER_NUM_MSGS; //Iteruj PID v LIN_table_record_t (identifikace PID)
                msgTableIndex++) {
                if (responderMessageTable[msgTableIndex].msgID == data) {   //Pokud zprava obsahuje znama PID prerus case
                    break;
                }
            }
            if (msgTableIndex >= LIN_RESPONDER_NUM_MSGS) {  //Pokud byl for ukoncen bez prerueseni 
                data = LIN_MESSAGE_NOT_FOUND;               //Prepis data na 0xFF (nic se nestane)
            }

            switch (data) {
                //Pri detekci techto PID zavolej callback
                case 0x03:
                    tempChksum.word = data; //Uloz PID pro spocitani checksumu
                    byteCounter     = 0;    //Vynuluj byteCounter bajtu v data framu
                    LIN_state       = LIN_RX_STATE_DATA;    //Nastav switch na LIN_RX_STATE_DATA (Pokracuj spoctenim chcecksumu z prijatych dat)
                    *gStateMachine  = LIN_STATE_DATA_FIELD; //Nastav irq FSM na LIN_STATE_DATA_FIELD (Pokracuj v extrakci ramce)
                    break;

                case 0x0A: /* ID 0x0A */    //Pri detekci techto PID pouze odesli data
                case 0x02: /* ID 0x02 */
                    delay_cycles(LIN_RESPONSE_LAPSE);
                    switch (data) {
                    case 0x0A: //Zpracuj a posli data prevodniku 0
                        adcResult = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0); //precti adc pamet s indexem 0
                        SensorData1[1] = adcResult; SensorData1[0] = adcResult >> 8;    //rozdeleni dat z 10bit AD prevodniku na 2 bajty
                        sendLINResponderTXMessage(uart, msgTableIndex, SensorData1, responderMessageTable);     //Posli gResponderTXMessageData (2bajy z adc)
                    break;
                    case 0x02: //Zpracuj a posli data prevodniku 1
                        adcResult = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_1); 
                        SensorData2[1] = adcResult; SensorData2[0] = adcResult >> 8;
                        sendLINResponderTXMessage(uart, msgTableIndex, SensorData2, responderMessageTable);
                    break;
                    }
                *gStateMachine = LIN_STATE_WAIT_FOR_BREAK; //Prepni FSM na LIN_STATE_WAIT_FOR_BREAK (Pockej na dalsi packet)
                LIN_state      = LIN_RX_STATE_ID;          //Nastav switch na LIN_RX_STATE_ID (Pri dalsim packetu se zacne identifikaci PID)
                break;
            }
            break;

        case LIN_RX_STATE_DATA:                         //Secte data a nastav LIN_STATE na checksum
            gResponderRXBuffer[byteCounter] = data;         //ulozeni aktualniho zpracovaneho bajtu do bufferu
            tempChksum.word += gResponderRXBuffer[byteCounter]; //Pridavani vsech data bajtu do tempChksum.word
            byteCounter++;                                          

            if (byteCounter >= responderMessageTable[msgTableIndex].msgSize) { //Pri zpracovani vsech bajtu 
                LIN_state = LIN_RX_STATE_CHECKSUM;                             //Prepni switch pro spocitani checksumu
            }
            break;

        case LIN_RX_STATE_CHECKSUM:
            rxChecksum = data;  //uloz checksum z prijateho packetu

            tempChksum.word = tempChksum.byte[0] + tempChksum.byte[1];
            checksum  = tempChksum.byte[0];
            checksum += tempChksum.byte[1];
            checksum = 0xFF - checksum;

            if (rxChecksum == checksum) {               //pri ekvivalenci checksumu vyvolej callback funkci
                callbackFunction = responderMessageTable[msgTableIndex].callbackFunction;
                callbackFunction();
            }

            /* Clear break field and sync byte flags */
            *gStateMachine = LIN_STATE_WAIT_FOR_BREAK;  //Nastav switch do puvodniho stavu
            LIN_state      = LIN_RX_STATE_ID;
            break;
        default:
            LIN_state = LIN_RX_STATE_ID;
            break;
    }
}

int main(void)
{
    SYSCFG_DL_init();   //inicializace (gpio, lin, adc...)

    /* Enable transceiver */
    DL_GPIO_setPins(GPIO_LIN_ENABLE_PORT, GPIO_LIN_ENABLE_USER_LIN_ENABLE_PIN);

    NVIC_ClearPendingIRQ(LIN_0_INST_INT_IRQN); //zrus cekajici preruseni
    NVIC_EnableIRQ(LIN_0_INST_INT_IRQN);       //Povol preruseni
    DL_SYSCTL_enableSleepOnExit();
    DL_ADC12_startConversion(ADC12_0_INST); //Zapni AD prevodnik

    while (1) {
        //Hlavni smycka: Pouze reaguje na interrupt
        __WFI();
/*
        adcResult = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
        SensorData[0] = adcResult; SensorData[1] = adcResult >> 8;
*/
    }
}

void LIN_0_INST_IRQHandler(void)
{
    uint16_t counterVal       = 0;
    uint8_t data              = 0;
    uint16_t averageBitTime   = 0;
    uint16_t measuredBaudRate = 0;
    
    //Stavový automat:
    // WAIT_FOR_BREAK -> Čeká na začátek LIN rámce

    // BREAK_FIELD -> Zpracovává break pole

    // SYNC_FIELD_NEG_EDGE -> Sestupná hrana sync pole
    // SYNC_FIELD_POS_EDGE -> Vzestupná hrana sync pole

    // PID_FIELD -> Prijem Protected ID

    // DATA_FIELD -> Prijem dat


    //Sekvence LIN ramce
    // [BREAK] -> [SYNC 0x55] -> [PID] -> [DATA] -> [CHECKSUM]
    //     ↓           ↓          ↓        ↓         ↓
    //FALLING_EDGE -> NEG_EDGE -> RX -> DATA_FIELD -> CHECKSUM -> POS_EDGE

    switch (DL_UART_Extend_getPendingInterrupt(LIN_0_INST)) {   //FSM extrakce packetu
        /* LIN Minimum Break Field Width Interrupt. */
        case DL_UART_EXTEND_IIDX_LIN_FALLING_EDGE:          //Detekce začátku Break pole(Sestupna hrana), (Break pole = log 0 po dobu ≥13 bitů = start komunikačního rámce)
            /* Signals the start of the break field. */
            if (gStateMachine == LIN_STATE_WAIT_FOR_BREAK) {    // 
                gStateMachine = LIN_STATE_BREAK_FIELD;          // Po detekci sestupne hrany prepni state

                counterVal = DL_UART_Extend_getLINCounterValue(LIN_0_INST);
                /* Validation check of the length of the break field. */
                if (counterVal < (gLin0TbitWidthVar * 13.5) && counterVal > (gLin0TbitWidthVar * LIN_0_TBIT_COUNTER_COEFFICIENT)) //Overeni delky break fieldu
                {
                    gStateMachine = LIN_STATE_SYNC_FIELD_NEG_EDGE;  //Prepni state na detekci synchronizacniho pole
                    DL_UART_Extend_enableLINCounterClearOnFallingEdge(LIN_0_INST); //Pri detekci dalsi falling edge(sync field) vynuluj counter
                    DL_UART_Extend_enableInterrupt(LIN_0_INST, DL_UART_EXTEND_INTERRUPT_RXD_NEG_EDGE); //Detekuj falling endge
                    DL_UART_Extend_enableLINSyncFieldValidationCounterControl(LIN_0_INST);
                } 
                else {
                    gStateMachine = LIN_STATE_WAIT_FOR_BREAK;   //Pri nespravne delce break fieldu zacni znovu cekat na bteak field
                }
                DL_UART_Extend_disableInterrupt(LIN_0_INST, DL_UART_EXTEND_INTERRUPT_RX);
            }
            break;
        /* Rising Edge Detection Interrupt on UARTxRXD. */
        case DL_UART_EXTEND_IIDX_RXD_POS_EDGE:                                  //Positivni hrana -> zacatek break field
            /* Signals the positive edge of a sync field segment. */
            if (gStateMachine == LIN_STATE_SYNC_FIELD_POS_EDGE) {
                gBitTimes[gNumCycles].posEdge = DL_UART_Extend_getLINRisingEdgeCaptureValue(LIN_0_INST); //Detekce delky log1 za pozitivni hranou
                /* Validation check of the timing of the sync field segment.
                 * Finding an invalid sync bit stores each bit time to
                 * calculate new baud rate */
                if (gBitTimes[gNumCycles].posEdge > ((gLin0TbitWidthVar * 95) / 100)    //Pri delce log1 sync fieldu s toleranci 5% mer pristi log1
                && gBitTimes[gNumCycles].posEdge < ((gLin0TbitWidthVar * 105) / 100)) 
                {
                    gNumCycles++;
                } 
                else if (!gFirstSyncBit) {
                    gTotalBitTime =
                        gTotalBitTime + gBitTimes[gNumCycles].posEdge; //Pri jine delce log1 nastav novou namerenou delku
                    gNumSyncErrors++;
                } 
                else {
                    gFirstSyncBit = false;
                }
                /* Only 5 segments of a sync field. */
                if ((gNumSyncErrors + gNumCycles) == LIN_RESPONDER_SYNC_CYCLES) {
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
        case DL_UART_EXTEND_IIDX_RX:                                 
            /* Determine if SYNC byte was received */
            if (gStateMachine == LIN_STATE_SYNC_FIELD_POS_EDGE) { //Po detekci sync fieldu 
                data = DL_UART_Extend_receiveData(LIN_0_INST);    //prijmi data
                if (data == LIN_SYNC_BYTE) {
                    /* First received byte is SYNC. Ignore it and wait for PID */
                    gStateMachine = LIN_STATE_PID_FIELD;
                } else if ((data != LIN_SYNC_BYTE) && gAutoBaudUsed) { 
                    /* Incorrect baud rate can cause an incorrect sync byte to be received.
                     * If autobaud was used and the sync byte was incorrect, the state machine
                     * proceeds as usual.*/
                    gStateMachine = LIN_STATE_PID_FIELD; //Byla pouzita korekce baudu a neprisel sync bajt -> prepni na prijem PID
                } else {
                    /* Unexpected byte, return to idle status */
                    gStateMachine = LIN_STATE_WAIT_FOR_BREAK; 
                }
            } else if (gStateMachine == LIN_STATE_PID_FIELD) {
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
