
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BMA250.h>
#include "model.h"

#define FIFO_SZ         4

// PRessure sensor
Adafruit_BMP280 bmp;

// Acceleration sensor
BMA250 acc;

// barometer variables
float h;

// fifo control
byte fi = 0;
int16_t fx[FIFO_SZ], fy[FIFO_SZ], fz[FIFO_SZ];

// acceleration variables
int16_t ax, ay, az;
int16_t bx, by, bz;
float g;

byte SensorInit(void)
{
    byte Ret = 0;

    acc.reset(); //initialize();
    delay(100);
    if(acc.testConnection()) 
    {
        Serial.println("Could not find a valid ACCELERATION sensor, check wiring!");
        Serial.print("Accel ID is ");
        Serial.println(acc.getDeviceID());
        //Ret |= 0x01; --bypass sensor verification
    }
    else
    {
        // initialized, set range to 16G
        acc.setRange(BMA250_RANGE_16G);
        acc.setBandwidth(BMA250_BW_MIN_HZ);
    }

    // clear fifos
    memset(fx, 0, FIFO_SZ * sizeof(int16_t));
    memset(fy, 0, FIFO_SZ * sizeof(int16_t));
    memset(fz, 0, FIFO_SZ * sizeof(int16_t));
    fi = 0;

    // initially, no biases
    bx = 0;
    by = 0;
    bz = 0;

    if(Model == MODEL_CANSAT)
    {
        if (!bmp.begin()) 
        {
            Serial.println("Could not find a valid PRESSURE sensor, check wiring!");
            Serial.print("Pressure ID is ");
            Serial.println(bmp.chipID());
            Ret |= 0x02;
        }
    }

    return(Ret);
}

void SensorSetBias(int16_t x, int16_t y, int16_t z)
{
    // save them
    bx = x;
    by = y;
    bz = z;
}

void SensorSetRange(byte Range)
{
    if(Range == 16)     acc.setRange(BMA250_RANGE_16G);
    else if(Range == 8) acc.setRange(BMA250_RANGE_8G);
    else if(Range == 4) acc.setRange(BMA250_RANGE_4G);
    else                acc.setRange(BMA250_RANGE_2G);

    // set also BW filter
    acc.setBandwidth(BMA250_BW_MIN_HZ);
}


void SensorTest(void)
{
    int i;

    long us_min, us_max, us_avg, us;

    for(i = 0; i < 10; i++)
    {

        // Measure accelerometer time
    }

    
}

void SensorAcc(int16_t* x, int16_t* y, int16_t* z, bool sensor)
{
    if(sensor) 
    {
        // read from sensor
        acc.getAcceleration(x, y, z);

        // remove bias
        *x -= bx;
        *y -= by;
        *z -= bz;
    }
    else 
    {
        // Use last readout value
        *x = ax;
        *y = ay;
        *z = az;
    }
}


float SensorG(void)
{
    // read accelerometer
    acc.getAcceleration(&ax, &ay, &az);

    // remove biases
    ax = ax - bx;
    ay = ay - by;
    az = az - bz;

    // insert into fifo
    fx[fi] = ax;
    fy[fi] = ay;
    fz[fi] = az;

    // next
    fi++;
    if(fi == FIFO_SZ) fi = 0;

    // Get the average of last 4 measures
    ax = FfoAverage(fx, fi, FIFO_SZ);
    ay = FfoAverage(fy, fi, FIFO_SZ);
    az = FfoAverage(fz, fi, FIFO_SZ);


    // Compute gravity in Gs (512 LSB / 1G * Range)
    g = sqrt(pow((float)ax, 2) + pow((float)ay, 2) + pow((float)az, 2)) * (0.001953125 * 16.0);

    // output
    return(g);
}

float SensorH(void)
{
    // Temperature and pressure data
    bmp.readTemperature();
    bmp.readPressure();

    // Compute and return altitude in QNE!
    return(bmp.readAltitude());
}


int16_t FfoAverage(int16_t *f, byte fi, byte sz)
{
    byte i;
    int16_t avg;

    avg = 0;
    
    // check limit
    if(sz > FIFO_SZ) sz = FIFO_SZ;

    // check easy way
    if(sz == FIFO_SZ) 
    {
        for(fi = 0; fi < FIFO_SZ; fi++) { avg += *f; f++; }
    }
    else
    {
        // not easy way...
        i = sz;
        while(i)
        {
            // accumulate item
            avg += f[fi];

            // get the item just before
            if(fi) fi--;
            else fi = FIFO_SZ - 1;
            
            // one less
            i--;
        }
    }

    // average all of sum
    avg = avg / sz;
    return(avg);
}








