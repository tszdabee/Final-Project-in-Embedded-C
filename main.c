// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

//Required include statements for .h files
#include <xc.h>
#include <stdio.h>
#include "dc_motor.h"
#include "color.h"
#include "lights.h"
#include "serial.h"
#include "interrupts.h"
#include "string.h"


#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  
#define turn90left 55 //Define the delay time for a 90 degree left turn
#define turn90right 40 //Define the delay time for a 90 degree right turn
#define turn180left 270 //Define the delay time for a 180 degree turn
#define turn135right 120  //Define the delay time for a 135 degree right turn
#define turn135left 120  //Define the delay time for a 135 degree left turn
#define PWMcycle 199
volatile unsigned int check = 0; // Interrupt flag to trigger color detection routine

void main(void){
    color_click_init(); // Initialize color click 2
    initDCmotorsPWM(PWMcycle); // Initialize PWM
    lights_init(); // Initialize LEDs on buggy
    initUSART4(); // Initialize USART
    interrupts_master_init(); // Initialize the master device interrupts (clicker 2)
    interrupts_slave_init(); // Initialize the slave device interrupts (color click)

    //Declare two DC_motor structures 
    struct DC_motor motorL, motorR; 		
    //Left motor
    motorL.power=0; 						//zero power to start
    motorL.direction=0; 					//set default motor direction
    motorL.dutyHighByte=(unsigned char *)(&PWM6DCH);	//store address of PWM duty high byte
    motorL.dir_LAT=(unsigned char *)(&LATE); 		//store address of LAT in E
    motorL.dir_pin=4; 						//pin RE4 controls direction for motorL
    motorL.PWMperiod=PWMcycle;              //store PWMperiod for motor
    //Right motor
    motorR.power=0;                         //zero power to start
    motorR.direction=0;                     //set default motor direction
    motorR.dutyHighByte=(unsigned char *)(&PWM7DCH);    //store address of PWM duty high byte
    motorR.dir_LAT=(unsigned char *)(&LATG);        //store address of LAT in C
    motorR.dir_pin=6;                       // pin RC6 controls direction for motorR
    motorR.PWMperiod=PWMcycle;              //store PWMperiod for motor
   
    // Declare structure for the measured RGB values and the calibration values
    struct RGB_val rgb;
    // Assigning calibration values for black and white at clear threshold
    rgb.W_R = 950;
    rgb.W_G = 620;
    rgb.W_B = 470;
    rgb.B_R = 500;
    rgb.B_G = 300;
    rgb.B_B = 220;
 
    // Structure for the path memory
    struct Memory m; //Structure to store the path that the buggy took
    
    //Clearing the path memory arrays
    memset(m.time_forward, 0, sizeof(m.time_forward));
    memset(m.turn, 0, sizeof(m.turn));
    
    //Initializing the debugging LED
    LATHbits.LATH3 = 0;
    TRISHbits.TRISH3 = 0;
    
    char msg[40]; //Create msg array for sending serial output
    int step=0; //Create a step vairable for incrementing the position in path memory arrays 
    
    while(1){
        
        LATDbits.LATD4 = 1;
        //sprintf(msg,"%d %d %s \n",step,m.time_forward[step-1],m.turn); // Uncomment to only display Forward and Turns to send to realterm display
        sprintf(msg,"%.02f %.02f %.02f %.02f %.02f %d %s\n",rgb.R,rgb.G,rgb.B,rgb.C,rgb.hue,m.time_forward[step-1],m.turn); // Combine RGBC values, Hue and Forward and Turns to send to realterm display
        sendStringSerial4(msg); // Send RGB string reading to realterm
        sendTxBuf(); // Interrupt flag to start transmit process
        __delay_ms(5); // 1.53ms Execution time
        LATDbits.LATD4 = 0;
        
        fullSpeedAhead(&motorL,&motorR); // Move buggy forwards
        m.time_forward[step] = m.time_forward[step] + 1; // Counter for time spent moving forwards
        
        if(check) // If the clear light threshold is exceeded (An obstacle is detected)
        {
            stop(&motorL,&motorR);  //Stopping the buggy
            fullSpeedBack(&motorL,&motorR);     //Make buggy drive backwards
            __delay_ms(30);                     //Drive backwards for this amount of time
            color_read_RGB(&rgb);   // Update RGB values
            calibrate_RGB(&rgb);    // Calibrate RGB values
            RGB_to_Hue(&rgb);       // Convert RGB to hue
            __delay_ms(30);

            m.time_forward[step] =  m.time_forward[step] - 160; // Correcting for the time driven backwards
            stop(&motorL,&motorR);  // Stopping the buggy
            
            if(rgb.max - rgb.min < 30) //if white or light blue is registered...
            {
                if(rgb.hue > 230 || rgb.hue < 150) // if white is registered...
                {
                retrace(&m,&motorL,&motorR,step);   //Retrace the path of the buggy
                step = 0;                           //Set the step count to zero
                stop(&motorL,&motorR);              //Stopping the buggy
                __delay_ms(1000);
                }else                               //if light blue is registered...
                {
                    turnLeft(&motorL,&motorR);     // Turn by 135 degrees to the right 
                    __delay_ms(turn135left);
                    stop(&motorL,&motorR);          //Stopping the buggy
                    m.turn[step] = 'b';             //Add b to the turn memory array
                }    
            }
            else if(340<=rgb.hue && rgb.hue<=360)   //if pink/red/orange is registered...
            {
                if(rgb.G > 60 && rgb.B > 60)        // If pink is registered...
                {
                    fullSpeedBack(&motorL,&motorR); //Make buggy drive backwards
                    __delay_ms(1200);               //Drive backwards for this amount of time
                    turnLeft(&motorL,&motorR);     //Turn by 90 degrees to the left
                    __delay_ms(turn90left);
                    stop(&motorL,&motorR);
                    m.time_forward[step] = m.time_forward[step] - 110; //Cutting off the "dead end" from memory
                    m.turn[step] = 'P';             //Add P to the turn memory array      
                }
                else if(rgb.R > 175 && rgb.G<75)    //if red is registered...
                {
                    turnRight(&motorL,&motorR);     // Turn by 90 degrees to the right
                    __delay_ms(turn90right);
                    stop(&motorL,&motorR);          //Stopping the buggy
                    m.turn[step] = 'R';             //Add R to the turn memory array
                }else //If orange is registered
                { 
                    turnRight(&motorL,&motorR);     // Turn by 135 degrees to the right
                    __delay_ms(turn135right);
                    stop(&motorL,&motorR);          //Stopping the buggy
                    m.turn[step] = 'O';             //Add O to the turn memory array
                }
            }
            else if(120<=rgb.hue && rgb.hue<=160)   //If green is registered...
            {
                turnLeft(&motorL,&motorR);          // Turn by 90 degrees to the left
                __delay_ms(turn90left);
                stop(&motorL,&motorR);              //Stopping the buggy
                m.turn[step] = 'G';                 //Add G to the turn memory array
                
            }   
            else if(165<=rgb.hue && rgb.hue<=190)   //If blue is registered...
            {
                turnLeft(&motorL,&motorR);          // Turn by 180 degrees
                __delay_ms(turn180left);
                stop(&motorL,&motorR);              // Stopping the buggy
                m.turn[step] = 'B';                 //Add B to the turn memory array
                
            }
            else if(0<=rgb.hue && rgb.hue<=50)      // If yellow is registered...
            {
                fullSpeedBack(&motorL,&motorR);     //Make buggy drive backwards
                __delay_ms(1200); 
                turnRight(&motorL,&motorR);         // Turn by 90 degrees to the right
                __delay_ms(turn90right);
                stop(&motorL,&motorR);              // Stopping the buggy
                m.time_forward[step] = m.time_forward[step] - 110; //Cutting off the "dead end" from memory
                m.turn[step] = 'Y';                 //Add Y to the turn memory array
            }
            else // If black is detected or unidentified colour, return back to starting position
            {
                retrace(&m,&motorL,&motorR,step);   //Retrace the path of the buggy
                step = 0;                           //Set the step count to zero
                stop(&motorL,&motorR);              //Stopping the buggy
                __delay_ms(1000);
            }
            
            step = step + 1;   //Increment the step count for the memory arrays
            check = 0;         //Clear the check flag
        }
    }
}   
