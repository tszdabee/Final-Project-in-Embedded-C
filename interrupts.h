#ifndef _interrupts_H
#define _interrupts_H

#include <xc.h>

#define _XTAL_FREQ 64000000

//function prototypes (Function descriptions are to be found in the .c file)
void Interrupts_init(void);
void __interrupt(high_priority) HighISR();
void interrupts_slave_init(void);
void interrupts_master_init(void);
void interrupt_clear(void);
void __interrupt(low_priority) LowISR();

#endif
