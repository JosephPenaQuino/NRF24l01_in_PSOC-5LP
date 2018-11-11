/* ========================================
 *
 * Copyright UTEC, 2018
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
/* ---------------------- Dependencias ----------------------*/
// Macros : Commands
#define R_REGISTER          0b000
#define W_REGISTER          0b001
#define R_RX_PAYLOAD        0b01100001
#define W_TX_PAYLOAD        0b10100000

// Macros : Registers
#define R_RX_PL_WID         0b0110000
#define RX_PW_P0            0x11
#define EN_RXADDR           0x02
#define CONFIG              0X00
#define DYNPD               0x1C
#define FEATURE             0x1D
// Macros : Bits
#define EN_CRC              3
#define PWR_UP              1
#define PRIM_RX             0
#define EN_DPL              2
// Typedefs
typedef unsigned long long  int word;
// Libraries and Headers
#include "project.h"
// Global Variables
char ready = 0;
// Function Declaration
void PinIsrHandler (void);
void PinSendIsrHandler (void);
word UseCommand(char command, char parameter, unsigned int data);
void PinTxIsrHandler (void);
/* ------------------------ Int main ------------------------*/
int main(void) {
    // Start LCD
    LCD_Start();
    LCD_ClearDisplay();
    // Start SPI
    SPIM_Start();
    SPIM_2_Start();
    // Start Interrupts
    isr_pin_Start();
    isr_pin_StartEx(PinIsrHandler);
    isr_pin_send_Start();
    isr_pin_send_StartEx(PinSendIsrHandler);
    isr_pin_Tx_Start();
    isr_pin_Tx_StartEx(PinTxIsrHandler);
    // Enable global interrupt
    CyGlobalIntEnable; 
    for(;;) {
        // Print LCD Screen
        LCD_ClearDisplay();
        LCD_Position(0u,0u);
        LCD_PrintString("Rx:     Tx:  ");
        // Print SPI's buffer 1
        LCD_Position(1u, 0u);
        LCD_PrintDecUint16(SPIM_GetRxBufferSize());
        // Print SPI's data 1
        LCD_Position(1u,3u);
        LCD_PrintHexUint16(SPIM_ReadRxData());
        // Print SPI's buffer 2
        LCD_Position(1u, 9u);
        LCD_PrintDecUint16(SPIM_2_GetRxBufferSize());
        // Print SPI's data 2
        LCD_Position(1u,12u);
        LCD_PrintHexUint16(SPIM_2_ReadRxData());
        // Delay 2 seconds
        CyDelay(1000);
    }
}
/* ----------------------- Functions ------------------------*/
// Interrupt Handler to read
void PinIsrHandler (void) {
    // Read CONFIG register
    SPIM_WriteTxData(UseCommand(R_REGISTER, CONFIG, 0));
    SPIM_2_WriteTxData(UseCommand(R_REGISTER, CONFIG, 0));
    CyDelay(10);
    // Read DYNPD register
    SPIM_WriteTxData(UseCommand(R_REGISTER, DYNPD, 0));
    SPIM_2_WriteTxData(UseCommand(R_REGISTER, DYNPD, 0));
    CyDelay(10);
    // Read FEATURE register
    SPIM_WriteTxData(UseCommand(R_REGISTER, FEATURE, 0));
    SPIM_2_WriteTxData(UseCommand(R_REGISTER, FEATURE, 0));
    CyDelay(10);
}
// Interrupt Handler to write
void PinSendIsrHandler (void) {
    // --------------------- Receiver ------------------------ //
    // Power up
    SPIM_WriteTxData(UseCommand(W_REGISTER, CONFIG, (1<<EN_CRC)|(1<<PWR_UP)));
    CyDelay(10);    
    // Set to Rx
    SPIM_WriteTxData(UseCommand(W_REGISTER, CONFIG, (1<<EN_CRC)|(1<<PWR_UP)|(1<<PRIM_RX)));
    CyDelay(10);      
    // Set as Dynamic Payload pipes
    SPIM_WriteTxData(UseCommand(W_REGISTER, DYNPD, 0b00111111));
    CyDelay(10);
    // Set as Dynamic Payload
    SPIM_WriteTxData(UseCommand(W_REGISTER, FEATURE, (1<<EN_DPL)));
    CyDelay(10);
    // Set to pipe 0
    SPIM_WriteTxData(UseCommand(W_REGISTER, EN_RXADDR, (1<<0)));
    CyDelay(10);

    // ------------------- Transmitter ---------------------- //
    // Power up
    SPIM_2_WriteTxData(UseCommand(W_REGISTER, CONFIG, (1<<EN_CRC)|(1<<PWR_UP)));
    CyDelay(10);    
    // Set as Dynamic Payload pipes
    SPIM_2_WriteTxData(UseCommand(W_REGISTER, DYNPD, 0b00111111));
    CyDelay(10);
    // Set as Dynamic Payload
    SPIM_2_WriteTxData(UseCommand(W_REGISTER, FEATURE, (1<<EN_DPL)));
    CyDelay(2);
}
word UseCommand(char command, char parameter, unsigned int data){
    word output;
    // Command with address as parameter
    if (command == R_REGISTER || command == W_REGISTER) {
        output = (((command<<5 )+ parameter)<<8) +data;
    } 
    // Command without address as parameter
    else {
        output = (command<<8) + data;
    }
    return output;
}

void PinTxIsrHandler (void) {
    uint8 data = 0xbc;
    // Send data
    SPIM_2_WriteTxData(UseCommand(W_TX_PAYLOAD, 0, data));
    CyDelay(10);
    // Receive data
    SPIM_WriteTxData(UseCommand(R_RX_PAYLOAD,0,0));
    CyDelay(10);
}
/* [] END OF FILE */
