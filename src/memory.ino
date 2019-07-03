
#include <EEPROM.h>

byte MemInit(void)
{
    // Check for magic number
    byte Magic = EEPROM.read(0);
    if(Magic != MAGIC)
    {
        PORTC |= 0x01;
        
        // Clear all parameters
        for(byte i = EE_LEDS; i < EE_ACC_BIAS_Z+1; i++)
        {
            // clear that address
            EEPROM.write(i, 0x00);
        }

        // Set up magic number
        EEPROM.write(0, MAGIC);

        delay(250);
        PORTC &= ~0x01;

        return(1);
    } 

    return(0);
}

byte MemRead8(unsigned int addr)
{
    return(EEPROM.read(addr));
}


byte MemRead16(unsigned int addr)
{
    int h, l;
    
    l = EEPROM.read(addr); addr++;
    h = EEPROM.read(addr);

    l &= 0x00FF;
    h <<= 8;

    return(h | l);
}


void MemWrite8(unsigned int addr, byte val)
{
    EEPROM.write(addr, val);
}


void MemWrite16(unsigned int addr, int val)
{
    EEPROM.write(addr, (byte)(val & 0x00FF)); 
    
    addr++;
    val = val >> 8;
    
    EEPROM.write(addr, (byte)(val & 0x00FF)); 
}



