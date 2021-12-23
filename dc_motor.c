#include <xc.h>
#include "dc_motor.h"
#include "string.h"

#define turn90left 55 //Define the delay time for a 90 degree left turn
#define turn90right 40 //Define the delay time for a 90 degree right turn
#define turn180left 270 //Define the delay time for a 180 degree left turn
#define turn135right 120 //Define the delay time for a 135 degree right turn
#define turn135left 120 //Define the delay time for a 135 degree left turn

/************************************
 * Function to initialise Timer2 and PWM for DC motor control
 * Inputs: Pulse Width Modulated signal period length in ms
 * Outputs: None
 * Functions called within: None
************************************/
void initDCmotorsPWM(int PWMperiod){
	//initialise your TRIS and LAT registers for PWM
    TRISEbits.TRISE4 = 0; 
    TRISGbits.TRISG6 = 0; 
    TRISCbits.TRISC7 = 0;
    TRISEbits.TRISE2 = 0;
  
    LATEbits.LATE4 = 0;
    LATGbits.LATG6 = 0;
    LATEbits.LATE2 = 0;
    LATCbits.LATC7 = 0;

    
    // timer 2 config
    T2CONbits.CKPS=0b0011; // 1:8 prescaler. Calculated 6.22 but taking larger prescaler for longer overflow.
    T2HLTbits.MODE=0b00000; // Free Running Mode, software gate only
    T2CLKCONbits.CS=0b0001; // Fosc/4

    // Tpwm*(Fosc/4)/prescaler - 1 = PTPER
    T2PR= PWMperiod; //Period reg 10kHz base period. Calculated with a PS = 8. Timer2 count required for overflow. (was 199 bef changed to PWMperiod))
    T2CONbits.ON=1;
    
    RE2PPS=0x0A; //PWM6 on RE2
    RC7PPS=0x0B; //PMW7 on RC7

    PWM6DCH=0; //0% power. Send frac value out of 199 for %.
    PWM7DCH=0; //0% power. Send frac value out of 199 for %.
    
    PWM6CONbits.EN = 1;
    PWM7CONbits.EN = 1;
}

/************************************
 * Function to set PWM output from the values in the motor structure 
 * and thus control motor power and movement
 * Inputs: DC_motor structure and pointer "m"
 * Outputs: None
 * Functions called within: None
************************************/
void setMotorPWM(struct DC_motor *m)
{
	int PWMduty; //tmp variable to store PWM duty cycle

	if (m->direction){ //if forward
		// low time increases with power
		PWMduty=m->PWMperiod - ((int)(m->power)*(m->PWMperiod))/100;
	}
	else { //if reverse
		// high time increases with power 
		PWMduty=((int)(m->power)*(m->PWMperiod))/100;
	}

	*(m->dutyHighByte) = PWMduty; //set high duty cycle byte 
        
	if (m->direction){ // if direction is high
		*(m->dir_LAT) = *(m->dir_LAT) | (1<<(m->dir_pin)); // set dir_pin bit in LAT to high without changing other bits
	} else {
		*(m->dir_LAT) = *(m->dir_LAT) & (~(1<<(m->dir_pin))); // set dir_pin bit in LAT to low without changing other bits
	}
}

/************************************
 * Function to stop the DC Motor gradually
 * Inputs: DC_motor structure and pointer for the left motor and the right motor.
 * Outputs: None
 * Functions called within: The function to set PWM output from the values in the motor structure is called
************************************/
void stop(struct DC_motor *mL, struct DC_motor *mR)
{
    while(((mL->power) != 0) || ((mR->power) != 0)) // While power is not 0
    {
        if (mL->power>0) {mL->power--;} // Decrement left motor power by 1
        if (mR->power>0) {mR->power--;} // Decrement right motor power by 1
        setMotorPWM(mL); // Apply power changes to left motor
        setMotorPWM(mR); // Apply power changes to right motor
        __delay_ms(5); // Execution time to allow for gradual change
    }
}

/************************************
 * Function to make the buggy turn left abruptly (for better contorl)
 * Inputs: DC_motor structure and pointer for the left motor and the right motor.
 * Outputs: None
 * Functions called within: The function to set PWM output from the values in the motor structure is called
************************************/
void turnLeft(struct DC_motor *mL, struct DC_motor *mR)
{
    stop(mL,mR);
    // Set turning direction to left
    mL->direction = 1;
    mR->direction = 0;
    // Set full power for left turn
    mL->power = 100;
    mR->power = 100;
    setMotorPWM(mL); // Apply power changes to left motor
    setMotorPWM(mR); // Apply power changes to right motor
    __delay_ms(10); // Execution time
}

/************************************
 * Function to make the buggy turn right abruptly (for better contorl)
 * Inputs: DC_motor structure and pointer for the left motor and the right motor.
 * Outputs: None
 * Functions called within: The function to set PWM output from the values in the motor structure is called
************************************/
void turnRight(struct DC_motor *mL, struct DC_motor *mR)
{
    stop(mL,mR);
    // Set turning direction to right
    mL->direction = 0;
    mR->direction = 1;
    // Set full power for right turn
    mL->power = 100;
    mR->power = 100;
    setMotorPWM(mL); // Apply power changes to left motor
    setMotorPWM(mR); // Apply power changes to right motor
    __delay_ms(10); // Execution time
   
    
}

