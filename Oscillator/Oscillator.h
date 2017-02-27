/*
 Oscillator.cpp - Library for using oscillators.
 Created by Jildert Viet, October 24, 2015.
 Released into the public domain.
 */

#ifndef Oscillator_h
#define Oscillator_h

#include "Arduino.h"
#include <math.h>

class Oscillator
{
public:
    Oscillator();
    Oscillator(int waveTableRes=128, float freq=1, float phase_offset=0, int mode=0);
//    void setup(int waveTableRes, float freq, float phase_offset);


    float process();
    float phase_offset = 0;
    
    float freq;
    int waveTableRes;
    float *waveTable;
    
    void setupWaveTable(int mode);
    float stepSize;
    float pulseWidth = 0.5;
    
    
    float value;
    bool bActive = true;
    
    void setFreq(float freq);
    
    float millisTemp = 0;
    unsigned long millisMin = 0;
    
    void retrigger();
};

#endif