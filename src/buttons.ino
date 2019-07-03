
#include "buttons.h"

// Button states
byte Buttons;
byte ButtonsOld;

// Button changes
byte Rising;
byte Falling;


void BtnInit(void)
{
    // Set buttons as INPUT
    DDRD &= ~(BTN_0 | BTN_1);

    // Set buttons pull-up
    PORTD |= (BTN_0 | BTN_1);

    // variables
    Buttons    = 0; 
    ButtonsOld = 0;
}


void BtnLoop(void)
{
    static long TimerBtn = 1000;
    
    // ready?
    if(millis() < TimerBtn)
    {
        // no
        return;
    }

    
    // read in 10 ms
    TimerBtn += 10;

    // save older state
    ButtonsOld = Buttons;

    // read all buttons in PINS D
    Buttons = ~PIND;

    // get edges
    Falling = Buttons & ~ButtonsOld; // is 1, was 0
    Rising = ~Buttons &  ButtonsOld; // is 0, was 1

    // do callback (pressed, released)
    BtnCallback(Falling, Rising);
}



