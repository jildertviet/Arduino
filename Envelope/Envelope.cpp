/*
 Envelop.cpp - Library for using envelopes.
 Created by Jildert Viet, October 24, 2015.
 Released into the public domain.
 */

#include "Arduino.h"
#include "Envelope.h"

Envelope::Envelope(){
    
}

Envelope::Envelope(int attackTime=100, int sustainTime=200, int releaseTime=200){
    this->attackTime = attackTime;
    this->sustainTime = sustainTime;
    this->releaseTime = releaseTime;
    
    value = 0;
    gate = loop = sustainBool = false;
}

void Envelope::attack(){
    if(value < 1){
        value += 1./(attackTime);
    } else{
        sustainBool = true;
    }
    if(value > 1) value = 1;
}

void Envelope::sustain(){
    static int counter = 0;
    if(counter < sustainTime){
        counter++;
    } else{
        sustainBool = false;
        gate = false;
        counter = 0;
    }
}

void Envelope::release(){
    if(value > 0){
        value -= 1./(releaseTime);
    } else{
        if(loop)
            gate = true;
    }
    if(value < 0) value = 0;
}

float Envelope::process(){ // Run 1 time per ms
    if(gate)
        attack();
    if(sustainBool)
        sustain();
    if(!gate)
        release();
    return value;
}