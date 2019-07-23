#include "model.h"
#include "sensor.h"
#include "buttons.h"
#include "memory.h"

// Timer rates (in ms)
#define TMR_BLINK       625 // default blink
#define TMR_BLINK_FAST  250 // fast blinking state
#define TMR_ARMED       50  // measurement rate in armed state
#define TMR_FALL        10  // measurement rate during fall (this is the real deal!)

// General timer
long Timer;

// general timeout
long Timeout;

// Timer for G measurement
long TimerG;

// Step timer
long TimerStp;

// Measured G max value
float G_max = 0;
float G_max_2; // not initialized

// Measured Height
float H_base;
float H_max = 0;

// Size of data buffer
#define BUFFER_SZ       EE_BARO_SZ //1100

// Log data for both models
byte Buffer[BUFFER_SZ];

// writing pointer
byte *pBuff;

// used amount
uint16_t BuffSz = 0;

// states
#define ST_IDLE         0x00    // on / idle state
#define ST_HOLD         0x01    // BTN_0 hold detection
#define ST_SEQM         0x02    // Sequency monitor
#define ST_LEDS         0x03    // Show the impact result
#define ST_CLRD         0x04    // Clear Data
#define ST_CHKG         0x05    // Check for gravity
#define ST_BUFF         0x06    // Display buffer and goes back to IDLE
#define ST_PREP         0x07    // Prepare for arm, depending of the model
#define ST_DEAD         0xFF    // sensor failure

#define ST_MOK_ARMD     0x10    // Armed for impact measure
#define ST_MOK_ACCL     0x11    // Read accelerometer

#define ST_CAN_ARMD     0x20    // Armed for launch measure
#define ST_CAN_ACCL     0x21    // Monitor 0G for few seconds
#define ST_CAN_LOGM     0x22    // Start full log

// Events
#define EV_BTN_0        0x01    // Button 0 was pressed
#define EV_BTN_1        0x02    // Button 1 was pressed
#define EV_RLS_0        0x04    // Button 0 was released
#define EV_RLS_1        0x08    // Button 1 was released
#define EV_DEPLOY       0x10    // Falling (for mockup), Launch (cansat)

// Current state
byte State;
byte StateEvt;

// Leds state
byte Leds;

// Arming sequency: click B1 twice
byte SeqArm[] = { EV_BTN_1, EV_BTN_1, 0 };

// Clear sequency:  click B1 then B0, then B1.
byte SeqClr[] = { EV_BTN_1, EV_BTN_0, EV_BTN_1, 0 };

// Buffer sequency:  click B1 then B0, then B0.
byte SeqBuf[] = { EV_BTN_1, EV_BTN_0, EV_BTN_0, 0 };

// Check sequency: B0, B1, B1
byte SeqChk[] = { EV_BTN_0, EV_BTN_1, EV_BTN_1, 0 };

// Test sequency: B0, B1, B0 (Used for whatever test is necessary)
byte SeqTst[] = { EV_BTN_0, EV_BTN_1, EV_BTN_0, 0 };


// All sequencies
byte* SeqPtr[] = { SeqArm,  SeqClr,  SeqBuf,  SeqChk,  SeqTst  };  // pointers
byte  SeqSta[] = { ST_PREP, ST_CLRD, ST_BUFF, ST_CHKG, ST_IDLE };  // state to go

// total of sequencies
#define SEQ_MAX     5

void setup()
{
    // Serial interface
    Serial.begin(38400);
    Serial.println("-- CRASHER -----------");
    Serial.print("Current model: ");

    // Initialize memory
    MemInit();   

    // Setup buttons
    BtnInit();

    // get working model
    Model = GetModel();
    if(Model == MODEL_CANSAT) Serial.print("Cansat\r\n");
    else                      Serial.print("Mockup\r\n");

    // Initialize LEDs
    DDRC |= 0x0F;
    PORTC |= 0x0F;
    delay(500);
    for(int i = 0; i < 4; i++)
    {
        PORTC >>= 1;
        delay(150);
    }

    // Load accelerometer parameters from eeprom
    int16_t x = MemRead16(EE_ACC_BIAS_X);
    int16_t y = MemRead16(EE_ACC_BIAS_Y);
    int16_t z = MemRead16(EE_ACC_BIAS_Z);

    // Set sensor bias
    SensorSetBias(x, y, z);

    // Initialize all sensors
    Leds = SensorInit();

    // check sensor status
    if(Leds) State = ST_DEAD;
    else     State = ST_IDLE;

    Timer = millis() + 300;
    TimerStp = 625;
}
  
