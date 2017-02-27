/*
 Oscillator.cpp - Library for using Oscillators.
 Created by Jildert Viet, October 24, 2015.
 Released into the public domain.
 */

#include "Arduino.h"
#include "Oscillator.h"

Oscillator::Oscillator(){
    
}

Oscillator::Oscillator(int waveTableRes, float freq, float phase_offset, int mode){
    this->freq = freq;
    this->waveTableRes = waveTableRes;
    this->phase_offset = phase_offset;
    value = 0;
    stepSize = waveTableRes*(1.f/(1.f/freq))/100.f;
    setupWaveTable(mode);
    //    i=phase_offset*waveTableRes;
}

float Oscillator::process(){ // Run one time per ms.
//    static float i=phase_offset*waveTableRes;
    
    // Dit klopt nog niet
//    i += stepSize;
//    if(i > waveTableRes) i = 0;
//    value = waveTable[(int)i];
//    return value;
    
    float phase;
    
     if(bActive){
         millisTemp = millis() - millisMin; // For resetting phase
         millisTemp /= ((1.f/freq)*1000);
         phase = (int)(millisTemp * waveTableRes) % waveTableRes;
         phase += (phase_offset * waveTableRes);
         phase = (int)phase % waveTableRes;
         float output = waveTable[(int)phase];
         return output;
     } else {
         return 0;
     }
    
}

void Oscillator::setFreq(float freq){
    this->freq = freq;
    stepSize = waveTableRes*(1.f/(1.f/freq))/100.f;
}

void Oscillator::setupWaveTable(int mode){ // 0 = sin, 1 = pulse
    switch (mode) {
        case 0:{
            double two_pi = 4.0*atan(1.0)*2.0;
            waveTable = new float[waveTableRes];
            for(int i=0; i<waveTableRes; i++){
                float period = (i*(1.0/(float)waveTableRes))*two_pi;
                period = sin(period);
                waveTable[i] = period;
            }
        }
            break;
        case 1:
            for(int i=0; i<waveTableRes; i++){
                if(i<waveTableRes*pulseWidth){
                    waveTable[i] = 1;
                } else{
                    waveTable[i] = 0;
                }
            }
            break;
            
        default:
            break;
    }

}

void Oscillator::retrigger(){
    millisMin = millis();
}
