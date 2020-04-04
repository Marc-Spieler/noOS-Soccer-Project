/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.03.2020                                                  */
/************************************************************************/

#include "serial.h"
#include "string.h"
#include "compass.h"
#include "data.h"
#include "experiments.h"
#include "motor.h"
#include "ric.h"
#include "timing.h"

#define SYNC        0x55
#define ESC         0xaa
#define ESC_ESC     0x00
#define ESC_SYNC    0x01

uint8_t txLen = 0;
uint8_t txId = 0;
uint8_t txBuf[32];

Bool frameStarted = false;
Bool escDetected = false;
uint8_t rxMsg[32];
uint8_t txMsg[32];
uint8_t rxByteCnt = 0;
uint8_t txByteCnt = 0;
uint8_t rxMsgLen = 0;
uint8_t txMsgLen = 0;
uint8_t rxChecksum = 0;
uint8_t txChecksum = 0;

uint32_t lastTx = 0;
uint32_t lastRx = 0;

uint8_t dataRequestCnt = 0;
uint8_t requestedData[32];

typedef enum
{
    DR_BATTERY_P = 1,
    DR_COMPASS,
    DR_LINE,
    DR_LINE_CALIBRATION,
} dataRequest_t;

typedef enum
{
    EC_UPDATE_LED = 128,
    EC_UPDATE_LINE_CALIBRATION,
    EC_ACTION,
    EC_MOTOR_ENABLE,
    EC_MOTOR_DISABLE,
    EC_MOTOR_LEFT,
    EC_MOTOR_RIGHT,
    EC_MOTOR_REAR,
    EC_MOTOR_MOVE,
    EC_COMPASS_CALIBRATION,
} executionCommand_t;

void serialWrite(uint8_t *pbuf, uint8_t len);
void addByteToTxBuffer(uint8_t byteToAdd);
void processRxPacket(void);

void serialInit(void)
{
    pmc_enable_periph_clk(ID_USART0);
    usart_disable_rx(USART0);
    usart_disable_tx(USART0);
    
    sam_usart_opt_t uart_settings =
    {
        .baudrate = 38400,
        .char_length = US_MR_CHRL_8_BIT,
        .stop_bits = US_MR_NBSTOP_1_BIT,
        .parity_type = US_MR_PAR_NO,
        .channel_mode = US_MR_CHMODE_NORMAL,
        .irda_filter = 0
    };
    
    usart_init_rs232(USART0, &uart_settings, sysclk_get_peripheral_hz());
    usart_enable_interrupt(USART0, US_IER_RXRDY);
    NVIC_EnableIRQ(USART0_IRQn);
    usart_enable_rx(USART0);
    usart_enable_tx(USART0);
}

void serialWrite(uint8_t *pbuf, uint8_t len)
{
    txLen = len;
    txId = 0;
    memcpy(txBuf, pbuf, len);
    usart_write(USART0, txBuf[txId++]);
    usart_enable_interrupt(USART0, US_IER_TXRDY);
}

void addByteToTxBuffer(uint8_t byteToAdd)
{
    txChecksum += byteToAdd;
    
    switch (byteToAdd)
    {
        case SYNC:
            txMsg[txByteCnt++] = ESC;
            txMsg[txByteCnt++] = ESC_SYNC;
            break;
        case ESC:
            txMsg[txByteCnt++] = ESC;
            txMsg[txByteCnt++] = ESC_ESC;
            break;
        default:
            txMsg[txByteCnt++] = byteToAdd;
            break;
    }
}

void serialMaintenance(void)
{
    if((getTicks() - lastTx) >= 200)
    {
        lastTx = getTicks();
        
        uint8_t handledDataRequests = 0;
    
        txMsg[0] = SYNC;
        txByteCnt = 1;
        addByteToTxBuffer(0x00); // message length spacing
        txChecksum = 0;
        txMsgLen = 0;
    
        while(handledDataRequests < dataRequestCnt)
        {
            switch(requestedData[handledDataRequests])
            {
                case DR_BATTERY_P:
                    addByteToTxBuffer(DR_BATTERY_P);
                    addByteToTxBuffer(s2m.battery.percentage);
                    txMsgLen += 2;
                    break;
                case DR_COMPASS:
                    addByteToTxBuffer(DR_COMPASS);
                    addByteToTxBuffer(((uint16_t)((data.compass + 180.0f) * 10.0f) & 0xff));
                    addByteToTxBuffer((((uint16_t)((data.compass + 180.0f) * 10.0f) & 0xf00) >> 8));
                    txMsgLen += 3;
                    break;
                case DR_LINE:
                    addByteToTxBuffer(DR_LINE);
                    addByteToTxBuffer((s2m.line.all & 0xff));
                    addByteToTxBuffer(((s2m.line.all & 0xf00) >> 8));
                    txMsgLen += 3;
                    break;
                case DR_LINE_CALIBRATION:
                    addByteToTxBuffer(DR_LINE_CALIBRATION);
                    addByteToTxBuffer(m2s.line_cal_value);
                    txMsgLen += 2;
                default:
                    break;
            }
        
            handledDataRequests++;
        }
    
        txMsg[1] = txMsgLen;
        addByteToTxBuffer(txChecksum);
        serialWrite(txMsg, txByteCnt);
    }        
}