/************************************
 * Function to make the buggy go forward 
 * Inputs: DC_motor structure and pointer for the left motor and the right motor.
 * Outputs: None
 * Functions called within: The function to set PWM output from the values in the motor structure is called
************************************/
void fullSpeedAhead(struct DC_motor *mL, struct DC_motor *mR)
{
    // Set direction to forward
    mL->direction = 0;
    mR->direction = 0;
    while(mL->power<50 || mR->power<50) // While power is not 50, max forward power limit is 50
    {
        if (mL->power<50) {mL->power++;} // Increment left motor power by 1
        if (mR->power<50) {mR->power++;} // Increment right motor power by 1
    setMotorPWM(mL); // Apply power changes to left motor
    setMotorPWM(mR); // Apply power changes to right motor
    __delay_ms(5); // Execution time to allow for gradual change
    }  
}

/************************************
 * Function to make the buggy go backwards 
 * Inputs: DC_motor structure and pointer for the left motor and the right motor.
 * Outputs: None
 * Functions called within: The function to set PWM output from the values in the motor structure is called
************************************/
void fullSpeedBack(struct DC_motor *mL, struct DC_motor *mR)
{
    // Set direction to backwards
    mL->direction = 1;
    mR->direction = 1;
    while(mL->power<50 || mR->power<50){ // While power is not 50, max backward power limit is 50
        if (mL->power<50) {mL->power++;} // Increment left motor power by 1
        if (mR->power<50) {mR->power++;} // Increment right motor power by 1
    setMotorPWM(mL); // Apply power changes to left motor
    setMotorPWM(mR); // Apply power changes to right motor
    __delay_ms(10); // Execution time to allow for gradual change
    }  
}

/************************************
 * Function to make the buggy go retrace its steps
 * Inputs: The Memory structure and pointer m that can be used to access the memory 
 * array for the time driven forwards and the types of turns made. Also the DC motor structures and pointers as well
 * as the count for the steps made by the buggy. 
 * Outputs: None
 * Functions called within: The function to execute the appropriate motor functions outlined above and 
 * to clear the path memory arrays
************************************/
void retrace(struct Memory *m,struct DC_motor *motorL, struct DC_motor *motorR, int step)
{      
    LATHbits.LATH3 = 1;  //Turn on LED that signifies retrace
    turnLeft(motorL,motorR); // Turn by 180 degrees
    __delay_ms(turn180left);
    stop(motorL,motorR); // Stop buggy
    __delay_ms(500);
    
    //We trace back a distance driven once before we enter the while loop that follows, because there is one more
    //distance driven compared to the number of turns.
    while(m->time_forward[step]>0) // While the counter value recoded for the step is greater than 0
    {
        fullSpeedAhead(motorL,motorR); // Driving full speed ahead
        __delay_ms(12); //the same delay as in between the count increments in main
        m->time_forward[step]--;
    }
    step--; // decrement the step to go through the memory arrays
            
    while(step>=0)
    {
        //Vary lights before every remembered turn, not used for measurement purposes, purely aesthetic!
        LATGbits.LATG1 = 1; // Red LED
        LATAbits.LATA4 = 0; // Green LED
        LATFbits.LATF7 = 0; // Blue LED
        __delay_ms(200);
        LATGbits.LATG1 = 0; // Red LED
        LATAbits.LATA4 = 1; // Green LED
        LATFbits.LATF7 = 0; // Blue LED
        __delay_ms(200);
        LATGbits.LATG1 = 0; // Red LED
        LATAbits.LATA4 = 0; // Green LED
        LATFbits.LATF7 = 1; // Blue LED
        __delay_ms(200);
        LATGbits.LATG1 = 0; // Red LED
        LATAbits.LATA4 = 0; // Green LED
        LATFbits.LATF7 = 0; // Blue LED
        
        __delay_ms(200);
        if(m->turn[step] == 'R') //If red remembered...
        {
            //Undo a red turn
            turnLeft(motorL,motorR); // Turn by 90 degrees to the left
            __delay_ms(turn90left);
        }
        else if(m->turn[step] == 'G') //If green remembered...
        {
            //undo a green turn
            turnRight(motorL,motorR); // Turn by 90 degrees to the right
            __delay_ms(turn90right);
        }
        else if(m->turn[step] == 'B') //If blue remembered...
        {
            //undo a blue turn
            turnLeft(motorL,motorR); // Turn by 90 degrees to the right
            __delay_ms(turn180left);
        }
        else if(m->turn[step] == 'Y') //If yellow remembered...
        {
            //undo a yellow turn
            turnLeft(motorL,motorR);         // Turn by 90 degrees to the left
            __delay_ms(turn90right);
        }
        else if(m->turn[step] == 'P') //If pink remembered...
        {
            //undo a pink turn
            turnRight(motorL,motorR);     //Turn by 90 degrees to the right
            __delay_ms(turn90left);
        }
        else if(m->turn[step] == 'O') //If orange remembered...
        {
            //undo an orange turn
            turnLeft(motorL,&motorR);     // Turn by 135 degrees to the left
            __delay_ms(turn135left);
        }
        else if(m->turn[step] == 'b') //If blue remembered...
        {
            //undo a light blue turn
            turnRight(motorL,motorR);     // Turn by 135 degrees to the right
            __delay_ms(turn135right);
        }
        
        stop(motorL,motorR);
        __delay_ms(500);
        
        //Drive back the remembered distance
        while(m->time_forward[step]>0) // While the counter value recoded for the step is greater than 0
        {
            fullSpeedAhead(motorL,motorR); //Buggy will drive full speed ahead
            __delay_ms(12); //the same delay as in between the count increments in main
            m->time_forward[step]--;
        }
        step--; // we decrement the step to go through the memory arrays
    }
    //Clearing the memory arrays for a new path memory to be stored
    memset(m->time_forward, 0, sizeof(m->time_forward));
    memset(m->turn, 0, sizeof(m->turn));
    LATHbits.LATH3 = 0;                 //Turn off LED that signifies retrace
}