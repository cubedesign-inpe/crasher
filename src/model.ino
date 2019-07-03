
#include "model.h"

byte Model;

byte GetModel(void)
{
    byte ret;

    // Logic is: Set PD6 pullup and PD5 to LOW
    // If PD6 is read HIGH, jumper is open, if PD6 is read LOW, jumper is set.
    
    // INPUT and PULL-UP PORTD.6
    DDRD  &= ~(1<<6);
    PORTD |=  (1<<6);

    // OUTPUT and LOW PORTD.5
    DDRD  |=  (1<<5);
    PORTD &= ~(1<<5);

    delay(1);

    // read pin
    // Still HIGH? CANSAT model
    if(PIND & (1<<6)) ret = MODEL_CANSAT;

    // PIN is low? MOCKUP model
    else ret = MODEL_MOCKUP;

    // disable pull-up
    PORTD &= ~(1<<6);

    // disable output
    DDRD  &= ~(1<<5);

    return(ret);
}


