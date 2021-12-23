#include <xc.h>
#include "color.h"
#include "i2c.h"

/************************************
 * Function to perform the initialization of the color click
 * Inputs: None
 * Outputs: None
 * Functions called within:
 * The I2C_2_Master_Init function is called to initialize the I2C communication before
 * the color_writetoaddr() function is called which sends values to appropriate registers to 
 * initialize the color click using I2C communication.
************************************/
void color_click_init(void)
{   
    //setup colour sensor via i2c interface
    I2C_2_Master_Init();      //Initialise i2c Master

     //set device PON
	color_writetoaddr(0x00, 0x01);
    __delay_ms(3); //need to wait 3ms for everthing to start up
    
    //turn on device ADC
	color_writetoaddr(0x00, 0x03);

    //set integration time
	color_writetoaddr(0x01, 0xD5);
}

/************************************
 * Function to write values to associated addresses on the color_click
 * Inputs: Register address, value to be stored
 * Outputs: None
 * Functions called within: The I2C functions to start communication, declare write mode, 
 * declare address to be written to, and write the value to the address are called in that order.
************************************/
void color_writetoaddr(char address, char value){
    I2C_2_Master_Start();         //Start condition
    I2C_2_Master_Write(0x52 | 0x00);     //7 bit device address + Write mode
    I2C_2_Master_Write(0x80 | address);    //command + register address
    I2C_2_Master_Write(value);    
    I2C_2_Master_Stop();          //Stop condition
}

/************************************
 * Function to read sensor values representing color intensity from the color click
 * Inputs: RGB_val structure and pointer rgb
 * Outputs: None
 * Functions called within: I2C functions are called to set up the auto-incremented reading of 
 * the sensor high and low byte registers which are stored in addresses 0x14-0x1B. The
 * read values are stored in the structure RGB_val.
************************************/
void color_read_RGB(struct RGB_val *rgb)
{
	unsigned int tmp=0;
	I2C_2_Master_Start();         //Start condition
	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x14);    //command (auto-increment protocol transaction) + start at Clear low register
	I2C_2_Master_RepStart();			// start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
    tmp=I2C_2_Master_Read(1);			//read the Clear LSB
	tmp=tmp | (I2C_2_Master_Read(1)<<8); //read the Clear MSB 
    rgb->C = tmp; // Write Clear value to structure
	tmp=I2C_2_Master_Read(1);			//read the Red LSB
	tmp=tmp | (I2C_2_Master_Read(1)<<8); //read the Red MSB 
    rgb->R = tmp; // Write Red value to structure
    tmp=I2C_2_Master_Read(1);			//read the Green LSB
	tmp=tmp | (I2C_2_Master_Read(1)<<8); //read the Green MSB 
    rgb->G = tmp; // Write Green value to structure
    tmp=I2C_2_Master_Read(1);			//read the Blue LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8); //read the Blue MSB (don't acknowledge as this is the last read)
   	rgb->B = tmp; // Write Blue value to structure
    I2C_2_Master_Stop();          //Stop condition
}

/************************************
 * Function to convert the values read from the color_click sensors to RGB values ranging (0-255). 
 * red, green, and blue calibration measurements for Black RGB(0,0,0) and white RGB(255,255,255) are interpolated 
 * in between to obtain the 'normalized' RGB values for each color. 
 * Inputs: RGB_val structure and pointer rgb
 * Outputs: None
 * Functions called within: None
************************************/
void calibrate_RGB(struct RGB_val *rgb)
{
    rgb->R = 0 + ((rgb->R - rgb->B_R) * (255 - 0) / (rgb->W_R - rgb->B_R)); // Linear interpolation to calibrate R value to conventional 0,255 RGB scale
    rgb->G = 0 + ((rgb->G - rgb->B_G) * (255 - 0) / (rgb->W_G - rgb->B_G)); // Linear interpolation to calibrate G value to conventional 0,255 RGB scale
    rgb->B = 0 + ((rgb->B - rgb->B_B) * (255 - 0) / (rgb->W_B - rgb->B_B)); // Linear interpolation to calibrate B value to conventional 0,255 RGB scale
}


/************************************
 * Function to convert the normalized RGB values to Hue. 
 * Associates a hue value from 0 to 360 to each color.
 * Hue value is calculated using predefined equations
 * Inputs: RGB_val structure and pointer rgb
 * Outputs: None
 * Functions called within: None
************************************/
void RGB_to_Hue(struct RGB_val *rgb)
{// Different hue equations depending on if R,G or B max
    
    if(rgb->R >= rgb->G && rgb->R >= rgb->B){ // If R largest return R
        rgb->max = rgb->R;
    }else if(rgb->G >= rgb->R && rgb->G >= rgb->B){ // If G largest return G 
        rgb->max = rgb->G;
    }else{ // Else return B
       rgb->max = rgb->B;
    }
    
    if(rgb->R <= rgb->G && rgb->R <= rgb->B){ // If R smallest return R
        rgb->min = rgb->R;
    }else if(rgb->G <= rgb->R && rgb->G <= rgb->B){ // If G smallest return G 
        rgb->min = rgb->G;
    }else{ // Else return B
        rgb->min = rgb->B;
    }
    
    if(rgb->max == rgb->R){ // For R max
        rgb->hue = ((rgb->G - rgb->B) / (rgb->max-rgb->min)) * 60;
    }else if(rgb->max == rgb->G){ // For G max
        rgb->hue = (2 + ((rgb->B - rgb->R) / (rgb->max-rgb->min))) * 60;
    }else{ // For B max
        rgb->hue = (4 + ((rgb->R - rgb->G) / (rgb->max-rgb->min))) * 60;
    }
    
    if(rgb->hue < 0){ // Conversion for hue if negative number
        rgb->hue = 360 + rgb->hue; // Add another cycle to make it positive
    }
}
