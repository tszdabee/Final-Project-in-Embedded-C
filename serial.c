#include <xc.h>
#include "serial.h"

/************************************************
Function to initialise USART
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void initUSART4(void) {
    // Configure pins RC0 and RC1 to map to EUSART module
    TRISCbits.TRISC1=1; 
	RC0PPS = 0x12; // Map EUSART4 TX to RC0
    RX4PPS = 0x11; // RX is RC1   
    
    // Set up EUSART to asyncronous baud rate 9600 using 8 bit baud rate generator value 
    BAUD4CONbits.BRG16 = 0; 	//set baud rate scaling
    TX4STAbits.BRGH = 0; 		//high baud rate select bit
    SP4BRGL = 51; 			//set baud rate to 51 = 19200bps
    SP4BRGH = 0;			//not used

    RC4STAbits.CREN = 1; 		//enable continuos reception
    TX4STAbits.TXEN = 1; 		//enable transmitter
    RC4STAbits.SPEN = 1; 		//enable serial port
}
/************************************************
Function to wait for a byte to arrive on serial port and read it once it does 
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
char getCharSerial4(void) {
	while (!PIR4bits.RC4IF);//wait for the data to arrive
	return RC4REG; //return byte in RCREG
}

/************************************************
Function to check the TX reg is free and send a byte 
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void sendCharSerial4(char charToSend) {
    while (!PIR4bits.TX4IF); // wait for flag to be set
    TX4REG = charToSend; //transfer char to transmitter
}

/************************************************
Function to send a string over the serial interface
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void sendStringSerial4(char *string){
    while(*string != 0){ // Send string using pointers and sendchar functions
        sendCharSerial4(*string++);
    }
}

/************************************************
// Function to retrieve a byte from the circular buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
char getCharFromRxBuf(void){
    if (RxBufReadCnt>=RX_BUF_SIZE) {RxBufReadCnt=0;} 
    return EUSART4RXbuf[RxBufReadCnt++];
}

/************************************************
// Function to add a byte to the buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void putCharToRxBuf(char byte){
    if (RxBufWriteCnt>=RX_BUF_SIZE) {RxBufWriteCnt=0;}
    EUSART4RXbuf[RxBufWriteCnt++]=byte;
}

/************************************************
// Function to check if there is data in the RX buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
char isDataInRxBuf (void){
    return (RxBufWriteCnt!=RxBufReadCnt);
}

/************************************************
// Function to retrieve a byte from the TX buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
char getCharFromTxBuf(void){
    if (TxBufReadCnt>=TX_BUF_SIZE) {TxBufReadCnt=0;} 
    return EUSART4TXbuf[TxBufReadCnt++];
}

/************************************************
// Function to add a byte to the TX buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void putCharToTxBuf(char byte){
    if (TxBufWriteCnt>=TX_BUF_SIZE) {TxBufWriteCnt=0;}
    EUSART4TXbuf[TxBufWriteCnt++]=byte;
}

/************************************************
// Function to check if there is data in the TX buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
char isDataInTxBuf (void){
    return (TxBufWriteCnt!=TxBufReadCnt);
}

/************************************************
// Function to add a string to the buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void TxBufferedString(char *string){
    while(*string != 0){
        putCharToTxBuf(*string++); // Go through string to send individual characters to transmit buffer
    }
}

/************************************************
// Function to initialise interrupt driven transmission of the Tx buf
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void sendTxBuf(void){
    if (isDataInTxBuf()) {PIE4bits.TX4IE=1;} //enable the TX interrupt to send data
}