void loop() 
{
    // run button task
    BtnLoop();

    // operational states
    switch(State)
    {
        /**********************************************************************************************
         * 
         */
        case ST_IDLE:

            // keep blinking green led
            if(millis() > Timer)
            {
                static byte G_Counter = 16;
                
                // next blink
                Timer += TMR_BLINK;
                
                // Read G force
                float g = SensorG();
                Serial.print("G: ");
                Serial.print(g, 2);
                if(Model == MODEL_CANSAT)
                {
                    float h = SensorH();
                    Serial.print("\tH: ");
                    Serial.print(h, 1);
                }
                Serial.println();

                // blink yelow if 10% off 1G
                if(0.9 > g || g > 1.1) 
                {
                    // Decrement counter
                    if(G_Counter) G_Counter--;
                    else
                    {
                        // It means we are out of 1G for more than 10s
                        // There are two possibilities:
                        // - We are onboard a missile being launched
                        // - There is something wrong with sensor range (Prob default 2G)
                        SensorSetRange(16);
                        Serial.println("\nRe-adjusting sensor scale to 16G!\n");

                        // Reset counter, now for 1 minute
                        G_Counter = 96; // MAX 255!
                    }
                    
                    PORTC |= (1<<2);    // led on
                }
                else G_Counter = 16;
                
                PORTC |= (1<<3);    // led on
                delay(15);          // wait
                PORTC &= ~(3<<2);   // leds off
            }

            // Check for event
            if(StateEvt & EV_BTN_0)
            {
                // clear event
                StateEvt &= ~EV_BTN_0;

                // go to hold state
                State = ST_HOLD;

                // Begin sequence
                //pSeqArm = SeqArm;
                //pSeqClr = SeqClr;

                // Indicates with the yelow led
                PORTC |= 0x04;

                // Set timeout for holding
                Timer = millis() + 1500;
            }

            // Check for event
            if(StateEvt & EV_BTN_1)
            {
                // clear event
                StateEvt &= ~EV_BTN_1;

                // go to show result state
                State = ST_LEDS;

                // Show result
                PORTC &= ~0x0F;
                PORTC |= GetLeds();

                // Set timeout for displaying
                Timer = millis() + 500;
            }
        break;

        /**********************************************************************************************
         * 
         */
        case ST_HOLD:

            // if any event happens
            if(StateEvt & (EV_BTN_0 | EV_BTN_1 | EV_RLS_0 | EV_RLS_1))
            {
                // abort holding, go back to idle
                State = ST_IDLE;

                // Clear the yelow led
                PORTC &= ~0x04;

                // blink within 500ms
                Timer = millis() + 500;
            }

            // If nothing happened and the holding timeouts
            if(millis() > Timer)
            {
                // go to sequency monitor state
                State = ST_SEQM;

                // Clear the yelow led to indicate holding is done
                PORTC &= ~0x04;

                // Reload all sequencies pointers
                // Same as declared on 'SeqPtr'
                SeqPtr[0] = SeqArm;
                SeqPtr[1] = SeqClr;
                SeqPtr[2] = SeqBuf;
                SeqPtr[3] = SeqChk;
                SeqPtr[4] = SeqTst;

                // Timeout is 1 sec
                Timer = millis() + 1000;
            }
        
        break;

        /**********************************************************************************************
         * 
         */
        case ST_SEQM:

            // Time is up?
            if(millis() > Timer)
            {
                // abort sequency, go back to idle
                State = ST_IDLE;

                // Clear the yelow led
                PORTC &= ~0x04;

                // blink within 500ms
                Timer = millis() + 500;                
            }

            // if any event happens
            if(StateEvt & (EV_BTN_0 | EV_BTN_1))
            {                
                for(byte i = 0; i < SEQ_MAX; i++)
                {
                    // Match with Arming sequency?
                    if(SeqPtr[i] && (StateEvt & *SeqPtr[i]) == *SeqPtr[i])
                    {
                        // next
                        SeqPtr[i]++;
    
                        // over? 
                        if(!*SeqPtr[i]) 
                        {
                            // Now go to sequence defined state
                            State = SeqSta[i];
    
                            // Timer to read G sensor, if needed...
                            TimerG = millis() + 50;

                            // general timeout for actions
                            Timeout = millis() + 5000;
                        }
                    }
                    else
                    {
                        // oops, not this sequence
                        SeqPtr[i] = NULL;
                    }
                }

                // reset timeout to 1s
                Timer = millis() + 1000;
            }
        
        break;


        /**********************************************************************************************
         * 
         */
        case ST_CLRD:

            // keep blinking red led, quickly
            if(millis() > Timer)
            {
                Timer += TMR_BLINK;
                PORTC |= 0x01;    // led on
                delay(15);          // wait
                PORTC &= ~0x01;   // led off
            }


            // button 1 pressed?
            if(StateEvt & EV_BTN_1)
            {
                // Clear memory
                MemWrite8(EE_LEDS, 0x00);
                MemWrite8(EE_MAXG, 0x00);
                MemWrite8(EE_BARO, 0x00);

                // imply a timeout
                Timeout = 0L;             
            }


            // timed out?
            if(millis() > Timeout)
            {
                // go back to idle
                State = ST_IDLE;

                // Clear all leds
                PORTC &= ~0x0F;

                // blink within 500ms
                Timer = millis() + 500;    
            }

        break;



        /**********************************************************************************************
         * 
         */
        case ST_PREP:

            // check model
            if(Model == MODEL_CANSAT)
            {


                // Goes to Cansat armed state
                State = ST_CAN_ARMD;
            }
            else
            {

                // goes to MOCKUP armed state
                State = ST_MOK_ARMD;
            }

        break;

        /**********************************************************************************************
         * 
         */
        case ST_MOK_ARMD:

            // keep blinking yelow led, slowly
            if(millis() > Timer)
            {
                Timer += TMR_BLINK;
                PORTC |= 0x04;    // led on
                delay(15);          // wait
                PORTC &= ~0x04;   // led off
            }

            // Time to read G sensor?
            if(millis() > TimerG)
            {
                static byte G_counter = 4;
                
                // next read within 50ms
                TimerG += TMR_ARMED;

                // read G sensor, check for free-fall (less than 0.2 Gs)
                float g = SensorG();
                Serial.print("ARM: ");
                Serial.println(g, 2);
                if(g < 0.2)
                {
                    // check for persistency
                    if(G_counter) G_counter--;
                    else
                    {
                        // Ok, falling for enough time
                        State = ST_MOK_ACCL;

                        // clear max G
                        G_max = 0;

                        // reset buffer to store data
                        BuffSz = 0;
                        pBuff = Buffer;
                    }
                }
                else
                {
                    // not falling, reset G_counter up to 4*50 = 200ms
                    if(G_counter < 4) G_counter++;
                }
            }
            // any button pressed?
            if (StateEvt & (EV_BTN_0 | EV_BTN_1))
            {
                    byte Data;
                    // MAX G is stored as single byte, for atomic operations
                    // 4 bits mantissa: 0..15
                    // 4 bits decimal places: 0 .. 1/16
                    // Thus leading to a max value of 16-(1/16) = 15.9375
                    if(G_max > 15.9) Data = 0xFF; // 15+(15/16)
                    else
                    {
                        // Fixed point value
                        Data = (byte)(G_max * 16.0);
                    }

                    // Max G value
                    MemWrite8(EE_MAXG, Data);
                // go back to idle
                State = ST_IDLE;

                // Clear all leds
                PORTC &= ~0x0F;

                // blink within 500ms
                Timer = millis() + 500;
            }
            break;

        /**********************************************************************************************
         * 
         */
        case ST_MOK_ACCL:

            // keep blinking yelow led, faster
            if(millis() > Timer)
            {
                Timer += TMR_BLINK_FAST;
                PORTC |= 0x04;    // led on
                delay(15);          // wait
                PORTC &= ~0x04;   // led off
            }


            // Time to read G sensor?
            if(millis() > TimerG)
            {
                static byte G_Counter = 50; // 1 sec
                
                // next read within 20ms
                TimerG += TMR_FALL;

                //PORTC |= 0x02;    // led on

                // read G sensor, check for max value
                float g = SensorG();

                // Check if there is available buffer
                if(BuffSz < BUFFER_SZ)
                {
                    // Store it
                    *pBuff = (byte)(g * 16.0);

                    // next
                    pBuff++;
                    BuffSz++;
                }

                // New MAX value for G ?
                if(g > G_max)
                {
                    byte Data;
                    
                    G_max = g;
                    G_max_2 = g;

                    // Store current value in memory as LEDs
                    // 1 ..  3G -> Green 0x08
                    // 3 ..  5G -> Yelow 0x04
                    // 5 ..  9G -> 1 Red 0x02
                    // 9 .. 16G -> 2 Red 0x01
                    if(G_max < 3.0) Data = 0x88;
                    else if(G_max < 5.0) Data = 0xCC;
                    else if(G_max < 9.0) Data = 0xEE;
                    else Data = 0xFF;

                    // Led status
                    MemWrite8(EE_LEDS, Data);

                    // MAX G is stored as single byte, for atomic operations
                    // 4 bits mantissa: 0..15
                    // 4 bits decimal places: 0 .. 1/16
                    // Thus leading to a max value of 16-(1/16) = 15.9375
                    if(G_max > 15.9) Data = 0xFF; // 15+(15/16)
                    else
                    {
                        // Fixed point value
                        Data = (byte)(G_max * 16.0);
                    }

                    // Max G value
                    MemWrite8(EE_MAXG, Data);

                    // Reset G counter
                    G_Counter = 50;
                }


                //PORTC &= ~0x02;   // led off
                
                // Check if g is whithin 0.8 and 1.2 G for a few times
                // if so, that means we reached ground and stablized
                if(0.8 < g && g < 1.2)
                {
                    // Now check counter
                    if(G_Counter) G_Counter--;
                    else
                    {
                        // Z_Counter reached zero. That means G_Counter has been set to 0 for more than 10
                        // measurements. Goes to ST_LEDS and display impact
                        State = ST_LEDS;

                        // Show result
                        PORTC &= ~0x0F;
                        PORTC |= GetLeds();
                    }
                }
                else
                {
                   // not zero, reset G countdown
                   G_Counter = 50; 
                }
            }

//            // any button pressed?
//            if(StateEvt & (EV_BTN_0 | EV_BTN_1))
//            {
//                // go back to idle
//                State = ST_IDLE;
//
//                // Clear all leds
//                PORTC &= ~0x0F;
//
//                // blink within 500ms
//                Timer = millis() + 500;                
//            }

        break;



        /**********************************************************************************************
         * 
         */
        case ST_CAN_ARMD:

            // keep blinking yelow led, slowly
            if(millis() > Timer)
            {
                Timer += TMR_BLINK;
                PORTC |= 0x04;    // led on
                delay(15);          // wait
                PORTC &= ~0x04;   // led off
            }

            // Time to read G sensor?
            if(millis() > TimerG)
            {
                static byte G_counter = 2; // 1/10 sec
                
                // next read within 50ms
                TimerG += TMR_ARMED;

                PORTC |= 0x02;

                    // read G sensor, check for deployment
                    float g = SensorG();

                PORTC &= ~0x02;
                
                Serial.print("ARM: ");
                Serial.println(g, 2);
                if(g > 2.0)
                {
                    // check for persistency
                    if(G_counter) G_counter--;
                    else
                    {
                        // Ok, falling for enough time
                        State = ST_CAN_ACCL;

                        // reset buffer to store data
                        BuffSz = 0;
                        pBuff = Buffer;

                        // Timer to measure height
                        Timer = millis() + 25; // 40Hz
                    }
                }
                else
                {
                    // not falling, reset G_counter up to 2*50 = 100ms
                    if(G_counter < 2) G_counter++;
                }

                PORTC |= 0x02;

                    // read H sensor, check for deployment
                    float h = SensorH();

                    // save base altitude
                    H_base = h;

                PORTC &= ~0x02;

            }

        break;

        /**********************************************************************************************
         * 
         */
        case ST_CAN_ACCL:

            // Time to read G sensor?
            if(millis() > TimerG)
            {
                static byte G_counter = 5; // halfway from each transition
                
                // next read within 50ms
                TimerG += TMR_ARMED;

                // read G sensor, check for free-fall (less than 0.2 Gs)
                float g = SensorG();
                Serial.print("ARM: ");
                Serial.println(g, 2);
                if(g < 0.2)
                {
                    // check for persistency
                    if(G_counter) G_counter--;
                    else
                    {
                        // Ok... 
                        // cansat has been launched: G > 2   for 100ms
                        // cansat has been flying  : G < 0.2 for 500ms
                        // Move to full log state, skipping this verification
                        State = ST_CAN_LOGM;
                    }
                }
                else
                {
                    // not falling, reset G_counter up to 10*50 = 200ms
                    if(G_counter < 10) G_counter++;
                    else
                    {
                        // Ok, false alarm...
                        // Go back to armed and wait for another shot
                        State = ST_CAN_ARMD;

                        // reset buffer to store data
                        BuffSz = 0;
                        pBuff = Buffer;

                        // reset, so stop here
                        break;
                    }
                }
            }

        // Does not break! Just move ahead and do logs
        //break;

        /**********************************************************************************************
         * 
         */
        case ST_CAN_LOGM:


            // keep blinking yelow led, faster
            if(millis() > Timer)
            {
                Timer += 25;

                // Read height sensor
                float h = SensorH();

                // relative to base height
                h = h - H_base;

                // Buffer available?
                if(BuffSz < BUFFER_SZ)
                {
                    // height data in fixed point, 1 bit/decimal
                    h = h * 2.0;
    
                    // store it in buffer
                    *pBuff = (byte)h;
    
                    // next
                    pBuff++;
                    BuffSz++;
                }
                else
                {
                    // Buffer is full, stop logging and show leds
                    State = ST_LEDS;

                    // Show result
                    PORTC &= ~0x0F;
                    PORTC |= GetLeds();
                }

                
                PORTC |= 0x04;    // led on
                delay(5);          // wait
                PORTC &= ~0x04;   // led off
            }

        break;


        /**********************************************************************************************
         * 
         */
        case ST_CHKG:

            // check sensor
            if(millis() > Timer)
            {
                int16_t x,y,z;
                byte sig = 0;
                byte led = 0;
                
                Timer += TimerStp;

                // read gravity
                float g = SensorG();

                // read components (last, not from sensor)
                SensorAcc(&x, &y, &z, false);

                Serial.print("Acc: \t");
                Serial.print(x);
                Serial.write('\t');
                Serial.print(y);
                Serial.write('\t');
                Serial.print(z);
                Serial.write('\t');
                Serial.print(g, 2);                
                Serial.write('\r');
                Serial.write('\n');

                // if G less than 1
                if(g < 1.0) { g = 1.0 - g; sig = 1; }

                // check intensity
                if(g > 1.4) led = 0x0F;         // 2 red
                else if(g > 1.2) led = 0x0E;    // 1 red
                else if(g > 1.1) led = 0x0C;    // 1 ylw
                else led = 0x08;                // 1 grn

                PORTC &= 0x0F;
                PORTC |= led;    // led on
                delay(TimerStp>>1);          // wait
                if(sig) PORTC &= ~0x0F;   // led off
            }

            // button 1 pressed?
            if(StateEvt & EV_BTN_1)
            {
                // adjust rate
                if(TimerStp == TMR_BLINK)           TimerStp = TMR_BLINK/2;
                else if(TimerStp == (TMR_BLINK/2))  TimerStp = TMR_BLINK/4;
                else if(TimerStp == (TMR_BLINK/4))  TimerStp = TMR_BLINK/8;
                else if(TimerStp == (TMR_BLINK/8))  TimerStp = TMR_BLINK/16;
                else                                TimerStp = TMR_BLINK;
            }

            // button 0 pressed?
            if(StateEvt & EV_BTN_0)
            {
                // go back to idle
                State = ST_IDLE;

                // Clear all leds
                PORTC &= ~0x0F;

                // blink within 500ms
                Timer = millis() + 500;                
            }

        break;

        /**********************************************************************************************
         * 
         */
        case ST_LEDS:

            // any button pressed?
            if(StateEvt & (EV_BTN_0 | EV_BTN_1))
            {
                // go back to idle
                State = ST_IDLE;

                // Clear all leds
                PORTC &= ~0x0F;

                // blink within 500ms
                Timer = millis() + 500;                
            }

            // display data
            if(millis() > Timer)
            {
                Timer += TMR_BLINK;

                // read max g from memory
                ShowMaxG();
            }
        
        break;


        /**********************************************************************************************
         * 
         */
        case ST_BUFF:

            // display data
            if(millis() > Timer)
            {
                // timer
                uint16_t Tmr = 0;
                
                // for all bytes in buffer
                byte i = BuffSz;
                pBuff = Buffer;

                // Buffer is mockup?
                if(Model == MODEL_MOCKUP)
                {
                    // header
                    Serial.print("Time(ms)\tG\r\n");
    
                    while(i)
                    {                
                        // convert it to float point
                        float g = (float)*pBuff;
                        g = g * 0.0625;
    
                        // Print time
                        if(Tmr < 1000) Serial.write(' ');
                        if(Tmr < 100) Serial.write(' ');
                        if(Tmr < 10) Serial.write(' ');
                        Serial.print(Tmr);
                        Serial.write('\t');
                        Serial.write('\t');
        
                        // print it
                        if(g < 10.0) Serial.write(' ');
                        Serial.println(g, 2);
    
                        // next time
                        Tmr += TMR_FALL;
    
    
                        // one less
                        i--;
                        pBuff++;
                    }
                }

                // next timer
                Timer = millis() + 500;

                // back to idle
                State = ST_IDLE;
            }
        
        break;

        /**********************************************************************************************
         * 
         */
        default:
            Leds = 0x0F;
            
        case ST_DEAD:
            while(Leds)
            {
                // blink specified leds to indicate error
                PORTC ^= Leds;
                delay(TMR_BLINK_FAST);
            }
        break;
    }

    // Clear events each process
    StateEvt = 0x00;
}

