#include <xc.h>
#include "interrupts.h"
#include "serial.h"
#include "color.h"
#include "i2c.h"

// Declare external variable for use in the ISR
extern unsigned int check;

/************************************
 * High priority interrupt service routine to handle the receiving and transmitting data
 * Input: none
 * Output: none
 * Functions called: The function to check if data is in the transmit buffer and 
 * the function to get the stored characters from the transmit buffer is called.
************************************/
void __interrupt(high_priority) HighISR()
{
	if(PIR4bits.RC4IF) // If recieve register is flagged
    {
        putCharToRxBuf(RC4REG);  //Put byte in register to recieve buffer. return byte in RCREG. clear RC4IF by reading the data in RC4REG.
    }
    if(PIR4bits.TX4IF){ // If something transmitted
        if(isDataInTxBuf()){ // If data in transmit buffer
            TX4REG = getCharFromTxBuf(); // Set register to transmitted characters in transmit buffer
        }else{
            PIE4bits.TX4IE = 0; // Reset Flag, the TX4IF for the transmit register is set whenever the TX4REG is empty
        }
    }
}

/************************************
 * Function to initialize the interrupts on the slave device (clicker 2)
 * Inputs: None
 * Outputs: None
 * Functions called within: color_writetoaddr() is called to write values to the appropriate 
 * registers for initializing the interrupts on the color click.
************************************/
void interrupts_slave_init()
{
    //(AIEN) RGBC interrupt enable. When asserted, permits RGBC interrupts to be generated. Bit 5 in the message sent. Currently ON
    color_writetoaddr(0x00,0b10011); 
    color_writetoaddr(0x0C, 0b011); //Set persistence value to 3 
    interrupt_clear();
    
    //A low threshold and high threshold for the clear light value must be set. The interrupt is triggered when the light level falls outside of this range. 
    //Setting clear light low threshold lower byte 
    color_writetoaddr(0x04,0b00000000); 
    //Setting clear light low threshold higher byte 
    color_writetoaddr(0x05,0b00000000);
    //Setting clear light high threshold lower byte 
    color_writetoaddr(0x06,0b11011100);
    //Setting clear light high threshold higher byte
    color_writetoaddr(0x07,0b00000101);     	//1500 
    //Also add you battery monitoring so that the car turns around when its at 50% of it's starting value 
}

/************************************
 * Function to initialize the interrupts on the master device (clicker 2)
 * Inputs: None
 * Outputs: None
 * Functions called within: None
************************************/
void interrupts_master_init()
{
    // Turn on Global Interrupts, Peripheral Interrupts, and Interrupt Source (Turn on Global last)
    PIE4bits.RC4IE=1;	//receive interrupt
    //transmit interrupt (only turn on when you have more than one byte to send)
    INTCONbits.IPEN = 1; // Enaable priority levels on interrupt
    INTCONbits.GIEL = 1; //Enable peripheral interrupt
    INTCONbits.GIEH = 1; // Enable global interrupt
    
    //Enabling external interrupts on the clicker board
    PIE0bits.INT1IE = 1; //Enabling interrupt INT0
    IPR0bits.INT1IP = 0; //Setting interrupt priority high (1) or low (0)
    INTCONbits.GIE = 1; //Enabling global interrupts to have the processor branch to the interrupt vector following wakeup
    INTCONbits.INT1EDG = 0; //Interrupt set to occur on a falling edge because the the interrupt on the TCS causes the pin to go low
    INT1PPS = 0b001001; //Setting RB1 as the interrupt pin (it is by default, just to be sure)
    
    //Initializing the external interrupt pin on the board
    TRISBbits.TRISB1 = 1;
    ANSELBbits.ANSELB1=0;
}

/************************************
 * Low priority interrupt service routine to handle setting the check value high if the clear light value 
 * falls outside of the set threshold 
 * Inputs: None
 * Outputs: None
 * Functions called within: The function to clear the interrupt flag in the color click is called
************************************/
void __interrupt(low_priority) LowISR()
{
    if(PIR0bits.INT1IF)                     //check the interrupt source
    {                                       
        check = 1; // Trigger colour detection routine in main.c
        LATHbits.LATH3 = !LATHbits.LATH3;
        __delay_ms(10);
        interrupt_clear();//clear the interrupt flag in the slave
        PIR0bits.INT1IF = 0;               //clear the interrupt flag in the master                     
	}
}

/************************************
 * Function to clear the interrupt flag on the 
 * Inputs: None
 * Outputs: None
 * Functions called within: The function sequence for sending a command to the color click 
 * via I2C communication is called
************************************/
void interrupt_clear(void)
{
    // send a command to the TCS to clear the interrupt
    I2C_2_Master_Start();                  //Start condition
    I2C_2_Master_Write(0x52 | 0x00);       //7 bit device address + Write mode
    I2C_2_Master_Write(0b11100110);    //command + register address    
    I2C_2_Master_Stop();
}

