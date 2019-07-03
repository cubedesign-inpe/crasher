
#ifndef BUTTONS_H
#define BUTTONS_H

// Buttons
#define BTN_0       (1<<2)
#define BTN_1       (1<<3)

// Functions
void BtnInit(void);
void BtnLoop(void);

// Extern event function
extern void BtnCallback(byte Falling, byte Rising);

#endif