void processRxPacket(void)
{
    uint8_t processedBytes = 0;
    dataRequestCnt = 0;
    memset(requestedData, 0, sizeof(requestedData));
    
    while(processedBytes < (rxByteCnt - 2))
    {
        if(rxMsg[processedBytes] <= 0x7f) // data request
        {
            requestedData[dataRequestCnt++] = rxMsg[processedBytes];
        }
        else // execution command
        {
            switch(rxMsg[processedBytes])
            {
                case EC_UPDATE_LED:
                    //ioport_set_pin_level(LED_ONBOARD, (rxMsg[++processedBytes] > 0 ? true : false));
                    break;
                case EC_UPDATE_LINE_CALIBRATION:
                    m2s.line_cal_value = rxMsg[++processedBytes];
                    break;
                case EC_ACTION:
                    doTest = true;
                    break;
                case EC_MOTOR_ENABLE:
                    motorEnable();
                    break;
                case EC_MOTOR_DISABLE:
                    motorDisable();
                    break;
                case EC_MOTOR_LEFT:
                    motorSetIndividual(MOTOR_LEFT, (rxMsg[++processedBytes] - 100));
                    break;
                case EC_MOTOR_RIGHT:
                    motorSetIndividual(MOTOR_RIGHT, (rxMsg[++processedBytes] - 100));
                    break;
                case EC_MOTOR_REAR:
                    motorSetIndividual(MOTOR_REAR, (rxMsg[++processedBytes] - 100));
                    break;
                case EC_MOTOR_MOVE:
                    moveRobot(rxMsg[++processedBytes], rxMsg[++processedBytes], (rxMsg[++processedBytes] - 100));
                    break;
                case EC_COMPASS_CALIBRATION:
                    ioport_set_pin_level(LED_ONBOARD, true);
                    compassCalibrationStep();
                    ioport_set_pin_level(LED_ONBOARD, false);
                    break;
                default:
                    break;
            }
        }
    
        processedBytes++;
    }
}

void USART0_Handler(void)
{
    uint32_t uartStatus = usart_get_status(USART0);

    if (uartStatus & US_CSR_TXRDY)
    {
        if(txId < txLen)
        {
            usart_write(USART0, txBuf[txId++]);
        }
        else
        {
            usart_disable_interrupt(USART0, US_IDR_TXRDY);
        }
    }

    if (uartStatus & US_CSR_RXRDY)
    {
        uint32_t newData = 0;
        usart_read(USART0, &newData);
        uint8_t newByte = newData;
        //lastRx = getTicks();
        
        // detect SYNC character to define frame start
        if (newByte == SYNC)
        {
            if (frameStarted)
            {
                //frameError++;
            }

            // start a new frame
            frameStarted = true;
            rxByteCnt = 0;
        }

        // detect ESC character
        if (newByte == ESC)
        {
            escDetected = true;
        }
        else
        {
            // replace ESC sequence with correct byte
            if (escDetected)
            {
                escDetected = false;

                switch (newByte)
                {
                    case ESC_ESC:
                        newByte = ESC;
                        break;
                    case ESC_SYNC:
                        newByte = SYNC;
                        break;
                    default:
                        break;
                }
            }
        }

        if (frameStarted && !escDetected)
        {
            switch (rxByteCnt)
            {
                case 0: // SYNC byte
                    break;
                case 1:
                    rxMsgLen = newByte;
                    rxChecksum = 0;
                    break;
                default:
                    if (rxMsgLen > 0)
                    {
                        rxMsg[rxByteCnt - 2] = newByte;
                        rxChecksum += newByte;
                        rxMsgLen--;
                    }
                    else
                    {
                        // if message is error free then call process handler
                        if (rxChecksum == newByte)
                        {
                            processRxPacket();
                        }
                        else
                        {
                            //checksumError++;
                        }

                        frameStarted = false;
                        escDetected = false;
                    }
                break;
            }

            rxByteCnt++;
        }            
    }        
}