void BtnCallback(byte Falling, byte Rising)
{
    // Buttons event
    if(Falling & BTN_0) StateEvt |= EV_BTN_0;
    if(Falling & BTN_1) StateEvt |= EV_BTN_1;

    if(Rising & BTN_0) StateEvt |= EV_RLS_0;
    if(Rising & BTN_1) StateEvt |= EV_RLS_1;
}


byte GetLeds(void)
{
    // read base address
    byte data = MemRead8(0);

    // check magic number
    if(data != MAGIC) return(0);

    // Read led data address
    data = MemRead8(EE_LEDS);

    // Check 2 nibbles equal
    if(data & 0x0F != (data >> 4)) return(0);

    // Valid led data
    return(data & 0x0F);    
}

byte ShowMaxG(void)
{
    // read base address
    byte data = MemRead8(0);

    // check magic number
    if(data != MAGIC) return(0);

    // Read g data address
    data = MemRead8(EE_MAXG);

    // outputs via seral
    Serial.print("Max G is: ");
    float g = ((float)data) / 16.0;
    Serial.print(g, 2);
    Serial.print(" (");
    Serial.print(G_max_2, 2);
    Serial.println(")");

    // Valid led data
    return(data);  
}


// BACKUP codes....

                #if 0
                if(g > G_max)
                {
                    G_max = g;
                    G_max_2 = g;
                    G_Counter = 10;
                }
                else
                {
                    if(G_Counter > 1) G_Counter--;
                    else if(G_Counter == 1)
                    {
                        byte Data;
                        
                        // Store current value in memory as LEDs
                        // 1 ..  3G -> Green 0x08
                        // 3 ..  5G -> Yelow 0x04
                        // 5 ..  9G -> 1 Red 0x02
                        // 9 .. 16G -> 2 Red 0x01
                        if(G_max < 3.0) Data = 0x88;
                        else if(G_max < 5.0) Data = 0xCC;
                        else if(G_max < 9.0) Data = 0xEE;
                        else Data = 0xFF;

                        // Led status
                        MemWrite8(EE_LEDS, Data);

                        // MAX G is stored as single byte, for atomic operations
                        // 4 bits mantissa: 0..15
                        // 4 bits decimal places: 0 .. 1/16
                        // Thus leading to a max value of 16-(1/16) = 15.9375
                        if(G_max > 15.9) Data = 0xFF; // 15+(15/16)
                        else
                        {
                            // Fixed point value
                            Data = (byte)(G_max * 16.0);
                        }

                        // Max G value
                        MemWrite8(EE_MAXG, Data);

                        // Stop counting
                        G_Counter = 0;
                    }
                }
                #endif


