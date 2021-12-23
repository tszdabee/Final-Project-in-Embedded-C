#include <xc.h>
#include "lights.h"

/************************************************
Function to initialise buggy LED's
 * Inpute: None
 * Output: None
 * Functions called: None
 ***********************************************/
void lights_init(void)
{   
    //Initialize pins for car LEDs as outputs
    TRISDbits.TRISD3 = 0; // Main Beam (Full brightness)
    TRISDbits.TRISD4 = 0; // Brake (Full brightness)
    TRISFbits.TRISF0 = 0; // Turn Left
    TRISHbits.TRISH0 = 0; // Turn Right
    TRISHbits.TRISH1 = 0; // Headlamps (Front + Rear at reduced brightness)
    
    // Set initial pin states for car LEDs
    LATDbits.LATD3 = 0; // Main Beam (Full brightness)
    LATDbits.LATD4 = 0; // Brake (Full brightness)
    LATFbits.LATF0 = 0; // Turn Left
    LATHbits.LATH0 = 0; // Turn Right
    LATHbits.LATH1 = 0; // Headlamps (Front + Rear at reduced brightness)
    
    // Initialize pins for RGB
    TRISGbits.TRISG1 = 0; // Red LED
    TRISAbits.TRISA4 = 0; // Green LED
    TRISFbits.TRISF7 = 0; // Blue LED
    
    // Set initial pin states for RGB
    LATGbits.LATG1 = 1; // Red LED
    LATAbits.LATA4 = 1; // Green LED
    LATFbits.LATF7 = 1; // Blue LED
}
