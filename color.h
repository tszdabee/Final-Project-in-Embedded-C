#ifndef _color_H
#define _color_H

#include <xc.h>

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  

struct RGB_val //Defining the RGB value structure
{ 
    //Variables to store integer values returned for each colour by color_read_RGB
	float R, G, B, C, W_R, W_G, W_B, B_R, B_G, B_B, hue, max, min; //Read, Green, Blue, Clear then all the calibration values
};

//function prototypes (Function descriptions are to be found in the .c file)
void color_click_init(void);
void color_writetoaddr(char address, char value);
void color_read_RGB(struct RGB_val *rgb);
void calibrate_RGB(struct RGB_val *rgb);
void RGB_to_Hue(struct RGB_val *rgb);

#endif